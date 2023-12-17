#include "videowriter.h"
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
                    "Output args for FFmpeg, not including the output file.\n"
                    "These are appended after the input flag."
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

bool ImageWriter::WriteFrame(const FrameBufferRGB& buffer, size_t frame_index)
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
    case Format::PNG: result = WritePNG(buffer, file, m_png_compression); break;
    case Format::QOI: result = WriteQOI(buffer, file); break;
    }

    return true;
}

bool ImageWriter::WritePNG(const FrameBufferRGB& buffer, std::ostream& output, int compression)
{
    auto read_write_fn = [](spng_ctx *ctx, void *user, void *data, size_t n)
    {
        std::ostream& output = *(std::ostream*)user;
        output.write((const char*)data, n);
        return output ? 0 : SPNG_IO_ERROR;
    };

    int err = 0;
    spng_ihdr ihdr = {0};
    ihdr.width = buffer.GetWidth();
    ihdr.height = buffer.GetHeight();
    ihdr.bit_depth = buffer.GetBitDepth();
    ihdr.color_type = SPNG_COLOR_TYPE_TRUECOLOR;

    spng_ctx* ctx = spng_ctx_new(SPNG_CTX_ENCODER);
    defer { spng_ctx_free(ctx); };

    spng_set_ihdr(ctx, &ihdr);
    spng_set_png_stream(ctx, read_write_fn, (void*)&output);
    spng_set_option(ctx, SPNG_IMG_COMPRESSION_LEVEL, compression > 9 ? 9 : compression);

    err = spng_encode_image(ctx, buffer.GetData(), buffer.GetDataLength(), SPNG_FMT_PNG, SPNG_ENCODE_FINALIZE);

    if (err != 0)
    {
        fprintf(stderr, __FUNCTION__ "() -> Failed to write PNG. Error code %d\n", err);
        return false;
    }
    
    return true;
}

bool ImageWriter::WriteQOI(const FrameBufferRGB& buffer, std::ostream& output)
{
    qoi_desc desc;
    desc.channels = buffer.NUM_CHANNELS;
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

FFmpegWriter::FFmpegWriter(uint32_t width, uint32_t height, uint32_t framerate, const std::string& output_args, std::filesystem::path&& output_path)
{
    std::wstringstream ffmpeg_args;
    ffmpeg_args << "-c:v rawvideo -f rawvideo -pix_fmt rgb24 -s:v " << width << 'x' << height << " -framerate " << framerate << ' ';
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

bool FFmpegWriter::WriteFrame(const FrameBufferRGB& buffer, size_t frame_index)
{
    if (!m_pipe)
        return false;
    return m_pipe->Write(buffer.GetData(), buffer.GetDataLength());
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

    // Initialize all frames' image buffers, and put them in the empty pool
    for (size_t i = 0; i < num_frames; ++i)
    {
        m_all.push_back(std::make_shared<Frame>(frame_width, frame_height));
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