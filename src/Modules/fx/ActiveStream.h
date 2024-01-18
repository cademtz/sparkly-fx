#pragma once
#include <Modules/BaseModule.h>
#include <Helper/engine.h>
#include <Streams/Stream.h>
#include <shared_mutex>
#include <unordered_map>
#include <string>
#include <mutex>
#include <cstdint>
#include <utility>
#include <d3d9.h>

class IMaterial;
enum OverrideType_t;

/**
 * @brief Apply the active stream's settings during rendering.
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
        UPDATE_CONVARS = 1 << 2,
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

    /// @brief True if we have a readable depth surface
    bool IsDepthAvailable() const { return m_color_replacement && m_depth_replacement_surface; }
    /// @brief Draw the depth buffer (if enabled). This calls @ref ReadLock.
    /// @details This also re-draws ImGui. Otherwise, the menu will disappear.
    /// @return `true` if anything was drawn.
    bool DrawDepth();

protected:
    int OnDrawStaticProp();
    int PreDrawModelExecute();
    int PostDrawModelExecute();
    int OnFrameStageNotify();
    int OnOverrideView();
    int OnViewDrawFade();
    int OnReset();
    int OnPresent();

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

    /// @brief Plz run in game thread
    void UpdateViewMatricies();
    void UpdateRenderTarget();
    void UpdateHud();
    void UpdateFog();
    void UpdateConVars();
    /// @brief Store a material's original params (if not already stored)
    void StoreMaterialParams(IMaterial* mat);
    /// @brief Restore the original parameters to a material
    void RestoreMaterialParams(IMaterial* mat);
    /// @brief Override the material's color
    void SetMaterialColor(IMaterial* mat, const std::array<float, 4>& col);
    IMaterial* CreateMatteMaterial();
    void InitializeDxStuff();
    
    /// @brief Assigned `true` during Reset and `false` during Present
    bool m_dx_reset = true;
    IDirect3DSurface9* m_depth = nullptr;
    IDirect3DSurface9* m_color = nullptr;
    IDirect3DSurface9* m_depth_replacement_surface = nullptr;
    IDirect3DTexture9* m_depth_replacement_texture = nullptr;
    IDirect3DSurface9* m_color_replacement = nullptr;
    /// @brief Plz lock @ref m_matrices_mutex
    D3DMATRIX m_projection_matrix;
    std::mutex m_matrices_mutex;
    /// @brief Was the last DrawModelExecute call affected?
    bool m_is_dme_affected = false;
    /// @brief Should we iterate all materials and update them?
    bool m_should_update_materials = false;
    bool m_should_update_fog = false;
    std::vector<Helper::RestoringConVar> m_convars;
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
