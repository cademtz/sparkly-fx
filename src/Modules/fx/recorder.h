#include <Modules/BaseModule.h>
#include <string>
#include <cstdint>
#include <filesystem>
#include <Helper/str.h>
#include <Helper/imgui.h>
#include <Helper/threading.h>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include <Streams/movie.h>
#include <Streams/videowriter.h>

class FramePool;
class VideoWriter;
class Stream;
class FrameBufferDx9;

class CRecorder : public CModule
{
public:
    void StartListening() override;

    bool IsRecordingMovie();
    bool ShouldRecordFrame();
    bool StartMovie() { m_is_recording = true; }
    void StopMovie() { m_is_recording = false; }
    Helper::LockedRef<std::string> GetErrorLog();
    /// @brief Append an error to the log. Don't forget newlines!
    void AppendErrorLog(std::string_view str);
    /// @brief Write an error to the log. Don't forget newlines!
    template <class... TArgs>
    void FormatErrorLog(const char* fmt, TArgs&&... args) {
        AppendErrorLog(Helper::sprintf(fmt, std::forward<TArgs>(args)...));
    }
    void ClearErrorLog();

private:
    int OnPostImguiInput();
    int OnDraw();
    int OnPresent();
    int OnMenu();
    int OnFrameStageNotify();
    /// @brief This replaces the game's vanilla recording behavior
    int OnWriteMovieFrame();
    /// @brief Write the next frame for the stream
    /// @param stream If there are no streams, use `nullptr`
    void WriteFrame(std::shared_ptr<Stream> stream);
    void ToggleRecording();
    /// @brief Attempt to setup the movie
    /// @return True if the movie is created or already exists
    bool SetupMovie();
    /// @brief Only call this before/after the logic to write frames.
    /// Don't use from the menu, which may be mid-frame on the same thread.
    void CleanupMovie();

    /// @brief Render target used during the last Present call.
    /// @details Use this instead of accidentally getting a random render target.
    class IDirect3DSurface9* m_render_target = nullptr;

    // === Menu options === //

    Helper::KeyBind m_record_bind;
    /// @brief This is a signal to start/stop the movie in the game thread
    bool m_is_recording = false;
    bool m_record_indicator = false;
    /// @brief Resume the demo when recording starts
    bool m_autoresume_demo = true;
    /// @brief Pause the demo when recording stops
    bool m_autopause_demo = true;
    /// @brief Close the menu when recording starts
    bool m_autoclose_menu = false;
    /// @brief Stop the recording when the menu is opened
    bool m_autostop_recording = false;
    int m_framepool_size = 1;
    EncoderConfig m_videoconfig;
    /// @brief The root directory to contain all movie files
    std::filesystem::path m_movie_path;

    // === Current movie info === //

    std::string m_error_log;
    /// @brief Lock to read/write @ref m_first_movie_error
    std::mutex m_error_mutex;
    /// @brief The current movie
    std::optional<Movie> m_movie;
};

inline CRecorder g_recorder;