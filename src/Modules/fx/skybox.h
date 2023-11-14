#include "Modules/BaseModule.h"
#include <array>
#include <string>

class SkyboxModule : public CModule {
public:
    bool IsReady() const;

protected:
    void StartListening() override;

private:
    typedef bool(_cdecl* LoadNamedSkysFn)(const char*);

    int OnMenu();

    std::array<char, 256> m_skybox_path = { 0 };
    std::string m_last_load_error;
    bool m_update_skybox = false;
    LoadNamedSkysFn m_load_sky_fn;
};

extern SkyboxModule g_skybox;