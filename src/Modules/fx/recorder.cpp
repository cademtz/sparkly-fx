#include "recorder.h"
#include <Hooks/ClientHook.h>
#include <Modules/Menu.h>
#include <Base/Interfaces.h>
#include <Base/fnv1a.h>
#include <SDK/cdll_int.h>
#include <SDK/ienginetool.h>
#include <SDK/KeyValues.h>
#include <cstdint>
#include <direct.h>
#include <varargs.h>

// Libraries for writing movie data
#include <spng.h>
#include <cstdio>
#include <sstream>
//

static CRecorder g_recorder;

static std::string game_dir;
static std::string working_dir;
static std::array<char, 512> input_movie_path;

void CRecorder::StartListening()
{
    std::array<char, 512> buffer;

    Interfaces::engine_tool->GetGameDir(buffer.data(), buffer.size());
    game_dir = buffer.data();
    if (game_dir.back() != '/' || game_dir.back() != '\\')
        game_dir.push_back('/');

    if (!_getcwd(buffer.data(), buffer.size()))
        printf("Failed to get working directory");
    working_dir = buffer.data();
    if (working_dir.back() != '/' || working_dir.back() != '\\')
        working_dir.push_back('/');
    
    strncpy_s(input_movie_path.data(), input_movie_path.size(), game_dir.data(), game_dir.size());

    Listen(EVENT_MENU, [this]{ return OnMenu(); });
    Listen(EVENT_FRAMESTAGENOTIFY, [this]{ return OnFrameStageNotify(); });
}

int CRecorder::OnMenu()
{
    if (ImGui::CollapsingHeader("Recording"))
    {
        ImGui::BeginGroup();

        if (ImGui::Button(IsRecordingMovie() ? "Stop" : "Start"))
            IsRecordingMovie() ? StopMovie() : StartMovie(input_movie_path.data());

        ImGui::InputText("Output folder", input_movie_path.data(), input_movie_path.size());

        ImGui::Text("PNG compression (higher compression is slower):");
        ImGui::SliderInt("PNG compression", &m_png_compression_lvl, 0, 9);
        
        ImGui::Checkbox("Pause if menu is open", &m_pause_on_menu);

        ImGui::TextDisabled("Game directory: %s", game_dir.c_str());
        ImGui::TextDisabled("Working directory: %s", working_dir.c_str());
        
        if (!m_first_movie_error.empty())
            ImGui::Text("Error: %s", m_first_movie_error.c_str());

        ImGui::EndGroup();
    }
    return 0;
}

bool CRecorder::IsRecordingMovie() {
    return m_is_recording;
}

bool CRecorder::ShouldRecordFrame() {
    if (m_pause_on_menu && g_menu.IsOpen())
        return false;
    return IsRecordingMovie() && !Interfaces::engine->Con_IsVisible();
}

bool CRecorder::StartMovie(std::string_view movie_path) {
    if (IsRecordingMovie())
        return false;

    m_first_movie_error.clear();
    m_temp_audio_name.clear();
    m_movie_path.clear();
    m_frame_index = 0;

    // Ensure movie path ends with a slash
    m_movie_path = movie_path;
    if (m_movie_path.empty())
        return false;
    if (m_movie_path.back() != '/' && m_movie_path.back() != '\\')
        m_movie_path.push_back('/');
    
    // We use the engine to record the audio file, but it only accepts relative directories. So:
    // Make a temporary, relative audio file with a somewhat unique name. Then we can move it later.
    {
        std::array<char, 64> buf;
        snprintf(buf.data(), buf.size(), "audio-%X.wav", fnv::calculate_32(m_movie_path.c_str()));
        m_temp_audio_name = buf.data();
    }

    // Use engine_tool to record sound for us, otherwise we would need a signature to SND_StartMovie
    KeyValuesAD movie_params("movie_params");

    movie_params->SetString("filename", m_temp_audio_name.c_str());
    movie_params->SetInt("outputwav", 1);

    Interfaces::engine_tool->StartMovieRecording(movie_params);

    m_is_recording = true;
    return true;
}

void CRecorder::StopMovie() {
    m_is_recording = false;
    Interfaces::engine_tool->EndMovieRecording();
    
    // Move the audio file from its temp path to its official destination
    std::string new_audio_path = m_movie_path + "audio.wav";
    std::string old_audio_path = game_dir + m_temp_audio_name;
    if (rename(old_audio_path.c_str(), new_audio_path.c_str()))
        SetFirstMovieError("Failed to move temp audio file '%s'", m_temp_audio_name.c_str());
}

void CRecorder::SetFirstMovieError(const char* fmt, ...) {
    if (!m_first_movie_error.empty())
        return;
    
    m_first_movie_error.resize(1024);

    va_list args;
    va_start(args, fmt);
    vsnprintf(m_first_movie_error.data(), m_first_movie_error.size(), fmt, args);
    va_end(args);

    m_first_movie_error.resize(strlen(m_first_movie_error.c_str()));
}

void CRecorder::RecordNewFrame()
{
    int screen_w, screen_h;
    Interfaces::engine->GetScreenSize(screen_w, screen_h);

    FrameBufferRGB frame_buf(screen_w, screen_h);
    
    IMatRenderContext* render_ctx = Interfaces::mat_system->GetRenderContext();
    render_ctx->ReadPixels(0, 0, screen_w, screen_h, frame_buf.data, IMAGE_FORMAT_RGB888);

    std::stringstream sstream;
    sstream << m_movie_path << "frame_" << m_frame_index << ".png";

    std::string frame_path = sstream.str();

    if (!frame_buf.WritePNG(frame_path.c_str(), (uint8_t)m_png_compression_lvl))
        SetFirstMovieError("Failed writing frame to '%s'", frame_path.c_str());

    ++m_frame_index;
}

int CRecorder::OnFrameStageNotify()
{
    ClientFrameStage_t stage = g_hk_client.Context()->curStage;

    // Capture game during FRAME_RENDER_END (while rendering contexts haven't cleaned up)
    if (stage == FRAME_RENDER_END) {
        // Don't record unless a movie is started and the console is closed (TODO: and no loading screen)
        if (ShouldRecordFrame())
            RecordNewFrame();
    }

    return 0;
}

bool FrameBufferRGB::WritePNG(const char* file_name, uint8_t compression_level) const
{
    int err = 0;
    spng_ihdr ihdr = {0};
    constexpr auto close_file = [](FILE* f) { std::fclose(f); };
    FreeThis<FILE, close_file> file = std::fopen(file_name, "wb");
    ihdr.width = width;
    ihdr.height = height;
    ihdr.bit_depth = GetBitDepth();
    ihdr.color_type = SPNG_COLOR_TYPE_TRUECOLOR;

    if (!*file) {
        fprintf(stderr, __FUNCTION__ "() -> Failed to write PNG. Could not create file '%s'\n", file_name);
        return false;
    }

    FreeThis<spng_ctx, spng_ctx_free> ctx = spng_ctx_new(SPNG_CTX_ENCODER);

    spng_set_ihdr(*ctx, &ihdr);
    spng_set_png_file(*ctx, *file);
    spng_set_option(*ctx, SPNG_IMG_COMPRESSION_LEVEL, compression_level > 9 ? 9 : compression_level);

    err = spng_encode_image(*ctx, data, GetDataLength(), SPNG_FMT_PNG, SPNG_ENCODE_FINALIZE);

    if (err != 0)
    {
        fprintf(stderr, __FUNCTION__ "() -> Failed to write PNG. Error code %d\n", err);
        return false;
    }
    
    return true;
}