#pragma once
#include <filesystem>
#include <cstdint>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string_view>
#include <vector>
#include <unordered_set>
#include <Helper/d3d9.h>
#include <Helper/threading.h>
#include <Helper/str.h>
#include <Helper/json.h>

class FrameBufferDx9;
class FrameBufferRgb;
namespace ffmpipe { class Pipe; }

/**
 * @brief A global, thread-safe video log
 */
class VideoLog
{
public:
    struct ConsoleQueue
    {
        void Append(std::string text)
        {
            std::scoped_lock lock(m_mutex);
            m_queue.push_back(std::move(text));
        }

        void ExecuteAndClear();

        void Clear()
        {
            std::scoped_lock lock(m_mutex);
            m_queue.clear();
        }
    private:
        friend VideoLog;
        explicit ConsoleQueue() = default;

        std::vector<std::string> m_queue;
        std::mutex m_mutex;
    };

    static void Append(const std::string& text);

    /// @brief Append text to the error log. Newlines should be added.
    static void AppendError(const std::string& text);
    /// @brief Append text to the error log. Newlines should be added.
    static void AppendError(std::string_view text) {
        return AppendError(std::string(text));
    }
    /// @brief Append text to the error log. Newlines should be added.
    template <class... TArgs>
    static void AppendError(const char* fmt, TArgs&&... args) {
        AppendError(Helper::sprintf(fmt, std::forward<TArgs>(args)...));
    }
    static void Clear();
    static Helper::LockedRef<std::string> GetLog() { return Helper::LockedRef<std::string>(log, mutex); }
    /// @brief Lines are appended here to be removed and printed by the recorder in the game thread
    static ConsoleQueue& GetConsoleQueue() {
        return console_queue;
    }
    static bool HasErrors() {
        std::scoped_lock lock{mutex};
        return has_errors;
    }

private:
    static inline std::string log;
    static inline ConsoleQueue console_queue;
    static inline std::mutex mutex;
    static inline bool has_errors = false;
};

/**
 * @brief Video encoding options.
 * @details Mainly for use with the GUI and config system.
 */
class EncoderConfig : public Helper::JsonConfigurable
{
public:
    struct TypeDesc
    {
        /// @brief The name (as seen in the GUI) and, when applicable, the file extension
        const char* name;
        const char* desc;
    };
    struct FFmpegPreset
    {
        std::string name, desc, args, file_ext;
        FFmpegPreset(std::string name, std::string desc, std::string args, std::string file_ext)
            : name(name), desc(desc), args(args), file_ext(file_ext) {}
    };

    EncoderConfig();
    
    static const TypeDesc* Types();
    static size_t NumTypes();

    static const TypeDesc* TYPE_QOI;
    static const TypeDesc* TYPE_PNG;
    static const TypeDesc* TYPE_FFMPEG;

    /// @brief One of the `TYPE_` constants
    const TypeDesc* type = TYPE_PNG;
    int framerate = 60;
    /// @brief Output args appended after the `-i` flag, not including the output file
    std::string ffmpeg_output_args = "-c:v huffyuv";
    /// @brief The output file extension to use in FFmpeg
    std::string ffmpeg_output_ext = "avi";
    std::filesystem::path ffmpeg_path;
    /// @brief A value between 0 and 9
    int png_compression = 1;

    /// @brief Render the ImGui controls. It's best to push a unique ID before calling.
    void ShowImguiControls();
    void FromJson(const nlohmann::json* json) override;
    nlohmann::json ToJson() const override;

    /// @brief A list of FFmpeg presets
    /// @details The first preset is the safest for all use cases.
    static const std::vector<FFmpegPreset>& GetFFmpegPresets();
};

/**
 * @brief Implements a video encoder for @ref FramePool to write frames (as video or image sequence).
 * 
 * Each instance corresponds to one video file or one image sequence.
 */
class VideoWriter
{
public:
    virtual ~VideoWriter() {}
    /**
     * @brief Write the frame buffer. Blocking.
     * @param frame_index Index of the frame being written.
     * @return `false` on failure
     */
    virtual bool WriteFrame(const FrameBufferDx9& buffer, size_t frame_index) = 0;
    /**
     * @brief Whether this instance supports async and unordered writes.
     * @details When false, the FramePool will ensure that writes are ordered and synchronous.
     */
    virtual bool IsAsync() const = 0;
};

/**
 * @brief Write a sequence of images suffixed with a number and file extension
 */
class ImageWriter : public VideoWriter
{
public:
    enum class Format {
        QOI, PNG
    };

    /// @param base_path File path including the name but not the extension
    ImageWriter(uint32_t width, uint32_t height, Format file_format, std::filesystem::path&& base_path)
            : m_width(width), m_height(height), m_file_format(file_format), m_base_path(std::move(base_path)) {}

    /// @brief Write the frame to file
    bool WriteFrame(const FrameBufferDx9& buffer, size_t frame_index) override;
    bool IsAsync() const override { return true; }
    /// @param compression A value between 0 and 9
    void SetPngCompression(int compression) { m_png_compression = compression; }

    /// @param compression A value between 0 and 9
    static bool WritePNG(const FrameBufferRgb& buffer, std::ostream& output, int compression = 7);
    static bool WriteQOI(const FrameBufferRgb& buffer, std::ostream& output);

private:
    const uint32_t m_width;
    const uint32_t m_height;
    const Format m_file_format;
    int m_png_compression = 6;
    std::filesystem::path m_base_path;
};

class FFmpegWriter : public VideoWriter
{
public:
    /**
     * @param output_args FFmpeg output args to append after the `-i` flag, not including the output file name.
     * @param output_path Path of the output file
     */
    FFmpegWriter(uint32_t width, uint32_t height, uint32_t framerate, const std::string& output_args, const std::filesystem::path& output_path);
    ~FFmpegWriter();
    
    bool WriteFrame(const FrameBufferDx9& buffer, size_t frame_index) override;
    bool IsAsync() const override { return false; }

private:
    std::shared_ptr<ffmpipe::Pipe> m_pipe;
};

class FrameBufferBase
{
public:
    virtual ~FrameBufferBase() {}
    virtual uint32_t GetWidth() const = 0;
    virtual uint32_t GetHeight() const = 0;
    virtual D3DFORMAT GetFormat() const = 0;
    virtual const Helper::D3DFORMAT_info& GetFormatInfo() const = 0;
};

/**
 * @brief Wrap a read/writeable 24-bit RGB buffer
 */
class FrameBufferRgb : public FrameBufferBase
{
public:
    FrameBufferRgb(uint32_t width, uint32_t height)
        : m_width(width), m_height(height), m_data((uint8_t*)malloc(width * height * 3)) {}
    ~FrameBufferRgb() { free(m_data); }
    FrameBufferRgb(FrameBufferRgb&& other) : m_data(other.m_data), m_width(other.m_width), m_height(other.m_height) {
        other.m_data = nullptr;
    }

    uint32_t GetWidth() const override { return m_width; }
    uint32_t GetHeight() const override { return m_height; }
    D3DFORMAT GetFormat() const override { return Helper::D3DFMT_B8G8R8; }
    const Helper::D3DFORMAT_info& GetFormatInfo() const;
    uint8_t* GetData() { return m_data; }
    const uint8_t* GetData() const { return m_data; }
    size_t GetDataLength() const { return m_width * m_height * GetPixelStride(); }
    size_t GetPixelStride() const { return 3; }

private:
    uint32_t m_width;
    uint32_t m_height;
    uint8_t* m_data;
};

/**
 * @brief Wrap a readable Direct3D9 surface
 */
class FrameBufferDx9 : public FrameBufferBase
{
public:
    /// @brief Wrap an existing surface
    /// @details The surface is taken without incrementing its ref count.
    FrameBufferDx9(IDirect3DSurface9* surface);
    /// @brief Construct a new surface and wrap it
    FrameBufferDx9(uint32_t width, uint32_t height, D3DFORMAT d3dformat);
    ~FrameBufferDx9() {
        if (m_d3dsurface) m_d3dsurface->Release();
    }
    FrameBufferDx9(FrameBufferDx9&& other) : FrameBufferDx9(other.m_d3dsurface) {
        other.m_d3dsurface = nullptr;
    }

    /// @brief Convert to a 24-bit RGB surface
    FrameBufferRgb ToRgb() const;
    uint32_t GetWidth() const override { return m_width; }
    uint32_t GetHeight() const override { return m_height; }
    D3DFORMAT GetFormat() const override { return m_d3dformat; }
    const Helper::D3DFORMAT_info& GetFormatInfo() const override { return m_d3dformat_info; }
    IDirect3DSurface9* GetSurface() const { return m_d3dsurface; }
    uint8_t GetNumChannels() const { return GetFormatInfo().num_channels; }
    /// @brief The number of bytes between each pixel
    uint8_t GetPixelStride() const { return GetFormatInfo().stride; }

private:

    static void BlitRgb(FrameBufferRgb* dst, D3DLOCKED_RECT src);
    static void BlitBgr(FrameBufferRgb* dst, D3DLOCKED_RECT src);
    static void BlitRgba(FrameBufferRgb* dst, D3DLOCKED_RECT src);
    static void BlitBgra(FrameBufferRgb* dst, D3DLOCKED_RECT src);

    uint32_t m_width = 0;
    uint32_t m_height = 0;
    IDirect3DSurface9* m_d3dsurface = nullptr;
    D3DFORMAT m_d3dformat = D3DFMT_UNKNOWN;
    Helper::D3DFORMAT_info m_d3dformat_info = {0};
};

/**
 * @brief A worker pool that writes video frames.
 * 
 * Call @ref PopEmptyFrame to get an empty frame buffer.
 * Fill the buffer with data and call @ref PushFullFrame to have a worker encode it.
 * 
 * Threads are spawned in the constructor and do not stop until either the destructor or the @ref Close function is called.
 */
class FramePool
{
public:
    /**
     * @brief A frame from the pool.
     * 
     * The user must fill `buffer` with the desired contents and send it back to the pool using @ref PushFullFrame.
     * 
     * The worker threads must encode `buffer` using the provided `writer` and send it back to the pool using @ref PushEmptyFrame.
     */
    struct Frame
    {
        Frame(FrameBufferDx9&& buffer) : buffer(std::move(buffer)) {}
        FrameBufferDx9 buffer;
        size_t index;
        std::shared_ptr<VideoWriter> writer;
    };
    using FramePtr = std::shared_ptr<Frame>;

    /**
     * @brief Initialize the frame pool and spawn threads.
     * @param num_threads Number of worker threads to spawn.
     * @param num_frames Number of frame buffers in the pool.
     */
    FramePool(
        size_t num_threads, size_t num_frames,
        uint32_t frame_width, uint32_t frame_height
    );
    ~FramePool() { Close(); }

    /// @brief Stop accepting new work, wait for all threads to end, and drain the pool.
    /// @details Do not call this from within the pool's own worker thread(s).
    void Close();
    bool IsClosed() const { return m_all.empty(); }
    /**
     * @brief Block indefinitely until an empty frame can be popped, or work has finished.
     * @details Fill this frame and pass it to @ref PushFullFrame.
     * @return `nullptr` if the pool is closed, no longer accepting work.
     */
    FramePtr PopEmptyFrame();
    /// @brief Push a frame for a worker thread to pop and use
    /// @param index Index of the frame being written. `frame->index` will be assigned this.
    /// @param writer Method of writing the frame to video. `frame->writer` will be assigned this.
    void PushFullFrame(FramePtr frame, size_t index, std::shared_ptr<VideoWriter> writer);

private:
    /**
     * @brief Block indefinitely until a full frame can be popped, or work has finished.
     * @details Work on this frame and then pass it to @ref PushEmptyFrame.
     * @return `nullptr` if all work is finished.
     */
    FramePtr PopFullFrame();
    /**
     * @brief Free a frame after it's been written, returning it to the pool.
     * 
     * The `frame->writer` pointer must be valid.
     * If the writer is synchronous, it will become useable for a new thread again.
     * @param frame A valid pointer with a valid buffer and writer.
     */
    void PushEmptyFrame(FramePtr frame);
    static void WorkerLoop(FramePool* pool);

    std::vector<std::thread> m_threads;
    /// @brief All frames in the pool.
    /// This vector is cleared to indicate that no further work shall be added.
    std::vector<FramePtr> m_all;
    /// @brief Frames that are ready to be filled with pixels
    std::vector<FramePtr> m_empty;
    /// @brief Frames that are filled with pixels
    std::vector<FramePtr> m_full;
    /// @brief Synchronous writers that are currently in use
    std::unordered_set<const VideoWriter*> m_sync_writers;
    /// @brief The list of threads waiting on @ref m_empty
    std::condition_variable m_cv_empty;
    /// @brief The list of threads waiting on @ref m_full
    std::condition_variable m_cv_full;
    std::mutex m_mutex;
    /// @brief Locked during @ref Close to prevent repeated closing.
    std::mutex m_close_mutex;
};
