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
    /// @brief Signal to start the movie
    bool StartMovie() { m_is_recording = true; }
    /// @brief Signal to stop the movie
    void StopMovie() { m_is_recording = false; }

private:
    int OnPostImguiInput();
    int OnDraw();
    int OnMenu();
    int OnConfigSave();
    int OnConfigLoad();
    int OnFrameStageNotify();
    /// @brief This replaces the game's vanilla recording behavior
    int OnWriteMovieFrame();
    /// @brief This replaces the game's vanilla recording behavior
    int OnReadPixels();
    /// @brief Write the next frame for the stream
    /// @param stream If there are no streams, use `nullptr`
    void WriteFrame(std::shared_ptr<Stream> stream);
    void ToggleRecording();
    /// @brief Attempt to setup the movie.
    /// @details Only this call from the game thread.
    /// @return True if the movie is created or already exists
    bool SetupMovie();
    ///@brief Stop the movie, wait for writing to finish, and clean up.
    ///@details Only call this from the game thread.
    void CleanupMovie();
    void CopyCurrentFrameToSurface(class IDirect3DSurface9* dst);
    bool m_read_pixels = false;

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
    /// @brief The current movie
    std::optional<Movie> m_movie;
};

inline CRecorder g_recorder;