#include <Modules/Menu.h>
#include <Base/Sig.h>
#include <array>
#include "skybox.h"

SkyboxModule g_skybox;

void SkyboxModule::StartListening() {
    // Thank you, "GDPR_Anonymous" from unknowncheats.me
    m_load_sky_fn = (LoadNamedSkysFn)Sig::FindPattern("engine.dll", "55 8B EC 81 EC ? ? ? ? 8B 0D ? ? ? ? 53 56 57 8B 01 C7 45");

    Listen(EVENT_MENU, [this]{ return OnMenu(); });
}

bool SkyboxModule::IsReady() const {
    return m_load_sky_fn != nullptr;
}

int SkyboxModule::OnMenu() {
    if (ImGui::CollapsingHeader("Skybox")) {
        ImGui::BeginGroup();
        
        if (IsReady()) {
            ImGui::InputText("Skybox texture", m_skybox_path.data(), m_skybox_path.size());
            if (ImGui::Button("Apply"))
                m_update_skybox = true;
            if (!m_last_load_error.empty())
                ImGui::Text("%s", m_last_load_error.c_str());
        } else {
            ImGui::TextDisabled("This feature did not load correctly. It may be outdated.");
        }

        ImGui::EndGroup();
    }

    if (m_update_skybox) {
        m_update_skybox = false;
        if (m_load_sky_fn(m_skybox_path.data()))
            m_last_load_error.clear();
        else
            m_last_load_error = std::string("Could not load skybox '") + m_skybox_path.data() + "'";
    }

    return 0;
}