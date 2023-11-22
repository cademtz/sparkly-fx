#pragma once
#include <Modules/BaseModule.h>
#include <Streams/Stream.h>
#include <shared_mutex>
#include <unordered_map>
#include <string>
#include <mutex>
#include <cstdint>

class IMaterial;
enum OverrideType_t;

/**
 * @brief Store and apply the active stream.
 * 
 * Some tweaks and properties (like @ref MaterialTweak) are applied once, when necessary.
 * If such special tweaks/values are modified, call @ref ActiveStream::SignalUpdate to make it re-apply them.
 */
class ActiveStream : public CModule
{
public:
    enum UpdateFlags
    {
        UPDATE_MATERIALS = 1 << 0,
        UPDATE_FOG = 1 << 1,
        UPDATE_WORLD = 1 << 2,
    };

    void StartListening() override;
    
    /// @brief Get the active stream. This calls @ref ReadLock. 
    Stream::Ptr Get();
    /// @brief Set the active stream. This calls @ref WriteLock.
    /// @param stream Use `nullptr` to clear the active stream
    void Set(Stream::Ptr stream);
    /**
     * @brief Signal that a stream updated and should be re-applied. This calls @ref WriteLock.
     * 
     * The signal only raises if a stream is active.
     * @param stream The stream that was updated, or `nullptr` to update with the current one.
     * @param UpdateFlags Combined values from @ref UpdateFlags. The default value sets all flags.
     */
    void SignalUpdate(Stream::Ptr stream = nullptr, uint32_t flags = ~0);
    /**
     * @brief Immediately update material colors.
     * 
     * Only the recorder should use this to update materials many times in a single frame.
     * This calls @ref ReadLock.
     */
    void UpdateMaterials();
    /// @brief Acquire a lock for editing the active stream 
    std::unique_lock<std::shared_mutex> WriteLock() { return std::unique_lock{m_mtx};}
    /// @brief Acquire a lock for reading the active stream
    std::shared_lock<std::shared_mutex> ReadLock() { return std::shared_lock{m_mtx};}

protected:
    int OnDrawStaticProp();
    int PreDrawModelExecute();
    int PostDrawModelExecute();
    int OnFrameStageNotify();
    int OnOverrideView();

private:
    struct LastDrawParams
    {
        std::array<float, 4> color;
        IMaterial* mat_override = nullptr;
        OverrideType_t mat_override_type = (OverrideType_t )0;
    };

    struct OldMaterialParams
    {
        std::string name;
        std::array<float, 4> color;
    };

    void UpdateFog();
    void UpdateWorld();
    /// @brief Store a material's original params (if not already stored)
    void StoreMaterialParams(IMaterial* mat);
    /// @brief Restore the original parameters to a material
    void RestoreMaterialParams(IMaterial* mat);
    /// @brief Override the material's color
    void SetMaterialColor(IMaterial* mat, const std::array<float, 4>& col);
    IMaterial* CreateMatteMaterial();
    
    /// @brief Was the last DrawModelExecute call affected?
    bool m_is_dme_affected = false;
    /// @brief Should we iterate all materials and update them?
    bool m_should_update_materials = false;
    bool m_should_update_fog = false;
    /// @brief Parameters stored during the last DrawModelExecute call
    LastDrawParams m_last_dme_params;
    /**
     * @brief Affected materials.
     * 
     * There is no guarantee that pointers remain valid, or that the pointer hasn't been re-occupied.
     * If a material is found in the map, check that their strings match.
     */
    std::unordered_map<IMaterial*, OldMaterialParams> m_affected_materials;
    /// @brief Pretty please don't access this without a @ref ReadLock and @ref WriteLock!
    Stream::Ptr m_stream;
    std::shared_mutex m_mtx;
};

inline ActiveStream g_active_stream;
