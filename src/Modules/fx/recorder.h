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
#include <atomic>
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
    void StartMovie() { return StartMovie(m_movie_path); }
    void StartMovie(const std::filesystem::path& path);
    void StopMovie();
    void ToggleRecording(const std::filesystem::path& path);

private:
    int OnPostImguiInput();
    int OnDraw();
    int OnMenu();
    int OnConfigSave();
    int OnConfigLoad();
    int OnFrameStageNotify(enum ClientFrameStage_t stage);
    /// @brief This replaces the game's vanilla recording behavior
    int OnWriteMovieFrame();
    /// @brief This replaces the game's vanilla recording behavior
    int OnReadPixels();
    /// @brief Write the next frame for the stream
    /// @param stream If there are no streams, use `nullptr`
    void WriteFrame(std::shared_ptr<Stream> stream);
    /// @brief Attempt to setup the movie.
    /// @details Only this call from the game thread.
    /// @return True if the movie is created or already exists
    bool SetupMovie(std::filesystem::path&& path);
    ///@brief Stop the movie, wait for writing to finish, and clean up.
    ///@details Only call this from the game thread.
    void CleanupMovie();
    /// @brief Waits for all rendering to finish
    void WaitForRenderQueue();
    void CopyCurrentFrameToSurface(class IDirect3DSurface9* dst);

    /// @brief If `false`, this will prevent the engine from reading pixels during recording.
    /// @details This remains false to preserve performance. The engine's ReadPixels function is slow and used unecessarily.
    bool m_read_pixels = false;

    // === Menu options === //

    Helper::KeyBind m_record_bind;
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

    // === Threaded movie state === //

    /// @brief Protects all members under the "Threaded movie state" comment
    std::mutex m_movie_mtx;
    /// @brief Path of the next movie to record
    std::filesystem::path m_next_movie_path;
    /// @brief Signal to stop the current movie in the game thread
    bool m_do_stop_recording = false;
    /// @brief Signal to start a recording with @ref m_next_movie_path in the game thread
    bool m_do_start_recording = false;
    std::atomic<bool> m_is_recording_ = false;

    // === Hidden movie state === //
    
    /**
     * @brief The current movie. Equal to `std::nullopt` while not recording.
     * 
     * Must be exclusively accessed by the game thread.
     */
    std::optional<Movie> m_movie;
};

inline CRecorder g_recorder;