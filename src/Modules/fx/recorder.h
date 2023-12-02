#include <Modules/BaseModule.h>
#include <string>
#include <cstdint>
#include <filesystem>
#include <Helper/imgui.h>
#include <vector>
#include <mutex>
#include <condition_variable>

class FramePool;

class CRecorder : public CModule
{
public:
    CRecorder();

    /// @brief A video format.
    /// Update @ref VideoFormatName and @ref VideoFormatDesc when changed!
    enum class VideoFormat : int
    {
        QOI, // QOI sequence
        PNG, // PNG sequence
        NUM_FORMATS // Number of formats
    };

    void StartListening() override;

    bool IsRecordingMovie();
    bool ShouldRecordFrame();
    bool StartMovie(const std::string& path);
    void StopMovie();
    /// @brief Set the movie error, if one doesn't exist yet. Locks @ref m_mutex_error.
    void SetFirstMovieError(const char* fmt, ...);
    /// @brief Get the first movie error, or `nullptr` if it hasn't been set. Locks @ref m_mutex_error.
    const char* GetFirstMovieError();
    static const char* VideoFormatName(VideoFormat format);
    static const char* VideoFormatDesc(VideoFormat format);

private:
    int OnPostImguiInput();
    int OnDraw();
    int OnMenu();
    int OnFrameStageNotify();
    /// @brief Write the current frame to the stream
    void WriteFrame(const std::string& stream_name);
    void ToggleRecording();

    // === Menu options === //

    Helper::KeyBind m_record_bind;
    bool m_is_recording = false;
    bool m_record_indicator = false;
    bool m_pause_on_menu = true;
    int m_png_compression_lvl = 1;
    int m_framerate = 60;
    VideoFormat m_video_format = VideoFormat::PNG;
    int m_framepool_size = 1;
    
    // === Current movie info === //

    uint32_t m_frame_index;
    std::string m_input_movie_path;
    std::filesystem::path m_movie_path;
    std::string m_temp_audio_name; // Path to temp audio file
    std::string m_first_movie_error;
    /// @brief Locked in order to read/write @ref m_first_movie_error
    std::mutex m_mutex_error;
    std::shared_ptr<FramePool> m_framepool;
};

inline CRecorder g_recorder;

class FrameBufferRGB
{
public:
    static constexpr int NUM_CHANNELS = 3;
    static constexpr int NUM_BYTES = 1;

    FrameBufferRGB(uint32_t width, uint32_t height)
        : m_width(width), m_height(height), m_data((uint8_t*)malloc(width * height * NUM_CHANNELS * NUM_BYTES)) { }
    ~FrameBufferRGB() { free(m_data); }

    bool WritePNG(std::ostream& output, int compression_level = 6) const;
    bool WriteQOI(std::ostream& output) const;
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
 * @brief A thread-safe, adjustable pool of frames.
 * 
 * Threads are spawned in the constructor and do not stop until either the destructor or @ref Finish is called.
 */
class FramePool
{
public:
    struct Frame
    {
        Frame(uint32_t width, uint32_t height) : buffer(width, height) {}
        FrameBufferRGB buffer;
        /// @brief Frame path. Must include file name and extension.
        std::filesystem::path path;
    };
    using FramePtr = std::shared_ptr<Frame>;

    FramePool(
        size_t num_threads, size_t num_frames,
        CRecorder::VideoFormat format, int png_compression,
        uint32_t frame_width, uint32_t frame_height
    );
    ~FramePool() { Finish(); }

    /// @brief Wait for all threads to end and drain the pool
    void Finish();
    /// @brief Free a frame, returning it to the pool
    void PushEmptyFrame(FramePtr frame);
    /**
     * @brief Block indefinitely until an empty frame can be popped, or work has finished.
     * 
     * Fill this frame and pass it to @ref PushFullFrame.
     * @return `nullptr` if all work is finished.
     */
    FramePtr PopEmptyFrame();
    /// @brief Push a frame for a worker thread to pop and use
    void PushFullFrame(FramePtr frame);
    /**
     * @brief Block indefinitely until a full frame can be popped, or work has finished.
     * 
     * Work on this frame and then pass it to @ref PushEmptyFrame.
     * @return `nullptr` if all work is finished.
     */
    FramePtr PopFullFrame();

private:
    static void WorkerLoop(FramePool* pool);

    const CRecorder::VideoFormat m_format;
    const int m_png_compression;

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

template <class T, void(TFree)(T*)>
struct FreeThis
{
    FreeThis(const FreeThis&) = delete;
    FreeThis(T* value) : value(value) {}
    ~FreeThis() {
        if (value)
            TFree(value);
    }

    T* operator->() { return value; }
    T* operator*() { return value; }
    
    T* value;
};
