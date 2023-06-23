#include <Modules/BaseModule.h>
#include <cstdint>
#include <array>
#include <string>
#include <string_view>

class CRecorder : public CModule
{
public:
    void StartListening() override;

    bool IsRecordingMovie();
    bool ShouldRecordFrame();
    bool StartMovie(std::string_view movie_path);
    void StopMovie();
    void SetFirstMovieError(const char* fmt, ...);

private:
    int OnMenu();
    int OnFrameStageNotify();

    void RecordNewFrame();

    bool m_is_recording = false;
    bool m_pause_on_menu = true;
    int m_png_compression_lvl = 1;
    
    uint32_t m_frame_index;
    std::string m_movie_path; // Always ends with a slash
    std::string m_temp_audio_name; // Path to temp audio file
    std::string m_first_movie_error;
};

class FrameBufferRGB
{
public:
    FrameBufferRGB(uint32_t width, uint32_t height)
        : width(width), height(height), data(new uint8_t[width * height * 3]) { }
    FrameBufferRGB(const FrameBufferRGB&) = delete;

    bool WritePNG(const char* file_name, uint8_t compression_level = 6) const;
    inline uint8_t GetBitDepth() const { return 8; }
    inline uint32_t GetDataLength() const { return width * height * 3 * (GetBitDepth() / 8); }

    const uint32_t width, height;
    uint8_t* data;
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
