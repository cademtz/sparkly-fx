#pragma once
#include <vector>
#include <memory>
#include <string>
#include <filesystem>
#include <functional>
#include <optional>
#include "videowriter.h"

class VideoWriter;
class EncoderConfig;
class Stream;

/**
 * @brief Movie structure and encoder.
 * @details Use @ref GetFramePool to fill and dispatch frame buffers for writing.
 * @ref LogError must be called when an error occurs. The movie will not stop otherwise.
 * @see FramePool
 */
class Movie
{
public:
    struct StreamPair
    {
        std::shared_ptr<Stream> stream;
        std::shared_ptr<VideoWriter> writer;
    };
    using ErrorCallback = std::function<void(std::string_view)>;

    /***
     * @brief Set up all directories and structures.
     * @details Check @ref Failed afterwards.
     * @param root_path The directory to containt all movie files.
     * This directory is created automatically.
     * @param framepool_size Number of frame buffers to reserve
     * @param error_callback Called with an error message
     * @param default_videoconfig The video config to use for all streams without an override
     */
    Movie(
        uint32_t width, uint32_t height,
        std::filesystem::path&& root_path, const std::vector<std::shared_ptr<Stream>>& streams,
        size_t framepool_size, ErrorCallback error_callback, const EncoderConfig& default_videoconfig
    );

    const std::string& GetTempAudioName() const { return m_temp_audio_name; }
    /// @brief Root movie directory
    const std::filesystem::path& GetRootPath() const { return m_root_path; }
    const std::vector<StreamPair>& GetStreams() const { return m_streams; }
    std::vector<StreamPair>& GetStreams() { return m_streams; }
    /// @brief True if the movie failed to initialize 
    bool Failed() const { return m_has_errors; }
    /// @brief Get the frame pool.
    /// @details Do not call this if @ref Failed is true immediately after construction.
    FramePool& GetFramePool();

private:
    Movie(const Movie&) = delete;
    /// @brief Log an error message and cause @ref Failed to return `true` 
    void LogError(const std::string& error);
    static std::string CreateTempAudioName(const char* suffix);

    /// @brief The folder to contain all movie files
    const std::filesystem::path m_root_path;
    /// @brief Name of the temp audio file
    const std::string m_temp_audio_name;
    const ErrorCallback m_error_callback;
    std::vector<StreamPair> m_streams;
    /// @brief This is wrapped so we don't unnecessarily construct it.
    std::optional<FramePool> m_framepool;
    bool m_has_errors;
};