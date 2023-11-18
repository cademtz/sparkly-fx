#pragma once
#include <Modules/BaseModule.h>
#include <Streams/renderconfig.h>
#include <shared_mutex>
#include <unordered_map>
#include <string>
#include <mutex>

class IMaterial;
enum OverrideType_t;

/**
 * @brief Store and apply the active render config.
 * 
 * Some tweaks and properties (like @ref MaterialTweak) are applied once, when necessary.
 * If such special tweaks/values are modified, call @ref ActiveRenderConfig::SignalUpdate to make it re-apply them.
 */
class ActiveRenderConfig : public CModule
{
public:
    void StartListening() override;
    
    /// @brief Get the active config. This calls @ref ReadLock. 
    RenderConfig::Ptr Get();
    /// @brief Set the active config. This calls @ref WriteLock.
    /// @param config Use `nullptr` to clear the active config
    void Set(RenderConfig::Ptr config);
    /**
     * @brief Signal that a config updated and should be re-applied. This calls @ref WriteLock.
     * 
     * The signal is raised when `config == nullptr` or `config == Get()`.
     * The signal is not be raised when there is no active config.
     * @param config The config that was updated, or `nullptr`
     */
    void SignalUpdate(RenderConfig::Ptr config = nullptr);
    /**
     * @brief Immediately update material colors.
     * Only the recorder should use this to update materials many times in a single frame.
     * This calls @ref ReadLock.
     */
    void UpdateMaterials();
    /// @brief Acquire a lock for editing the active config 
    std::unique_lock<std::shared_mutex> WriteLock() { return std::unique_lock{m_mtx};}
    /// @brief Acquire a lock for reading the active config
    std::shared_lock<std::shared_mutex> ReadLock() { return std::shared_lock{m_mtx};}

protected:
    int OnDrawProp();
    int OnDrawPropArray();
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

    /// @brief Store a material's original params (if not already stored)
    void StoreMaterialParams(IMaterial* mat);
    /// @brief Restore the original parameters to a material
    void RestoreMaterialParams(IMaterial* mat);
    /// @brief Override the material's color
    void SetMaterialColor(IMaterial* mat, const std::array<float, 4>& col);

    static IMaterial* CreateMatteMaterial();
    
    /// @brief Was the last DrawModelExecute call affected?
    bool m_is_dme_affected = false;
    /// @brief Should we iterate all materials and update them?
    bool m_should_update_materials = false;
    /// @brief Parameters stored during the last DrawModelExecute call
    LastDrawParams m_last_dme_params;
    /// @brief A matte material
    IMaterial* m_matte_material = nullptr;
    /**
     * @brief Affected materials.
     * 
     * There is no guarantee that pointers remain valid, or that the pointer hasn't been re-occupied.
     * If a material is found in the map, check that their strings match.
     */
    std::unordered_map<IMaterial*, OldMaterialParams> m_affected_materials;
    /// @brief Pretty please don't access this without a @ref ReadLock and @ref WriteLock!
    RenderConfig::Ptr m_config;
    std::shared_mutex m_mtx;
};

inline ActiveRenderConfig g_active_rendercfg;
