#include "recorder.h"
#include <nlohmann/json.hpp>
#include <Helper/json.h>
#include <Helper/ffmpeg.h>
#include <Helper/defer.h>
#include <Streams/videowriter.h>
#include <Hooks/ClientHook.h>
#include <Hooks/OverlayHook.h>
#include <Hooks/fx/VideoModeHook.h>
#include <Hooks/fx/ShaderApiHook.h>
#include <Modules/fx/configmodule.h>
#include <Modules/InputModule.h>
#include "mainwindow.h"
#include <Modules/Draw.h>
#include "ActiveStream.h"
#include "StreamEditor.h"
#include <Base/Interfaces.h>
#include <SDK/cdll_int.h>
#include <SDK/ienginetool.h>
#include <SDK/KeyValues.h>
#include <SDK/ivrenderview.h>
#include <SDK/view_shared.h>
#include <SDK/convar.h>
#include <cstdint>
#include <varargs.h>
#include <chrono>
#include <array>
#include <cassert>
#include <thread>

static const std::string DEFAULT_STREAM_NAME = "video";

static std::filesystem::path game_dir;
static std::filesystem::path working_dir;
static ConCommand sf_recorder_start("sf_recorder_start",
    [](const CCommand& cmd) {
        if (cmd.ArgC() == 2) // Path was most likely passed in quotes, without spaces
            g_recorder.StartMovie(cmd.Arg(1));
        else if (cmd.ArgC() > 2) // Path was most likely passed without quotes
            g_recorder.StartMovie(cmd.ArgS());
        else // Use default path
            g_recorder.StartMovie();
    },
    "Usage: sf_recorder_start [path]\n"
    "Stop any current recording and start a new one.\n"
    "The path, if unspecified, is read from the config or GUI.\n"
);
static ConCommand sf_recorder_stop("sf_recorder_stop",
    [] { g_recorder.StopMovie(); },
    "Stop the current recording"
);

void CRecorder::StartListening()
{
    Interfaces::engine->ClientCmd_Unrestricted("echo SparklyFX is loaded. Press F11 or Insert to open the menu.");

    // Get game directory
    {
        std::array<char, 512> buffer;
        std::error_code err;

        Interfaces::engine_tool->GetGameDir(buffer.data(), buffer.size());
        game_dir = buffer.data();
        working_dir = std::filesystem::current_path(err);
        if (err)
            printf("Failed to get working directory");
    }

    // Find FFmpeg and a good default encoder config
    {
        auto ffmpeg_path_list = Helper::FFmpeg::ScanForExecutables();
        if (Helper::FFmpeg::GetDefaultPath().empty() && !ffmpeg_path_list.empty())
            Helper::FFmpeg::SetDefaultPath(ffmpeg_path_list.front());

        auto ffmpeg_path = Helper::FFmpeg::GetDefaultPath();
        bool has_ffmpeg = !ffmpeg_path.empty();
        if (has_ffmpeg)
            m_videoconfig.type = EncoderConfig::TYPE_FFMPEG;
        
        auto& preset = EncoderConfig::GetFFmpegPresets().front();
        m_videoconfig.ffmpeg_output_args = preset.args;
        m_videoconfig.ffmpeg_output_ext = preset.file_ext;
    }

    // By default, allocate 1 frame for every thread/core.
    m_framepool_size = std::thread::hardware_concurrency();
    if (m_framepool_size <= 1)
        m_framepool_size = 1;
    else
        m_framepool_size += 1; // Add an extra thread to minimize idle time

    InputModule::OnPostImguiInput.Listen(&CRecorder::OnPostImguiInput, this);
    CDraw::OnDraw.Listen(&CRecorder::OnDraw, this);
    MainWindow::OnTabBar.Listen(&CRecorder::OnTabBar, this);
    MainWindow::OnWindow.Listen(&CRecorder::OnWindow, this);
    CClientHook::OnFrameStageNotify.Listen(&CRecorder::OnFrameStageNotify, this);
    VideoModeHook::OnWriteMoveFrame.Listen(&CRecorder::OnWriteMovieFrame, this);
    ShaderApiHook::OnReadPixels.Listen(&CRecorder::OnReadPixels, this);
    ConfigModule::OnConfigSave.Listen(&CRecorder::OnConfigSave, this);
    ConfigModule::OnConfigLoad.Listen(&CRecorder::OnConfigLoad, this);
}

int CRecorder::OnPostImguiInput()
{
    if (m_record_bind.Poll() && !g_input.IsOverlayOpen())
    {
        if (!Interfaces::engine->Con_IsVisible()) // Don't active the keybind while typing in the console
            ToggleRecording(m_movie_path);
    }
    return 0;
}

int CRecorder::OnDraw()
{
    if (m_record_indicator && IsRecordingMovie())
    {
        const std::string_view text = "Recording";
        const ImU32 text_color = 0xFF0000FF;
        const ImU32 bkg_color = 0x40000000;
        const float padding = 8;
        const float text_space = 4; // Leave 4px space between circle and text
        const ImVec2 corner = ImVec2(10, 10);

        float text_height = ImGui::GetTextLineHeight();
        float rect_rounding = (text_height + padding) / 2;
        float text_width = ImGui::CalcTextSize(text.data()).x + text_height + text_space + rect_rounding/4;
        ImVec2 text_corner = ImVec2(padding / 2 + corner.x, padding / 2 + corner.y);
        ImVec2 rect_end = ImVec2(corner.x + padding + text_width, corner.y + padding + text_height);

        gDraw.List()->AddRectFilled(corner, rect_end, bkg_color, rect_rounding);
        gDraw.List()->AddRect(corner, rect_end, text_color, rect_rounding);
        gDraw.List()->AddCircleFilled(ImVec2(text_corner.x + text_height/2, text_corner.y + text_height/2), text_height/2, text_color);
        gDraw.List()->AddText(ImVec2(text_corner.x + text_height + text_space, text_corner.y), text_color, text.data());
    }

    return 0;
}

int CRecorder::OnTabBar()
{   
    if (!ImGui::BeginTabItem("Recorder"))
        return 0;

    bool clear_error_log = false;
    {
        auto locked_error_log = VideoLog::GetLog();
        if (!locked_error_log->empty())
        {
            ImGui::Text("Error log:");
            ImGui::SameLine();
            clear_error_log = ImGui::Button("Clear"); // NOTE: Clear() is deferred because VideoLog is locked right now
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,0,1));
            ImGui::SetNextItemWidth(-1.f); // Ensure that text spans the full width where the label usually appears
            ImGui::InputTextMultiline("##error_log",
                locked_error_log->data(), locked_error_log->length(), ImVec2(0,0), ImGuiInputTextFlags_ReadOnly
            );
            ImGui::PopStyleColor();
        }
    }

    if (clear_error_log)
        VideoLog::Clear();

    if (ImGui::Button(IsRecordingMovie() ? "Stop" : "Start"))
        ToggleRecording(m_movie_path);
    
    m_record_bind.OnMenu("Record hotkey");
    
    if (ImGui::TreeNode("Recording behavior"))
    {
        ImGui::BeginGroup();

        ImGui::Checkbox("Recording indicator", &m_record_indicator); ImGui::SameLine();
        Helper::ImGuiHelpMarker(
            "Displays an indicator on screen while recording\n"
            "(Currently, this decreases the recording performance by re-rendering the frame)"
        );
        ImGui::Checkbox("Auto-resume demo", &m_autoresume_demo); ImGui::SameLine();
        Helper::ImGuiHelpMarker("Resumes the demo when the recording starts");
        ImGui::Checkbox("Auto-pause demo", &m_autopause_demo); ImGui::SameLine();
        Helper::ImGuiHelpMarker("Pauses the demo when the recording stops");

        ImGui::EndGroup();
        ImGui::SameLine();
        ImGui::BeginGroup();

        if (ImGui::Checkbox("Auto-close menu", &m_autoclose_menu))
            m_autostop_recording &= m_autoclose_menu; // Do not auto-stop if auto-close is off.
        ImGui::SameLine(); Helper::ImGuiHelpMarker("Closes the menu when the recording starts");
        if (ImGui::Checkbox("Auto-stop recording", &m_autostop_recording))
            m_autoclose_menu |= m_autostop_recording; // Always auto-close if auto-stop is enabled.
        ImGui::SameLine(); Helper::ImGuiHelpMarker("Stops the recording when the menu is opened");

        ImGui::EndGroup();
        ImGui::TreePop();
    }

    bool was_recording = m_is_recording_; // Value could change. Store it here.
    if (was_recording)
        ImGui::BeginDisabled();

    if (m_movie_path.empty())
        ImGui::TextColored(ImVec4(1,1,0,1), "[!] Choose an output folder below");
    ImGui::Text("Output folder:"); ImGui::SameLine();
    Helper::ImGuiHelpMarker(
        "This folder will contain the movie files.\n"
        "The folder will be created automatically, if it doesn't exist."
    );
    std::string _movie_path_str = m_movie_path.string();
    ImGui::InputText("##output_folder", &_movie_path_str, ImGuiInputTextFlags_ReadOnly); ImGui::SameLine();
    if (ImGui::Button("Browse"))
    {
        auto optional_path = Helper::OpenFolderDialog(L"Select the output folder", &m_movie_path);
        if (optional_path)
            m_movie_path = std::move(*optional_path);
    }
    
    ImGui::PushID("##videoconfig");
    m_videoconfig.ShowImguiControls();
    ImGui::PopID();

    ImGui::SliderInt("Frame pool size", &m_framepool_size, 1, 128, "%d", ImGuiSliderFlags_AlwaysClamp);
    ImGui::SameLine();
    Helper::ImGuiHelpMarker(
        "The maximum number of frames that can be encoded at once.\n"
        "If your CPU has 8 cores, you may want a pool with 9 or 12 frames.\n"
        "Setting this value too high or too low can make it slower.\n"
    );

    int screen_w, screen_h;
    Interfaces::engine->GetScreenSize(screen_w, screen_h);
    // Approximate framepool RAM, assuming a 32-bit XRGB framebuffer
    float framepool_ram = screen_w * screen_h * 4;
    framepool_ram = framepool_ram * m_framepool_size / (1024 * 1024);
    
    if (was_recording)
        ImGui::EndDisabled();
    
    // Disabled text has less visual clutter
    ImGui::BeginDisabled();
    ImGui::Text("Framepool RAM: %f MB", framepool_ram);
    ImGui::Text("Framepool threads: %u", std::thread::hardware_concurrency());
    ImGui::TextWrapped("Game directory: %s", game_dir.u8string().c_str());
    ImGui::TextWrapped("Working directory: %s", working_dir.u8string().c_str());
    ImGui::EndDisabled();

    ImGui::EndTabItem();
    return 0;
}

int CRecorder::OnWindow()
{
    if (IsRecordingMovie() && m_autostop_recording)
        StopMovie();
    return 0;
}

int CRecorder::OnConfigSave()
{
    nlohmann::json j = {
        {"m_record_bind",           m_record_bind.ToJson()},
        {"m_record_indicator",      m_record_indicator},
        {"m_autoresume_demo",       m_autoresume_demo},
        {"m_autopause_demo",        m_autopause_demo},
        {"m_autoclose_menu",        m_autoclose_menu},
        {"m_autostop_recording",    m_autostop_recording},
        {"m_framepool_size",        m_framepool_size},
        {"m_movie_path",            m_movie_path},
        {"m_videoconfig",           m_videoconfig.ToJson()}
    };
    ConfigModule::GetOutput().emplace("Recorder", std::move(j));
    return 0;
}

int CRecorder::OnConfigLoad()
{
    const nlohmann::json* j = Helper::FromJson(ConfigModule::GetInput(), "Recorder");
    if (!j)
        return 0;
    
    int safe_framepool_size = m_framepool_size;
    m_record_bind.FromJson(Helper::FromJson(j, "m_record_bind"));
    Helper::FromJson(j, "m_record_indicator", m_record_indicator);
    Helper::FromJson(j, "m_autoresume_demo", m_autoresume_demo);
    Helper::FromJson(j, "m_autopause_demo", m_autopause_demo);
    Helper::FromJson(j, "m_autoclose_menu", m_autoclose_menu);
    Helper::FromJson(j, "m_autostop_recording", m_autostop_recording);
    Helper::FromJson(j, "m_framepool_size", safe_framepool_size);
    Helper::FromJson(j, "m_movie_path", m_movie_path);
    m_videoconfig.FromJson(Helper::FromJson(j, "m_videoconfig"));
    safe_framepool_size = min(safe_framepool_size, 128);
    safe_framepool_size = max(safe_framepool_size, 1);
    m_framepool_size = safe_framepool_size;
    return 0;
}

bool CRecorder::IsRecordingMovie() {
    return m_is_recording_;
}

bool CRecorder::ShouldRecordFrame() {
    return IsRecordingMovie() && !Interfaces::engine->Con_IsVisible();
}

void CRecorder::StartMovie(const std::filesystem::path& path)
{
    std::scoped_lock lock{m_movie_mtx};
    m_next_movie_path = path;
    m_do_start_recording = true;
}
void CRecorder::StopMovie()
{
    std::scoped_lock lock{m_movie_mtx};
    m_do_stop_recording = true;
}
void CRecorder::ToggleRecording(const std::filesystem::path& path)
{
    std::scoped_lock lock{m_movie_mtx};
    if (m_is_recording_)
        m_do_stop_recording = true;
    else {
        m_do_start_recording = true;
        m_next_movie_path = path;
    }
}

bool CRecorder::SetupMovie(const std::filesystem::path& path)
{
    if (m_movie)
        return true;
    
    VideoLog::Clear();

    if (path.empty())
    {
        VideoLog::AppendError("No folder was given for recording\n");
        return false;
    }
    
    if (!Interfaces::engine->IsInGame())
    {
        VideoLog::AppendError("Must be in-game to record\n");
        return false;
    }

    int screen_w, screen_h;
    Interfaces::engine->GetScreenSize(screen_w, screen_h);

    // Create the Movie instance
    {
        auto lock = g_active_stream.ReadLock();
        std::vector<Stream::Ptr> dummy_stream_list;
        auto* stream_list = &g_stream_editor.GetStreams();
        if (stream_list->empty()) // If there are no streams, make an empty one
        {
            dummy_stream_list.emplace_back(std::make_shared<Stream>("video"));
            stream_list = &dummy_stream_list;
        }

        m_movie.emplace(
            screen_w, screen_h, path, *stream_list,
            m_framepool_size, m_videoconfig
        );
    }

    if (m_movie->Failed())
    {
        m_movie = std::nullopt;
        return false;
    }

    // Use engine_tool to record sound for us, otherwise we would need a signature to SND_StartMovie
    KeyValuesAD movie_params("movie_params");
    movie_params->SetString("filename", m_movie->GetTempAudioName().c_str());
    movie_params->SetInt("outputwav", 1);
    movie_params->SetFloat("framerate", m_videoconfig.framerate); // This one is a float. Dunno why.
    Interfaces::engine_tool->StartMovieRecording(movie_params);

    if (m_autoresume_demo)
        Interfaces::engine->ExecuteClientCmd("demo_resume");
    if (m_autoclose_menu)
        g_input.SetOverlayOpen(false);
    
    return true;
}

static void AttemptToMoveTempAudioFile(const std::filesystem::path& old_path, const std::filesystem::path& new_path)
{
    constexpr int WAIT_MS = 200;
    constexpr int TIMEOUT_MS = 10'000;

    std::error_code err;
    for (int attempts = 0; attempts * WAIT_MS < TIMEOUT_MS; ++attempts)
    {
        std::filesystem::rename(old_path, new_path, err);
        if (!err)
            return;
        std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_MS));
    }

    VideoLog::AppendError("Failed to move temp audio file (%s): '%s' -> '%s'\n",
        err.message().c_str(),
        old_path.u8string().c_str(),
        new_path.u8string().c_str()
    );
}

void CRecorder::CleanupMovie()
{
    if (!m_movie)
        return;

    Interfaces::engine_tool->EndMovieRecording();
    if (m_autopause_demo)
        Interfaces::engine->ExecuteClientCmd("demo_pause");
    g_stream_editor.OnEndMovie(); // This just sets the preview stream again
    
    // Sometimes the audio file cannot be renamed because the engine is still writing to it.
    // The solution? Retry a couple times in another thread.

    std::filesystem::path new_audio_path = m_movie->GetRootPath() / "audio.wav";
    std::filesystem::path old_audio_path = game_dir / m_movie->GetTempAudioName();
    std::thread(AttemptToMoveTempAudioFile, std::move(old_audio_path), std::move(new_audio_path)).detach();
    m_movie = std::nullopt;
}

void CRecorder::WaitForRenderQueue()
{
    // This is a very indirect way to properly wait for rendering to finish.
    // ReadPixels has beeen hooked to do the waiting for us without reading the pixels.
    // The IShaderAPI's `ReadPixels` impl also calls `FlushBufferedPrimitives`, so we call that too.
    Interfaces::mat_system->GetRenderContext()->ReadPixels(0, 0, 0, 0, nullptr, IMAGE_FORMAT_BGRX8888);
    g_hk_shaderapi.ShaderApi()->FlushBufferedPrimitives();
}

void CRecorder::CopyCurrentFrameToSurface(IDirect3DSurface9* dst)
{
    WaitForRenderQueue();

    IDirect3DSurface9* render_target;
    g_hk_overlay.Device()->GetRenderTarget(0, &render_target);
    defer { render_target->Release(); };
    g_hk_overlay.Device()->StretchRect(render_target, nullptr, dst, nullptr, D3DTEXF_NONE);
}

int CRecorder::OnFrameStageNotify(ClientFrameStage_t stage)
{
    if (stage == FRAME_START)
    {
        // TODO(Cade): Move this out of here and into the base!!!
        ConVar_Register();

        // Print messages that were queued for the game thread
        VideoLog::GetConsoleQueue().ExecuteAndClear();

        // Start/stop the movie

        std::scoped_lock lock{m_movie_mtx};
        if (m_do_stop_recording)
        {
            m_do_stop_recording = false;
            CleanupMovie();
        }

        if (m_do_start_recording)
        {
            m_do_start_recording = false;
            CleanupMovie();
            
            if (!SetupMovie(m_next_movie_path))
                CleanupMovie();
        }
        
        m_is_recording_ = m_movie.has_value();
        return 0;
    }

    if (stage != FRAME_RENDER_END)
        return 0;

    if (!m_movie)
        return 0;

    if (m_movie->Failed() || m_movie->GetFramePool().IsClosed())
    {
        StopMovie();
        CleanupMovie();
        return 0;
    }
    
    // Don't record unless a movie is started and the console is closed (TODO: and no loading screen)
    if (!ShouldRecordFrame())
        return 0;

    size_t frame_index = m_movie->NextFrameIndex();
    
    // We don't explicitly lock any mutex.
    // Assume that nothing is modified while recording.
    
    // If there is only one stream and it has no rendering effects, then take this fast path.
    if (m_movie->GetStreams().size() == 1 && m_movie->GetStreams()[0].stream->GetRenderTweaks().empty())
    {
        auto& [stream, writer] = m_movie->GetStreams().front();
        auto frame = m_movie->GetFramePool().PopEmptyFrame();
        if (frame == nullptr)
            return 0; // The FramePool was closed
        CopyCurrentFrameToSurface(frame->buffer.GetSurface());
        m_movie->GetFramePool().PushFullFrame(frame, frame_index, writer);
        return 0;
    }
    
    // Here, many streams exist with different effects, so we will re-render for each of them.
    for (auto& [stream, writer] : m_movie->GetStreams())
    {
        g_active_stream.Set(stream);
        g_active_stream.SignalUpdate();
        // Update the materials right now, instead of waiting for the next frame.
        g_active_stream.UpdateMaterials();
        g_active_stream.RenderView();
        WaitForRenderQueue();
        g_active_stream.DrawDepth();

        auto frame = m_movie->GetFramePool().PopEmptyFrame();
        if (frame == nullptr)
            break; // The FramePool was closed
        CopyCurrentFrameToSurface(frame->buffer.GetSurface());
        m_movie->GetFramePool().PushFullFrame(frame, frame_index, writer);
    }

    return 0;
}

int CRecorder::OnWriteMovieFrame()
{
    if (IsRecordingMovie())
        return EventReturnFlags::NoOriginal;
    return 0;
}

int CRecorder::OnReadPixels()
{
    if (m_read_pixels)
        return 0;
    if (IsRecordingMovie())
        return EventReturnFlags::NoOriginal;
    return 0;
}