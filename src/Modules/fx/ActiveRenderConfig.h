#pragma once
#include <Modules/BaseModule.h>
#include <Streams/renderconfig.h>

/// @brief Set which render config will be used during rendering
class ActiveRenderConfig : public CModule
{
public:
    void StartListening() override;
    
    void Set(RenderConfig::Ptr config);
    void SetNone();

    bool ArePropsVisible() const;

protected:
    int OnDrawProp();
    int OnDrawPropArray();
    int PreDrawModelExecute();
    int PostDrawModelExecute();

private:
    struct LastDrawParams
    {
        std::array<float, 4> color;
    };

    RenderConfig::Ptr m_config;
    
    /// @brief Was the last DrawModelExecute call affected?
    bool m_is_dme_affected = false;
    /// @brief Parameters stored during the last DrawModelExecute call
    LastDrawParams m_last_dme_params;
};

inline ActiveRenderConfig g_active_rendercfg;
