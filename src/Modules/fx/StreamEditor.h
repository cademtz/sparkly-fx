#pragma once
#include <Modules/BaseModule.h>
#include <Streams/Stream.h>
#include <memory>
#include <list>

/**
 * @brief Add, remove, and edit video streams.
 * 
 * This will lock @ref ActiveStream's mutex when modifying collections (list, set, vector, ...).
 * 
 * While recording, no changes will be made.
 * Otherwise, it is unsafe to iterate or read video streams.
 */
class StreamEditor : public CModule
{
public:
    void StartListening() override;
    /// @brief Sets appropriate preview stream again
    void OnEndMovie();
    
private:
    int OnMenu();
    int OnConfigSave();
    int OnConfigLoad();
    void ShowStreamListEditor();
    /// @brief Rename a stream, or create one if `stream == nullptr`
    void PopupStreamRenamer(Stream::Ptr stream);
    void PopupStreamPresets();
    void ShowStreamEditor(const Stream::Ptr& stream);
    void PopupTweakCreator(const Stream::Ptr& stream);
    void ShowTweakEditor(const RenderTweak::Ptr& render_tweak);
    /// @brief True if the name already exists 
    bool IsDuplicateName(std::string_view name) const;
    /// @brief Create a unique name (by appending a number to it)
    /// @param base_name Beginning text for the name 
    std::string MakeUniqueName(std::string_view base_name) const;

    friend class CRecorder;
    /**
     * @brief The list of streams.
     * 
     * Do not read this list, except when recording. Thanks!
     */
    const std::vector<Stream::Ptr>& GetStreams() const { return m_streams; }

    /// @brief A list of every stream. No duplicate names are allowed.
    std::vector<Stream::Ptr> m_streams;
    /// @brief The selected stream index, or a value >= `m_streams.size()`
    int m_stream_index;
    bool m_preview = true;
};

inline StreamEditor g_stream_editor;