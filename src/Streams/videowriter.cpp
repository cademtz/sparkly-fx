#include "videowriter.h"
#include <Hooks/OverlayHook.h>
#include <Helper/defer.h>
#include <Helper/imgui.h>
#include <Helper/ffmpeg.h>
#include <ffmpipe/ffmpipe.h>
#include <fstream>
#include <sstream>
#include <cassert>
#include <cstdio>
#include <spng.h>
#define QOI_IMPLEMENTATION
#define QOI_NO_STDIO
#include <qoi.h>

static const COMDLG_FILTERSPEC COM_EXE_FILTER[] = {{L"Executable", L"*.exe"}, {0}};

static EncoderConfig::TypeDesc type_descs[] =
{
    {"qoi",     "Image sequence with fast, lossless compression"},
    {"png",     "Image sequence with slower, lossless compression"},
    {"ffmpeg",  "Any video format, fast or slow, lossless or lossy"},
};

const EncoderConfig::TypeDesc* EncoderConfig::TYPE_QOI = &type_descs[0];
const EncoderConfig::TypeDesc* EncoderConfig::TYPE_PNG = &type_descs[1];
const EncoderConfig::TypeDesc* EncoderConfig::TYPE_FFMPEG = &type_descs[2];

const EncoderConfig::TypeDesc* EncoderConfig::Types() {
    return type_descs;
}
size_t EncoderConfig::NumTypes() {
    return sizeof(type_descs) / sizeof(type_descs[0]);
}

void EncoderConfig::ShowImguiControls()
{
    ImGui::InputInt("Framerate", &framerate);

    if (ImGui::BeginCombo("Video format", type->name))
    {
        for (size_t i = 0; i < EncoderConfig::NumTypes(); ++i)
        {
            const EncoderConfig::TypeDesc* type_desc = &EncoderConfig::Types()[i];
            ImGui::PushID(i);
            if (ImGui::Selectable(type_desc->name, type == type_desc))
                type = type_desc;
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone))
                ImGui::SetTooltip("%s", type_desc->desc);
            // TODO: Why was this necessary? Remove if safe.
            //if (item_selected)
            //    ImGui::SetItemDefaultFocus();
            ImGui::PopID();
        }
        ImGui::EndCombo();
    }

    if (type == EncoderConfig::TYPE_PNG)
    {
        ImGui::SliderInt("Compression", &png_compression, 0, 9, "%d", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SameLine();
        Helper::ImGuiHelpMarker("Higher compression is slower to render, but creates a smaller file");
    }
    else if (type == EncoderConfig::TYPE_FFMPEG)
    {
        static auto ffmpeg_path_list = Helper::FFmpeg::ScanForExecutables();
        bool is_initialized = false;
        if (!is_initialized) // Find a good default FFmpeg path
        {
            is_initialized = true;
            if (Helper::FFmpeg::GetDefaultPath().empty() && !ffmpeg_path_list.empty())
                Helper::FFmpeg::SetDefaultPath(ffmpeg_path_list.front());
        }

        std::filesystem::path ffmpeg_path = Helper::FFmpeg::GetDefaultPath();
        bool has_ffmpeg = !ffmpeg_path.empty();
        const char* settings_label = has_ffmpeg ? "FFmpeg settings###ffmpeg_settings" : "[!] FFmpeg settings###ffmpeg_settings";
        if (!has_ffmpeg) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,.25,.25,1));
        bool settings_tree = ImGui::TreeNode(settings_label);
        if (!has_ffmpeg) ImGui::PopStyleColor();

        if (settings_tree)
        {
            if (has_ffmpeg)
            {
                ImGui::InputText("Args", &ffmpeg_output_args); ImGui::SameLine();
                Helper::ImGuiHelpMarker(
                    "Output args for FFmpeg, excluding the output file name.\n"
                    "The args are appended after the input flag.\n"
                    "Examples:\n"
                    "- 20 MB bitrate:           -b:v 20M\n"
                    "- H.264 (Default):         -c:v libx264\n"
                    "- H.264 (Nvidia GPU):      -c:v h264_nvenc\n"
                    "- H.264 (AMD GPU):         -c:v h264_amf\n"
                    "- HEVC (Default):          -c:v libx265\n"
                    "- HEVC (Nvidia GPU):       -c:v hevc_nvenc\n"
                    "- HEVC (AMD GPU):          -c:v hevc_amf\n"
                    "- HuffYUV:                 -c:v huffyuv\n"
                    "- UTVideo:                 -c:v utvideo\n"
                    "Notes:\n"
                    "- Avoid the default H.264 and HEVC codecs. They're slow.\n"
                    "- HuffYUV and UTVideo require an AVI file.\n"
                    "- H.264 with 40+ MB bitrate is practically lossless and super tiny."
                );
                ImGui::InputText("File format", &ffmpeg_output_ext); ImGui::SameLine();
                Helper::ImGuiHelpMarker(
                    "The file extension/container/format.\n"
                    "Examples: 'mkv', 'mp4', 'avi'.\n"
                    "Each container will support different codecs:\n"
                    "- AVI supports HuffYUV and UTVideo"
                );
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,.25,.25,1));
                ImGui::Text("[!] Select an FFmpeg executable");
                ImGui::PopStyleColor();
            }
            
            ImGui::Text("FFmpeg executable:");
            ImGui::InputText("##ffmpeg_path", &ffmpeg_path.string(), ImGuiInputTextFlags_ReadOnly);
            ImGui::SameLine();
            if (ImGui::Button("Browse"))
            {
                auto optional_path = Helper::OpenFileDialog(L"Select an FFmpeg executable", nullptr, COM_EXE_FILTER);
                if (optional_path)
                    Helper::FFmpeg::SetDefaultPath(std::move(*optional_path));
            }

            if (ImGui::BeginListBox("Executables", Helper::CalcListBoxSize(ffmpeg_path_list.size())))
            {
                for (const auto& path : ffmpeg_path_list)
                {
                    if (ImGui::Selectable(path.u8string().c_str(), false))
                        Helper::FFmpeg::SetDefaultPath(path);
                }
                ImGui::EndListBox();
            }
            ImGui::TreePop();
        }
    }
}

bool ImageWriter::WriteFrame(const FrameBufferDx9& buffer, size_t frame_index)
{
    const wchar_t* file_extension;
    switch (m_file_format)
    {
    case Format::PNG: file_extension = L"png"; break;
    case Format::QOI: file_extension = L"qoi"; break;
    default:
        assert(0 && "Unknown format. A switch case may be missing.");
    }

    auto suffix = std::to_wstring(frame_index) + L'.' + file_extension;

    std::filesystem::path path = m_base_path.wstring() + suffix;
    std::fstream file = std::fstream(path, std::ios::out | std::ios::binary);
    if (!file)
    {
        //g_recorder.SetFirstMovieError("Failed to open file for writing: '%s'", path.string().c_str());
        return false;
    }
    
    bool result = false;
    switch (m_file_format)
    {
    case Format::PNG: result = WritePNG(buffer.ToRgb(), file, m_png_compression); break;
    case Format::QOI: result = WriteQOI(buffer.ToRgb(), file); break;
    }

    return true;
}

bool ImageWriter::WritePNG(const FrameBufferRgb& buffer, std::ostream& output, int compression)
{
    auto write_fn = [](spng_ctx *ctx, void *user, void *data, size_t n)
    {
        std::ostream& output = *(std::ostream*)user;
        output.write((const char*)data, n);
        return output ? 0 : SPNG_IO_ERROR;
    };

    int err = 0;
    spng_ihdr ihdr = {0};
    ihdr.width = buffer.GetWidth();
    ihdr.height = buffer.GetHeight();
    ihdr.bit_depth = buffer.GetFormatInfo().bitdepth[0];
    ihdr.color_type = SPNG_COLOR_TYPE_TRUECOLOR;

    spng_ctx* ctx = spng_ctx_new(SPNG_CTX_ENCODER);
    defer { spng_ctx_free(ctx); };

    spng_set_ihdr(ctx, &ihdr);
    spng_set_png_stream(ctx, write_fn, (void*)&output);
    spng_set_option(ctx, SPNG_IMG_COMPRESSION_LEVEL, compression > 9 ? 9 : compression);

    err = spng_encode_image(ctx, buffer.GetData(), buffer.GetDataLength(), SPNG_FMT_PNG, SPNG_ENCODE_FINALIZE);

    if (err != 0)
    {
        fprintf(stderr, __FUNCTION__ "() -> Failed to write PNG. Error code %d\n", err);
        return false;
    }
    
    return true;
}

bool ImageWriter::WriteQOI(const FrameBufferRgb& buffer, std::ostream& output)
{
    qoi_desc desc;
    desc.channels = buffer.GetFormatInfo().num_channels;
    desc.colorspace = QOI_SRGB;
    desc.width = buffer.GetWidth();
    desc.height = buffer.GetHeight();

    // TODO: Redefine QOI_MALLOC to allocate from a reuseable vector.
    //  It will have to be thread-safe.

    int encoded_len;
    void* encoded = qoi_encode(buffer.GetData(), &desc, &encoded_len);
    if (!encoded)
        return false;
    
    output.write((const char*)encoded, encoded_len);
    free(encoded);
    return true;
}

const Helper::D3DFORMAT_info& FrameBufferRgb::GetFormatInfo() const
{
    static const Helper::D3DFORMAT_info info = {
        3,          // stride
        {8,8,8},    // bitdepth
        3,          // num_channels
        true,       // is_uniform_bitdepth
        false,      // is_float
        false,      // is_depth
    };
    return info;
}

FrameBufferDx9::FrameBufferDx9(IDirect3DSurface9* surface)
{
    D3DSURFACE_DESC desc;
    if (FAILED(surface->GetDesc(&desc)))
        assert(0 && "Failed to get D3D surface desc");

    m_width = desc.Width;
    m_height = desc.Height;
    m_d3dformat = desc.Format;
    m_d3dsurface = surface;
    if (!Helper::GetD3DFormatInfo(m_d3dformat, &m_d3dformat_info))
        assert(0 && "Invalid or unsupported D3DFORMAT");
}

FrameBufferDx9::FrameBufferDx9(uint32_t width, uint32_t height, D3DFORMAT d3dformat)
{
    m_width = width;
    m_height = height;
    m_d3dformat = d3dformat;
    if (!Helper::GetD3DFormatInfo(m_d3dformat, &m_d3dformat_info))
        assert(0 && "Invalid or unsupported D3DFORMAT");
    
    HRESULT result = g_hk_overlay.Device()->CreateRenderTarget(
        m_width,
        m_height,
        m_d3dformat,
        D3DMULTISAMPLE_NONE,
        0,      // MultisampleQuality
        TRUE,   // Lockable
        &m_d3dsurface,
        nullptr // pSharedhandle
    );
    if (FAILED(result))
        assert(0 && "Failed to create render target with desired D3DFORMAT");
}

FrameBufferRgb FrameBufferDx9::ToRgb() const
{
    assert(m_d3dsurface && "Missing a valid surface pointer");
    FrameBufferRgb frame{GetWidth(), GetHeight()};
    D3DLOCKED_RECT locked_rect;
    if (FAILED(m_d3dsurface->LockRect(&locked_rect, nullptr, D3DLOCK_READONLY)))
        assert(0 && "Failed to lock D3D surface rect");
    defer { m_d3dsurface->UnlockRect(); };
    
    switch (m_d3dformat)
    {
    case Helper::D3DFMT_B8G8R8: BlitRgb(&frame, locked_rect); break;
    case D3DFMT_R8G8B8: BlitBgr(&frame, locked_rect); break;
    case D3DFMT_A8B8G8R8: BlitRgba(&frame, locked_rect); break;
    case D3DFMT_A8R8G8B8: BlitBgra(&frame, locked_rect); break;
    default:
        assert(0 && "Blitting is not implemented for this D3DFORMAT");
    }

    return frame;
}

void FrameBufferDx9::BlitRgb(FrameBufferRgb* dst, D3DLOCKED_RECT src)
{
    uint32_t width = dst->GetWidth(), height = dst->GetHeight();
    size_t dst_stride = dst->GetPixelStride();
    const size_t src_stride = 3;

    for (uint32_t y = 0; y < height; ++y)
    {
        size_t row_size = dst->GetPixelStride() * dst->GetWidth();
        uint8_t* dst_row = dst->GetData() + y * row_size;
        const uint8_t* src_row = (const uint8_t*)src.pBits + y * src.Pitch;
        memcpy(dst_row, src_row, row_size);
    }
}
void FrameBufferDx9::BlitBgr(FrameBufferRgb* dst, D3DLOCKED_RECT src)
{
    uint32_t width = dst->GetWidth(), height = dst->GetHeight();
    size_t dst_stride = dst->GetPixelStride();
    const size_t src_stride = 3;

    for (uint32_t y = 0; y < height; ++y)
    {
        for (uint32_t x = 0; x < width; ++x)
        {
            uint8_t* dst_pixel = dst->GetData() + (y * width + x) * dst->GetPixelStride();
            const uint8_t* src_pixel = (const uint8_t*)src.pBits + y * src.Pitch + x * src_stride;
            dst_pixel[0] = src_pixel[2];
            dst_pixel[1] = src_pixel[1];
            dst_pixel[2] = src_pixel[0];
        }
    }
}
void FrameBufferDx9::BlitRgba(FrameBufferRgb* dst, D3DLOCKED_RECT src)
{
    uint32_t width = dst->GetWidth(), height = dst->GetHeight();
    size_t dst_stride = dst->GetPixelStride();
    const size_t src_stride = 4;

    for (uint32_t y = 0; y < height; ++y)
    {
        for (uint32_t x = 0; x < width; ++x)
        {
            uint8_t* dst_pixel = dst->GetData() + (y * width + x) * dst->GetPixelStride();
            const uint8_t* src_pixel = (const uint8_t*)src.pBits + y * src.Pitch + x * src_stride;
            dst_pixel[0] = src_pixel[0];
            dst_pixel[1] = src_pixel[1];
            dst_pixel[2] = src_pixel[2];
        }
    }
}
void FrameBufferDx9::BlitBgra(FrameBufferRgb* dst, D3DLOCKED_RECT src)
{
    uint32_t width = dst->GetWidth(), height = dst->GetHeight();
    size_t dst_stride = dst->GetPixelStride();
    const size_t src_stride = 4;

    for (uint32_t y = 0; y < height; ++y)
    {
        for (uint32_t x = 0; x < width; ++x)
        {
            uint8_t* dst_pixel = dst->GetData() + (y * width + x) * dst->GetPixelStride();
            const uint8_t* src_pixel = (const uint8_t*)src.pBits + y * src.Pitch + x * src_stride;
            dst_pixel[0] = src_pixel[2];
            dst_pixel[1] = src_pixel[1];
            dst_pixel[2] = src_pixel[0];
        }
    }
}

FFmpegWriter::FFmpegWriter(uint32_t width, uint32_t height, uint32_t framerate, const std::string& output_args, const std::filesystem::path& output_path)
{
    const char* pix_fmt = Helper::GetD3DFormatAsFFmpegPixFmt(D3DFMT_A8R8G8B8, true);
    assert(pix_fmt && "No equivalent FFmpeg pix_fmt for given D3DFORMAT");

    std::wstringstream ffmpeg_args;
    ffmpeg_args << "-y -c:v rawvideo -f rawvideo -pix_fmt " << pix_fmt << " -s:v " << width << 'x' << height << " -framerate " << framerate << ' ';
    ffmpeg_args << "-i - ";
    ffmpeg_args << output_args.c_str() << " \"" << output_path << '"';

    m_pipe = ffmpipe::Pipe::Create(Helper::FFmpeg::GetDefaultPath(), ffmpeg_args.str());
}

FFmpegWriter::~FFmpegWriter()
{
    if (m_pipe)
    {
        // Ensure the termination of FFmpeg without hanging the application.
        std::thread(
            [](std::shared_ptr<ffmpipe::Pipe> pipe) { pipe->Close(60'000, true); },
            m_pipe
        ).detach();
    }
}

bool FFmpegWriter::WriteFrame(const FrameBufferDx9& buffer, size_t frame_index)
{
    if (!m_pipe)
        return false;
    IDirect3DSurface9* d3dsurface = buffer.GetSurface();
    D3DLOCKED_RECT locked_rect;
    if (FAILED(d3dsurface->LockRect(&locked_rect, nullptr, D3DLOCK_READONLY)))
        return false;
    defer { d3dsurface->UnlockRect(); };
    for (uint32_t row = 0; row < buffer.GetHeight(); ++row)
    {
        const uint8_t* data = (const uint8_t*)locked_rect.pBits + row * locked_rect.Pitch;
        if (!m_pipe->Write(data, buffer.GetPixelStride() * buffer.GetWidth()))
            return false;
    }
    return true;
}

FramePool::FramePool(
    size_t num_threads, size_t num_frames,
    uint32_t frame_width, uint32_t frame_height
) {
    assert(num_frames > 0 && "Frame pool must contain at least 1 frame");

    m_threads.reserve(num_threads);
    m_all.reserve(num_frames);
    m_full.reserve(num_frames);
    m_empty.reserve(num_frames);

    IDirect3DSurface9* render_target;
    D3DSURFACE_DESC surface_desc;
    if (FAILED(g_hk_overlay.Device()->GetRenderTarget(0, &render_target)))
        assert(0 && "Failed to get render target");
    defer { render_target->Release(); };
    if (FAILED(render_target->GetDesc(&surface_desc)))
        assert(0 && "Failed to get the render target's surface desc");

    // Fill the empty-frame pool with empty frames
    for (size_t i = 0; i < num_frames; ++i)
    {
        m_all.push_back(std::make_shared<Frame>(FrameBufferDx9(frame_width, frame_height, D3DFMT_A8R8G8B8)));
        m_empty.push_back(m_all.back());
    }

    // Create worker threads (without running them yet)
    for (size_t i = 0; i < num_threads; ++i)
        m_threads.emplace_back(&WorkerLoop, this);
}

void FramePool::Close()
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

void FramePool::PushFullFrame(FramePtr frame, size_t index, std::shared_ptr<VideoWriter> writer)
{
    assert(writer != nullptr && "A writer must be provided to write the frame");
    assert(frame->buffer.GetWidth() * frame->buffer.GetHeight() != 0 && "Frame buffer must have non-zero size. Resize the buffer before use.");

    if (!writer->IsAsync())
    {
        // HACK: Write the frame and block to prevent synchronization issues.
        // FIXME: This can break if frames are pushed out-of-order.
        //   This also causes blockage, preventing async frames being pushed.
        //   Perhaps store an index for each synchronous writer. Then only submit frames
        writer->WriteFrame(frame->buffer, index);
        PushEmptyFrame(frame);
        return;
    }

    frame->writer = writer;
    frame->index = index;

    {
        std::lock_guard lock{m_mutex};
        assert((m_all.empty() || m_full.size() < m_all.size()) && "More frames are being pushed than exists in the pool");
        m_full.push_back(frame);
    }
    m_cv_full.notify_one();
}

FramePool::FramePtr FramePool::PopFullFrame()
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

void FramePool::PushEmptyFrame(FramePtr frame)
{
    frame->writer = nullptr;

    {
        std::lock_guard lock{m_mutex};
        assert((m_all.empty() || m_empty.size() < m_all.size()) && "More frames are being pushed than exists in the pool");
        m_empty.push_back(frame);
    }
    m_cv_empty.notify_one();
}

FramePool::FramePtr FramePool::PopEmptyFrame()
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

void FramePool::WorkerLoop(FramePool* pool)
{
    while (FramePtr frame = pool->PopFullFrame())
    {
        std::shared_ptr<VideoWriter> writer = frame->writer;
        assert(writer != nullptr && "frame->writer must be assigned before calling PushFullFrame");

        bool result = writer->WriteFrame(frame->buffer, frame->index);
        pool->PushEmptyFrame(frame);
        if (!result)
            break;
    }
}