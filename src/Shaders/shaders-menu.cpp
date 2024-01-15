#include "shaders.h"
#include <imgui/imgui.h>
#include <Helper/imgui.h>
#include <nlohmann/json.hpp>

namespace Shader
{

void DepthLinear::OnMenu()
{
    ImGui::InputFloat("Near Z", &m_near, 100, 1000);
    ImGui::InputFloat("Far Z", &m_far, 100, 1000);
    ImGui::Checkbox("Spherical correction", &m_spherical_correction);
}
nlohmann::json DepthLinear::SubclassToJson() const
{
    return {
        {"m_near", m_near},
        {"m_far", m_far},
        {"m_spherical_correction", m_spherical_correction}
    };
}
void DepthLinear::FromJson(const nlohmann::json* json)
{
    Helper::FromJson(json, "m_near", m_near);
    Helper::FromJson(json, "m_far", m_far);
    Helper::FromJson(json, "m_spherical_correction", m_spherical_correction);
}

void DepthLogarithm::OnMenu()
{
    ImGui::InputFloat("Near Z", &m_near, 100, 1000);
    ImGui::InputFloat("Far Z", &m_far, 100, 1000);
    ImGui::Checkbox("Spherical correction", &m_spherical_correction);
    ImGui::InputFloat("Near precision", &m_near_precision, 0.5, 1); ImGui::SameLine();
    Helper::ImGuiHelpMarker(
        "Increasing values (above 1) gives more precision in closer geometry, and less precision in farther geometry.\n"
    );
}
nlohmann::json DepthLogarithm::SubclassToJson() const
{
    return {
        {"m_near", m_near},
        {"m_far", m_far},
        {"m_spherical_correction", m_spherical_correction},
        {"m_near_precision", m_near_precision}
    };
}
void DepthLogarithm::FromJson(const nlohmann::json* json)
{
    Helper::FromJson(json, "m_near", m_near);
    Helper::FromJson(json, "m_far", m_far);
    Helper::FromJson(json, "m_spherical_correction", m_spherical_correction);
    Helper::FromJson(json, "m_near_precision", m_near_precision);
}

void DepthToRgb::OnMenu() {}
nlohmann::json DepthToRgb::SubclassToJson() const { return {}; }
void DepthToRgb::FromJson(const nlohmann::json* json) {}

void NormalToRgb::OnMenu() {}
nlohmann::json NormalToRgb::SubclassToJson() const { return {}; }
void NormalToRgb::FromJson(const nlohmann::json* json) {}

}