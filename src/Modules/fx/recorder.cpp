#include "recorder.h"
#include <Helper/ffmpeg.h>
#include <Streams/videowriter.h>
#include <Hooks/ClientHook.h>
#include <Modules/Menu.h>
#include <Modules/Draw.h>
#include "ActiveStream.h"
#include "StreamEditor.h"
#include <Base/Interfaces.h>
#include <SDK/cdll_int.h>
#include <SDK/ienginetool.h>
#include <SDK/KeyValues.h>
#include <SDK/ivrenderview.h>
#include <SDK/view_shared.h>
#include <cstdint>
#include <varargs.h>
#include <chrono>
#include <array>
#include <cassert>
#include <thread>

static const std::string DEFAULT_STREAM_NAME = "video";

static std::filesystem::path game_dir;
static std::filesystem::path working_dir;

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

    // By default, allocate 1 frame for every thread/core.
    m_framepool_size = std::thread::hardware_concurrency();
    if (m_framepool_size <= 1)
        m_framepool_size = 1;
    else
        m_framepool_size += 1; // Add an extra thread to minimize idle time

    Listen(EVENT_POST_IMGUI_INPUT, [this]{ return OnPostImguiInput(); });
    Listen(EVENT_DRAW, [this]{ return OnDraw(); });
    Listen(EVENT_MENU, [this]{ return OnMenu(); });
    Listen(EVENT_FRAMESTAGENOTIFY, [this]{ return OnFrameStageNotify(); });
}

int CRecorder::OnPostImguiInput()
{
    if (m_record_bind.Poll() && !g_menu.IsOpen())
    {
        if (!Interfaces::engine->Con_IsVisible()) // Don't active the keybind while typing in the console
            ToggleRecording();
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

int CRecorder::OnMenu()
{
    if (ImGui::CollapsingHeader("Recording"))
    {
        bool has_errors = !GetErrorLog()->empty();
        if (has_errors)
        {
            ImGui::Text("Errors:"); ImGui::SameLine();
            if (ImGui::Button("Clear"))
                ClearErrorLog();
            
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,0,1));
            auto locked_error_log = GetErrorLog();
            ImGui::InputTextMultiline("##error_log",
                locked_error_log->data(), locked_error_log->length(), ImVec2(0,0), ImGuiInputTextFlags_ReadOnly
            );
            ImGui::PopStyleColor();
        }

        if (ImGui::Button(IsRecordingMovie() ? "Stop" : "Start"))
            ToggleRecording();
        
        m_record_bind.OnMenu("Record hotkey");
        
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

        if (m_is_recording)
            ImGui::BeginDisabled();

        ImGui::Text("Output folder:"); ImGui::SameLine();
        Helper::ImGuiHelpMarker(
            "This folder will contain the movie files.\n"
            "The folder will be created automatically, if it doesn't exist."
        );
        ImGui::InputText("##output_folder", &m_movie_path.string(), ImGuiInputTextFlags_ReadOnly); ImGui::SameLine();
        if (ImGui::Button("Browse"))
        {
            auto optional_path = Helper::OpenFolderDialog(L"Select the output folder");
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
        float framepool_ram = screen_w * screen_h * FrameBufferRGB::NUM_CHANNELS * FrameBufferRGB::NUM_BYTES;
        framepool_ram = framepool_ram * m_framepool_size / (1024 * 1024);
        
        if (m_is_recording)
            ImGui::EndDisabled();
        
        // Disabled text has less visual clutter
        ImGui::BeginDisabled();
        ImGui::Text("Framepool RAM: %f MB", framepool_ram);
        ImGui::Text("Framepool threads: %u", std::thread::hardware_concurrency());
        ImGui::TextWrapped("Game directory: %s", game_dir.string().c_str());
        ImGui::TextWrapped("Working directory: %s", working_dir.string().c_str());
        ImGui::EndDisabled();
    }

    if (IsRecordingMovie() && m_autostop_recording)
        StopMovie();
    return 0;
}

bool CRecorder::IsRecordingMovie() {
    return m_is_recording;
}

bool CRecorder::ShouldRecordFrame() {
    return IsRecordingMovie() && !Interfaces::engine->Con_IsVisible() && m_movie;
}

bool CRecorder::SetupMovie()
{
    if (m_movie)
        return true;
    
    ClearErrorLog();

    if (m_movie_path.empty())
    {
        AppendErrorLog("No folder was given for recording");
        return false;
    }
    
    if (!Interfaces::engine->IsInGame())
    {
        AppendErrorLog("Must be in-game to record\n");
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
            screen_w, screen_h, std::filesystem::path{m_movie_path}, *stream_list, m_framepool_size,
            [this](std::string_view str) { AppendErrorLog(str); },
            m_videoconfig
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
        g_menu.SetOpen(false);
    
    return true;
}

static void AttemptToMovieTempAudioFile(std::filesystem::path&& old_path, std::filesystem::path&& new_path)
{
    const int WAIT_MS = 200;
    const int TIMEOUT_MS = 10'000;

    std::error_code err;
    for (int attempts = 0; attempts * WAIT_MS < TIMEOUT_MS; ++attempts)
    {
        std::filesystem::rename(old_path, new_path, err);
        if (!err)
            return;
        std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_MS));
    }

    g_recorder.FormatErrorLog("Failed to move temp audio file (%s): '%s' -> '%s'\n",
        err.message().c_str(),
        old_path.string().c_str(),
        new_path.string().c_str()
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

    std::filesystem::path new_audio_path = m_movie_path / "audio.wav";
    std::filesystem::path old_audio_path = game_dir / m_movie->GetTempAudioName();
    std::thread(AttemptToMovieTempAudioFile, std::move(old_audio_path), std::move(new_audio_path)).detach();
    m_movie = std::nullopt;
}

Helper::LockedRef<std::string> CRecorder::GetErrorLog() {
    return {m_error_log, m_error_mutex};
}
void CRecorder::AppendErrorLog(std::string_view str)
{
    std::scoped_lock lock(m_error_mutex);
    m_error_log.append(str);
}
void CRecorder::ClearErrorLog()
{
    std::scoped_lock lock(m_error_mutex);
    m_error_log.clear();
}

int CRecorder::OnFrameStageNotify()
{
    ClientFrameStage_t stage = g_hk_client.Context()->curStage;

    if (stage == FRAME_START)
    {
        // Start/stop the movie
        if (m_is_recording != m_movie.has_value())
        {
            if (m_is_recording)
            {
                if (!SetupMovie())
                    StopMovie();
            }
            else
                CleanupMovie();
        }
        return 0;
    }

    if (stage != FRAME_RENDER_END)
        return 0;

    if (!m_movie)
        return 0;
    else if (m_movie->Failed())
    {
        StopMovie();
        CleanupMovie();
        return 0;
    }
    
    // Don't record unless a movie is started and the console is closed (TODO: and no loading screen)
    if (!ShouldRecordFrame())
        return 0;
    
    // We don't explicitly lock any mutex.
    // Assume that nothing is modified while recording.
    
    IMatRenderContext* render_ctx = Interfaces::mat_system->GetRenderContext();
    CViewSetup view_setup;
    Interfaces::hlclient->GetPlayerView(view_setup);

    // Render "empty" streams first.
    // The scene is already rendered, so we don't re-render.
    // This is sub-optimal for multiple empty streams, but having many identical streams is impractical anyway.
    bool first_stream = true;
    for (auto& pair : m_movie->GetStreams())
    {
        if (!pair.stream->GetRenderTweaks().empty())
            continue; // Stream has rendering tweaks
        
        if (first_stream)
            g_active_stream.Set(pair.stream);
        first_stream = false;

        auto frame = m_movie->GetFramePool().PopEmptyFrame();
        render_ctx->ReadPixels(
            0, 0, frame->buffer.GetWidth(), frame->buffer.GetHeight(), frame->buffer.GetData(), IMAGE_FORMAT_RGB888
        );
        m_movie->GetFramePool().PushFullFrame(frame, pair.writer);
    }
    
    // Next, render streams with tweaks in them.
    // The scene will have to be re-rendered appropriately.
    float default_fov = view_setup.fov;
    for (auto& pair : m_movie->GetStreams())
    {
        if (pair.stream->GetRenderTweaks().empty())
            continue; // Stream has no rendering tweaks
        
        float fov = default_fov;
        for (auto tweak = pair.stream->begin<CameraTweak>(); tweak != pair.stream->end<CameraTweak>(); ++tweak)
        {
            if (tweak->fov_override)
                fov = tweak->fov;
        }

        g_active_stream.Set(pair.stream);
        g_active_stream.SignalUpdate();
        // Update the materials right now, instead of waiting for the next frame.
        g_active_stream.UpdateMaterials();
        view_setup.fov = fov;
        Interfaces::hlclient->RenderView(view_setup, VIEW_CLEAR_COLOR, RENDERVIEW_DRAWVIEWMODEL | RENDERVIEW_DRAWHUD);

        auto frame = m_movie->GetFramePool().PopEmptyFrame();
        render_ctx->ReadPixels(
            0, 0, frame->buffer.GetWidth(), frame->buffer.GetHeight(), frame->buffer.GetData(), IMAGE_FORMAT_RGB888
        );
        m_movie->GetFramePool().PushFullFrame(frame, pair.writer);
    }

    return 0;
}

void CRecorder::ToggleRecording() {
    m_is_recording = !m_is_recording;
}
