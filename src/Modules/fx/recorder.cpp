#include "recorder.h"
#include <imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <Hooks/ClientHook.h>
#include <Modules/Menu.h>
#include <Modules/Draw.h>
#include "ActiveStream.h"
#include "StreamEditor.h"
#include <Base/Interfaces.h>
#include <Base/fnv1a.h>
#include <SDK/cdll_int.h>
#include <SDK/ienginetool.h>
#include <SDK/KeyValues.h>
#include <SDK/itexture.h>
#include <SDK/ivrenderview.h>
#include <SDK/view_shared.h>
#include <cstdint>
#include <varargs.h>
#include <chrono>
#include <array>
#include <cassert>

// Libraries for threading/writing movie data 
#include <fstream>
#include <spng.h>
#define QOI_IMPLEMENTATION
#define QOI_NO_STDIO
#include <qoi.h>
#include <thread>
//

static const std::string DEFAULT_STREAM_NAME = "video";

static std::filesystem::path game_dir;
static std::filesystem::path working_dir;

CRecorder::CRecorder()
{
    // By default, allocate 1 frame for every thread/core.
    m_framepool_size = std::thread::hardware_concurrency();
    if (m_framepool_size < 1)
        m_framepool_size = 1;
    m_framepool_size += 1;
}

void CRecorder::StartListening()
{
    std::array<char, 512> buffer;
    std::error_code err;

    Interfaces::engine_tool->GetGameDir(buffer.data(), buffer.size());
    game_dir = buffer.data();
    working_dir = std::filesystem::current_path(err);
    if (err)
        printf("Failed to get working directory");
    
    m_input_movie_path = game_dir.string();

    Listen(EVENT_POST_IMGUI_INPUT, [this]{ return OnPostImguiInput(); });
    Listen(EVENT_DRAW, [this]{ return OnDraw(); });
    Listen(EVENT_MENU, [this]{ return OnMenu(); });
    Listen(EVENT_FRAMESTAGENOTIFY, [this]{ return OnFrameStageNotify(); });
}

int CRecorder::OnPostImguiInput()
{
    if (m_record_bind.Poll())
    {
        if (!g_menu.IsOpen() && !Interfaces::engine->Con_IsVisible())
            ToggleRecording();
    }
    return 0;
}

int CRecorder::OnDraw()
{
    if (IsRecordingMovie())
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
        ImGui::BeginGroup();

        if (ImGui::Button(IsRecordingMovie() ? "Stop" : "Start"))
            ToggleRecording();
        
        m_record_bind.OnMenu("Record hotkey");

        if (m_is_recording)
            ImGui::BeginDisabled();

        ImGui::InputText("Output folder", &m_input_movie_path);
        ImGui::SameLine();
        Helper::ImGuiHelpMarker(
            "This folder will contain the movie files.\n"
            "The folder will be created automatically, if it doesn't exist."
        );
        ImGui::InputInt("Framerate", &m_framerate);

        if (ImGui::BeginCombo("Video format", VideoFormatName(m_video_format)))
        {
            for (int fmt = 0; fmt < (int)VideoFormat::NUM_FORMATS; ++fmt)
            {
                ImGui::PushID(fmt);
                const bool item_selected = (VideoFormat)fmt == m_video_format;
                const char* item_text = VideoFormatName((VideoFormat)fmt);
                if (ImGui::Selectable(item_text, item_selected) && !item_selected)
                    m_video_format = (VideoFormat)fmt;
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone))
                    ImGui::SetTooltip(VideoFormatDesc((VideoFormat)fmt));
                if (item_selected)
                    ImGui::SetItemDefaultFocus();
                ImGui::PopID();
            }
            ImGui::EndCombo();
        }

        if (m_video_format == VideoFormat::PNG)
        {
            ImGui::Text("PNG compression (higher compression is slower):");
            ImGui::SliderInt("PNG compression", &m_png_compression_lvl, 0, 9);
        }


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
        
        ImGui::Checkbox("Pause if menu is open", &m_pause_on_menu);
        
        // Disabled text has less visual clutter
        ImGui::BeginDisabled();
        ImGui::Text("Framepool RAM: %f MB", framepool_ram);
        ImGui::Text("Framepool threads: %u", std::thread::hardware_concurrency());
        ImGui::TextWrapped("Game directory: %s", game_dir.string().c_str());
        ImGui::TextWrapped("Working directory: %s", working_dir.string().c_str());
        ImGui::EndDisabled();
        
        const char* movie_err = GetFirstMovieError();
        if (movie_err)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,0,1));
            ImGui::TextColored(ImVec4(1,1,0,1), "[Error] %s", movie_err);
            ImGui::PopStyleColor();
        }

        ImGui::EndGroup();
    }
    return 0;
}

bool CRecorder::IsRecordingMovie() {
    return m_is_recording;
}

bool CRecorder::ShouldRecordFrame()
{
    if (m_pause_on_menu && g_menu.IsOpen())
        return false;
    return IsRecordingMovie() && !Interfaces::engine->Con_IsVisible();
}

bool CRecorder::StartMovie(const std::string& path)
{
    if (IsRecordingMovie())
        return false;
    
    auto lock = g_active_rendercfg.ReadLock();

    m_first_movie_error.clear();
    m_movie_path.clear();
    m_frame_index = 0;

    if (path.empty())
    {
        SetFirstMovieError("Movie path was empty");
        return false;
    }
    m_movie_path = path;

    // Create the movie directory structure
    std::error_code err;
    std::filesystem::create_directory(m_movie_path, err);
    if (err)
    {
        SetFirstMovieError("Failed to create folder (%s): '%s'", err.message().c_str(), m_movie_path.string().c_str());
        return false;
    }

    if (g_render_frame_editor.GetStreams().empty()) // Default video folder
    {
        std::filesystem::path dir = m_movie_path / DEFAULT_STREAM_NAME;
        std::filesystem::create_directory(dir, err);
        if (err)
        {
            SetFirstMovieError("Failed to create folder: '%s'", dir.string().c_str());
            return false;
        }
    }
    else // Video folders named after each stream
    {
        for (auto stream : g_render_frame_editor.GetStreams())
        {
            std::filesystem::path dir = m_movie_path / stream->GetName();
            std::filesystem::create_directory(dir, err);
            if (err)
            {
                SetFirstMovieError("Failed to create folder: '%s'", dir.string().c_str());
                return false;
            }
        }
    }
    
    // We use the engine to record the audio file, but it only accepts relative directories. So:
    // Make a temporary, relative audio file with a somewhat unique name. Then we can move it later.
    {
        auto time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch());
        m_temp_audio_name = "audio-" + std::to_string(time.count()) + ".wav";
    }

    // Use engine_tool to record sound for us, otherwise we would need a signature to SND_StartMovie
    KeyValuesAD movie_params("movie_params");

    movie_params->SetString("filename", m_temp_audio_name.c_str());
    movie_params->SetInt("outputwav", 1);
    movie_params->SetFloat("framerate", m_framerate); // This one is a float. Dunno why.

    int screen_w, screen_h;
    Interfaces::engine->GetScreenSize(screen_w, screen_h);

    m_framepool = std::make_shared<FramePool>(
        std::thread::hardware_concurrency(), m_framepool_size,
        m_video_format, m_png_compression_lvl,
        screen_w, screen_h
    );
    Interfaces::engine_tool->StartMovieRecording(movie_params);
    m_is_recording = true;
    return true;
}

void CRecorder::StopMovie()
{
    m_is_recording = false;
    Interfaces::engine_tool->EndMovieRecording();
    m_framepool->Finish();
    g_active_rendercfg.Set(nullptr);
    
    // Move the audio file from its temp path to its official destination.
    // FIXME: Sometimes this fails. I don't know why yet. Maybe the engine is still writing to it?
    std::filesystem::path new_audio_path = m_movie_path / "audio.wav";
    std::filesystem::path old_audio_path = game_dir / m_temp_audio_name;
    std::error_code err;
    std::filesystem::rename(old_audio_path, new_audio_path, err);
    if (err)
    {
        SetFirstMovieError("Failed to move temp audio file (%s): '%s' -> '%s'",
            err.message().c_str(),
            old_audio_path.string().c_str(),
            new_audio_path.string().c_str());
    }
}

void CRecorder::SetFirstMovieError(const char* fmt, ...)
{
    std::lock_guard lock{m_mutex_error};
    if (!m_first_movie_error.empty())
        return;
    
    m_first_movie_error.resize(1024);

    va_list args;
    va_start(args, fmt);
    vsnprintf(m_first_movie_error.data(), m_first_movie_error.size(), fmt, args);
    va_end(args);

    m_first_movie_error.resize(strlen(m_first_movie_error.c_str()));
}

const char* CRecorder::GetFirstMovieError()
{
    std::lock_guard lock{m_mutex_error};
    if (m_first_movie_error.empty())
        return nullptr;
    return m_first_movie_error.c_str();
}

void CRecorder::WriteFrame(const std::string& stream_name)
{
    FramePool::FramePtr frame = m_framepool->PopEmptyFrame();
    if (!frame) // We're done recording
        return;
    
    {
        std::string frame_name = "frame_" + std::to_string(m_frame_index) + '.' + VideoFormatName(m_video_format);
        frame->path = m_movie_path / stream_name / frame_name;
    }

    IMatRenderContext* render_ctx = Interfaces::mat_system->GetRenderContext();
    render_ctx->ReadPixels(
        0, 0, frame->buffer.GetWidth(), frame->buffer.GetHeight(), frame->buffer.GetData(), IMAGE_FORMAT_RGB888
    );
    
    m_framepool->PushFullFrame(frame);
}

int CRecorder::OnFrameStageNotify()
{
    ClientFrameStage_t stage = g_hk_client.Context()->curStage;

    // Capture game during FRAME_RENDER_END (while rendering contexts haven't cleaned up)
    if (stage == FRAME_RENDER_END)
    {
        // Don't record unless a movie is started and the console is closed (TODO: and no loading screen)
        if (ShouldRecordFrame())
        {
            // We don't explicitly lock any mutex.
            // Assume that nothing is modified while recording.
            if (g_render_frame_editor.GetStreams().empty())
                WriteFrame(DEFAULT_STREAM_NAME);
            else
            {
                // TODO: Get rid of this test struct
                struct
                {
                    CViewSetup view_setup;
                } dummy;
                Interfaces::hlclient->GetPlayerView(dummy.view_setup);
                float default_fov = dummy.view_setup.fov;
                
                for (auto config : g_render_frame_editor.GetStreams())
                {
                    float fov = default_fov;
                    for (auto tweak = config->begin<CameraTweak>(); tweak != config->end<CameraTweak>(); ++tweak)
                    {
                        if (tweak->fov_override)
                            fov = tweak->fov;
                    }

                    g_active_rendercfg.Set(config);
                    // Update the materials right now, instead of waiting for the next frame.
                    g_active_rendercfg.UpdateMaterials();
                    dummy.view_setup.fov = fov;
                    Interfaces::hlclient->RenderView(dummy.view_setup, VIEW_CLEAR_DEPTH, RENDERVIEW_DRAWVIEWMODEL | RENDERVIEW_DRAWHUD);
                    WriteFrame(config->GetName());
                }
            }

            ++m_frame_index;
        }
    }

    return 0;
}

bool FrameBufferRGB::WritePNG(std::ostream& output, int compression_level) const
{
    auto read_write_fn = [](spng_ctx *ctx, void *user, void *data, size_t n)
    {
        std::ostream& output = *(std::ostream*)user;
        output.write((const char*)data, n);
        return output ? 0 : SPNG_IO_ERROR;
    };

    int err = 0;
    spng_ihdr ihdr = {0};
    ihdr.width = GetWidth();
    ihdr.height = GetHeight();
    ihdr.bit_depth = GetBitDepth();
    ihdr.color_type = SPNG_COLOR_TYPE_TRUECOLOR;

    FreeThis<spng_ctx, spng_ctx_free> ctx = spng_ctx_new(SPNG_CTX_ENCODER);

    spng_set_ihdr(*ctx, &ihdr);
    spng_set_png_stream(*ctx, read_write_fn, (void*)&output);
    spng_set_option(*ctx, SPNG_IMG_COMPRESSION_LEVEL, compression_level > 9 ? 9 : compression_level);

    err = spng_encode_image(*ctx, GetData(), GetDataLength(), SPNG_FMT_PNG, SPNG_ENCODE_FINALIZE);

    if (err != 0)
    {
        fprintf(stderr, __FUNCTION__ "() -> Failed to write PNG. Error code %d\n", err);
        return false;
    }
    
    return true;
}

bool FrameBufferRGB::WriteQOI(std::ostream& output) const
{
    qoi_desc desc;
    desc.channels = NUM_CHANNELS;
    desc.colorspace = QOI_SRGB;
    desc.width = GetWidth();
    desc.height = GetHeight();

    // TODO: Redefine QOI_MALLOC to allocate from a reuseable vector.
    //  It will have to be thread-safe.

    int encoded_len;
    void* encoded = qoi_encode(m_data, &desc, &encoded_len);
    if (!encoded)
        return false;
    
    output.write((const char*)encoded, encoded_len);
    free(encoded);
    return true;
}

const char* CRecorder::VideoFormatName(VideoFormat format)
{
    const char* const table[] = {
        "qoi",
        "png",
    };
    return table[(int)format];
}

const char* CRecorder::VideoFormatDesc(VideoFormat format)
{
    const char* const table[] = {
        "QOI image sequence\nLossless compression\nFast\n(https://qoiformat.org/)",
        "PNG image sequence\nLossless compression\nSlow",
    };
    return table[(int)format];
}

FramePool::FramePool(
    size_t num_threads, size_t num_frames,
    CRecorder::VideoFormat format, int png_compression,
    uint32_t frame_width, uint32_t frame_height
) : m_format(format), m_png_compression(png_compression)
{
    assert(num_frames > 0 && "Frame pool must contain at least 1 frame");

    m_threads.reserve(num_threads);
    m_all.reserve(num_frames);
    m_full.reserve(num_frames);
    m_empty.reserve(num_frames);

    for (size_t i = 0; i < num_frames; ++i)
    {
        m_all.push_back(std::make_shared<Frame>(frame_width, frame_height));
        m_empty.push_back(m_all.back());
    }

    for (size_t i = 0; i < num_threads; ++i)
        m_threads.emplace_back(&WorkerLoop, this);
}

void FramePool::Finish()
{
    {
        std::lock_guard lock{m_mutex};
        m_all.clear();
        m_empty.clear();
    }
    m_cv_empty.notify_all();
    m_cv_full.notify_all();
    
    for (auto& thread : m_threads)
        thread.join();
    m_threads.clear();
}

void FramePool::PushFullFrame(FramePtr frame)
{
    assert(!frame->path.empty() && "Frame path must be set before call to PushFullFrame()");
    {
        std::lock_guard lock{m_mutex};
        assert((m_all.empty() || m_full.size() < m_all.size()) && "More frames are being pushed than exists in the pool");
        m_full.push_back(frame);
    }
    m_cv_full.notify_one();
}

FramePool::FramePtr FramePool::PopFullFrame()
{
    while (true)
    {
        FramePtr back;
        {
            std::unique_lock lock{m_mutex};
            m_cv_full.wait(lock, [this]{ return m_all.empty() || !m_full.empty(); });
            if (m_all.empty())
                return nullptr;

            back = m_full.back();
            m_full.pop_back();
        }
        m_cv_full.notify_one();
        return back;
    }
}

void FramePool::PushEmptyFrame(FramePtr frame)
{
    frame->path.clear();

    {
        std::lock_guard lock{m_mutex};
        assert((m_all.empty() || m_empty.size() < m_all.size()) && "More frames are being pushed than exists in the pool");
        m_empty.push_back(frame);
    }
    m_cv_empty.notify_one();
}

FramePool::FramePtr FramePool::PopEmptyFrame()
{
    while (true)
    {
        FramePtr back;
        {
            std::unique_lock lock{m_mutex};
            m_cv_empty.wait(lock, [this]{ return m_all.empty() || !m_empty.empty(); });
            if (m_all.empty())
                return nullptr;

            back = m_empty.back();
            m_empty.pop_back();
        }
        m_cv_empty.notify_one();
        return back;
    }
}

void FramePool::WorkerLoop(FramePool* pool)
{
    while (FramePtr frame = pool->PopFullFrame())
    {
        std::fstream file = std::fstream(frame->path, std::ios::out | std::ios::binary);
        if (!file)
        {
            g_recorder.SetFirstMovieError("Failed to open file for writing: '%s'", frame->path.string().c_str());
            pool->PushEmptyFrame(frame);
            break;
        }
        
        bool result = false;
        switch (pool->m_format)
        {
        case CRecorder::VideoFormat::PNG: result = frame->buffer.WritePNG(file, pool->m_png_compression); break;
        case CRecorder::VideoFormat::QOI: result = frame->buffer.WriteQOI(file); break;
        default:
            assert(0 && "Unknown VideoFormat. A switch case may be missing.");
        }
        
        pool->PushEmptyFrame(frame);
        if (!file)
            g_recorder.SetFirstMovieError("Failure while writing frame: '%s'", frame->path.string().c_str());
        else if (!result)
            g_recorder.SetFirstMovieError("Failed to encode frame: '%s'", frame->path.string().c_str());
        if (!file || !result)
            break;
    }
}