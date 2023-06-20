#include "recorder.h"
#include <Hooks/ClientHook.h>
#include <Modules/Menu.h>
#include <SDK/cdll_int.h>
#include <Base/Interfaces.h>
#include <SDK/ienginetool.h>
#include <cstdint>

// Libraries for writing movie data
#include <spng.h>
#include <cstdio>
#include <sstream>
//

static CRecorder g_recorder;

void CRecorder::StartListening()
{
    Listen(EVENT_MENU, [this]{ return OnMenu(); });
    Listen(EVENT_FRAMESTAGENOTIFY, [this]{ return OnFrameStageNotify(); });
}

int CRecorder::OnMenu()
{
    static std::array<char, 512> input_movie_path = { 0 };

    if (ImGui::CollapsingHeader("Recording"))
    {
        ImGui::BeginGroup();

        if (ImGui::Button(IsRecordingMovie() ? "Stop" : "Start"))
            IsRecordingMovie() ? StopMovie() : StartMovie(input_movie_path.data());

        ImGui::Text("Output folder:");
        ImGui::InputText("", input_movie_path.data(), input_movie_path.size());

        ImGui::Text("PNG compression (higher compression is slower):");
        ImGui::SliderInt("", &m_png_compression_lvl, 0, 9);
        
        ImGui::Checkbox("Pause if menu is open", &m_pause_on_menu);
        
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

    m_movie_path = movie_path;
    if (m_movie_path.empty())
        return false;
    if (m_movie_path.back() != '/' && m_movie_path.back() != '\\')
        m_movie_path.push_back('/');
    
    m_is_recording = true;
    m_first_movie_error.clear();
    m_frame_index = 0;
    return true;
}

void CRecorder::StopMovie() {
    m_is_recording = false;
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
    {
        if (m_first_movie_error.empty())
        {
            sstream.str(""); // Clear stream
            sstream << "Failed writing frame to '" << frame_path << "'";
            m_first_movie_error = sstream.str();
        }
    }

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