#include <Modules/BaseModule.h>
#include <string>
#include <sstream>
#include <cstdint>
#include <cstdlib>
#include <filesystem>

class CRecorder : public CModule
{
public:
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
    void SetFirstMovieError(const char* fmt, ...);
    static const char* VideoFormatName(VideoFormat format);
    static const char* VideoFormatDesc(VideoFormat format);

private:
    int OnMenu();
    int OnFrameStageNotify();
    /// @brief Write the current frame to the stream
    void WriteFrame(const std::string& stream_name);

    bool m_is_recording = false;
    bool m_pause_on_menu = true;
    int m_png_compression_lvl = 1;
    VideoFormat m_video_format = VideoFormat::PNG;
    
    uint32_t m_frame_index;
    std::string m_input_movie_path;
    std::filesystem::path m_movie_path;
    std::string m_temp_audio_name; // Path to temp audio file
    std::string m_first_movie_error;
};

inline CRecorder g_recorder;

class FrameBufferRGB
{
public:
    const int NUM_CHANNELS = 3;
    const int NUM_BYTES = 1;

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
