#pragma once
#include <filesystem>
#include <cstdint>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string_view>

class FrameBufferRGB;
namespace ffmpipe { class Pipe; }

/**
 * @brief Video encoding options.
 * @details Mainly for use with the GUI and config system.
 */
struct EncoderConfig
{
    struct TypeDesc
    {
        /// @brief The name (as seen in the GUI) and, when applicable, the file extension
        const char* name;
        const char* desc;
    };
    
    static const TypeDesc* Types();
    static size_t NumTypes();
    /// @brief Render the ImGui controls. It's best to push a unique ID before calling.
    void ShowImguiControls();

    static const TypeDesc* TYPE_QOI;
    static const TypeDesc* TYPE_PNG;
    static const TypeDesc* TYPE_FFMPEG;

    /// @brief One of the `TYPE_` constants
    const TypeDesc* type = TYPE_PNG;
    int framerate = 60;
    /// @brief Output args appended after the `-i` flag, not including the output file
    std::string ffmpeg_output_args;
    /// @brief A value between 0 and 9
    int png_compression = 1;
};

/**
 * @brief An interface for @ref FramePool to write frames to a video file or image sequence.
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
    virtual bool WriteFrame(const FrameBufferRGB& buffer, size_t frame_index) = 0;
    /**
     * @brief Whether this instance can write frames out-of-order.
     * @details This will not 
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
    bool WriteFrame(const FrameBufferRGB& buffer, size_t frame_index) override;
    bool IsAsync() const override { return true; }
    /// @param compression A value between 0 and 9
    void SetPngCompression(int compression) { m_png_compression = compression; }

    /// @param compression A value between 0 and 9
    static bool WritePNG(const FrameBufferRGB& buffer, std::ostream& output, int compression = 7);
    static bool WriteQOI(const FrameBufferRGB& buffer, std::ostream& output);

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
    FFmpegWriter(uint32_t width, uint32_t height, uint32_t framerate, const std::string& output_args, std::filesystem::path&& output_path);
    ~FFmpegWriter();
    
    bool WriteFrame(const FrameBufferRGB& buffer, size_t frame_index) override;
    bool IsAsync() const override { return false; }

private:
    std::shared_ptr<ffmpipe::Pipe> m_pipe;
};

class FrameBufferRGB
{
public:
    static constexpr int NUM_CHANNELS = 3;
    static constexpr int NUM_BYTES = 1;

    FrameBufferRGB(uint32_t width, uint32_t height)
        : m_width(width), m_height(height), m_data((uint8_t*)malloc(width * height * NUM_CHANNELS * NUM_BYTES)) { }
    ~FrameBufferRGB() { free(m_data); }

    void Resize(uint32_t width, uint32_t height)
    {
        m_width = width, m_height = height;
        m_data = (uint8_t*)realloc(m_data, GetDataLength());
    }
    
    inline uint8_t GetBitDepth() const { return NUM_BYTES * 8; }
    inline uint32_t GetDataLength() const { return m_width * m_height * NUM_CHANNELS * NUM_BYTES; }
    uint8_t* GetData() { return m_data; }
    const uint8_t* GetData() const { return m_data; }
    uint32_t GetWidth() const { return m_width; }
    uint32_t GetHeight() const { return m_height; }

private:
    uint32_t m_width;
    uint32_t m_height;
    uint8_t* m_data;
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
        Frame(uint32_t width, uint32_t height) : buffer(width, height) {}
        FrameBufferRGB buffer;
        size_t index;
        std::shared_ptr<VideoWriter> writer;
    };
    using FramePtr = std::shared_ptr<Frame>;

    /// @brief Fill the pool and spawn threads
    FramePool(
        size_t num_threads, size_t num_frames,
        uint32_t frame_width, uint32_t frame_height
    );
    ~FramePool() { Close(); }

    /// @brief Stop accepting new work, wait for all threads to end, and drain the pool
    void Close();
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
    /// @brief Free a frame after it's been written, returning it to the pool.
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
    /// @brief The list of threads waiting on @ref m_empty
    std::condition_variable m_cv_empty;
    /// @brief The list of threads waiting on @ref m_full
    std::condition_variable m_cv_full;
    std::mutex m_mutex;
};
