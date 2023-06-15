#include "Modules/BaseModule.h"
#include <array>

class SkyboxModule : public CModule {
public:
    void StartListening() override;
    int OnMenu();

    bool IsReady() const;

private:
    typedef bool(_cdecl* LoadNamedSkysFn)(const char*);

    std::array<char, 256> m_skybox_path = { 0 };
    std::string m_last_load_error;
    bool m_update_skybox = false;
    LoadNamedSkysFn m_load_sky_fn;
};

extern SkyboxModule g_skybox;