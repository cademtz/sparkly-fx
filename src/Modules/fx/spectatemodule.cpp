#include "spectatemodule.h"
#include <Modules/Menu.h>
#include <Helper/imgui.h>
#include <Hooks/ClientHook.h>
#include <Base/Interfaces.h>
#include <Base/Entity.h>
#include <SDK/cdll_int.h>
#include <SDK/ienginetool.h>
#include <SDK/const.h>
#include <SDK/cmodel.h>
#include <SDK/IEngineTrace.h>
#include <SDK/view_shared.h>
#include <sstream>

void SpectateModule::StartListening()
{
    Listen(EVENT_MENU, [this]() { return OnMenu(); });
    Listen(EVENT_FRAMESTAGENOTIFY, [this]() { return OnFrameStageNotify(); });
    Listen(EVENT_OVERRIDEVIEW, [this]() { return OnOverrideView(); });
}

int SpectateModule::OnMenu()
{
    if (!ImGui::CollapsingHeader("Spectate"))
        return 0;
    
    ImGui::PushID("SpectateModule");

    ImGui::Checkbox("Spectate", &m_spectating);
    //ImGui::InputInt("Spectate mode", &m_observer_mode);

    if (ImGui::BeginListBox("Players", Helper::CalcListBoxSize(m_playerstates.size())))
    {
        std::scoped_lock lock(m_playerstates_mutex);
        for (auto& pair : m_playerstates)
        {
            ImGui::PushID(pair.first);
            if (ImGui::Selectable(pair.second.player_info.name, pair.first == m_spectate_target))
                m_spectate_target = pair.first;
            ImGui::PopID();
        }
        ImGui::EndListBox();
    }

    ImGui::Checkbox("Scan for air-shots", &m_scan_airshots); ImGui::SameLine();
    Helper::ImGuiHelpMarker("Log every player that was hit mid-air");
    if (ImGui::InputInt("Min air-shot height", &m_min_airshot_height) && m_min_airshot_height < 1)
        m_min_airshot_height = 1;
    if (ImGui::InputInt("Min air-shot damage", &m_min_airshot_damage) && m_min_airshot_damage < 1)
        m_min_airshot_damage = 1;
    if (ImGui::InputInt("Min air-shot healing", &m_min_airshot_heal) && m_min_airshot_heal < 1)
        m_min_airshot_heal = 1;

    {
        std::scoped_lock lock(m_log_mutex);
        if (ImGui::Button("Clear log"))
            m_log.clear();
        ImGui::InputTextMultiline("Log", m_log.data(), m_log.size(), ImVec2(0,0), ImGuiInputTextFlags_ReadOnly);
    }

    ImGui::Text("Client tick: %d", Interfaces::engine_tool->ClientTick());
    ImGui::Text("Server tick: %d", Interfaces::engine_tool->ServerTick());
    ImGui::Text("Host tick: %d", Interfaces::engine_tool->HostTick());

    int time = Interfaces::engine_tool->ClientTime();
    ImGui::Text("Client time: %d:%d", time / 60, time % 60);
    time = Interfaces::engine_tool->ServerTime();
    ImGui::Text("Server time: %d:%d", time / 60, time % 60);
    time = Interfaces::engine_tool->HostTime();
    ImGui::Text("Host time: %d:%d", time / 60, time % 60);
    time = Interfaces::engine_tool->Time();
    ImGui::Text("Time: %d:%d", time / 60, time % 60);

    ImGui::PopID();
    
    return 0;
}

int SpectateModule::OnFrameStageNotify()
{
    ClientFrameStage_t stage = g_hk_client.Context()->curStage;
    
    if (stage == FRAME_START)
    {
        if (!Interfaces::engine->IsInGame())
        {
            std::scoped_lock lock(m_playerstates_mutex);
            m_playerstates.clear();
        }
    }
    else if (stage == FRAME_NET_UPDATE_END)
    {
        if (!Interfaces::engine->IsInGame() || !ShouldScan())
            return 0;
        
        std::scoped_lock lock(m_playerstates_mutex);
        for (int i = 1; i <= Interfaces::engine->GetMaxClients(); i++)
        {
            CBaseEntity* ent = Interfaces::entlist->GetClientEntity(i);
            if (!ent)
                continue;
            
            player_info_t info;
            if (!Interfaces::engine->GetPlayerInfo(i, &info))
                continue;

            CBasePlayer* player = ent->ToPlayer();
            
            auto insertion = m_playerstates.emplace(std::make_pair(info.userID, PlayerState{}));
            PlayerState& player_state = insertion.first->second;
            bool is_alive = player->LifeState() == LIFE_ALIVE;
            int health = player->Health();

            if (insertion.second) // Value didn't exist before. Initialize it.
            {
                memset(&player_state, 0, sizeof(player_state));
                player_state.player_info = info;
                player_state.is_alive = is_alive;
                player_state.health = health;
            }

            bool took_damage = health != player_state.health || (!is_alive && player_state.is_alive);
            int damage = player_state.health - health;

            if (!is_alive)
                damage = INT_MAX;

            player_state.health = health;
            player_state.is_alive = is_alive;

            if (took_damage && !ent->IsDormant())
                OnPlayerDamage(player, player_state, damage);            
        }
    }

    return 0;
}

int SpectateModule::OnOverrideView()
{
    if (!m_spectating)
        return 0;
    
    int ent_index = Interfaces::engine->GetPlayerForUserID(m_spectate_target);
    if (ent_index <= 0)
        return 0;
    CBaseEntity* base_entity = Interfaces::entlist->GetClientEntity(ent_index);
    if (!base_entity)
        return 0;
    
    IClientRenderable* renderable = (IClientRenderable*)base_entity->Renderable();
    CViewSetup* view_setup = g_hk_client.Context()->pSetup;
    QAngle angle = renderable->GetRenderAngles();
    Vector cam_pos = renderable->GetRenderOrigin() + Vector(0,0,75);
    Vector direction;

    AngleVectors(angle, &direction);
    cam_pos -= direction * 150;
    view_setup->angles = angle;
    view_setup->origin = cam_pos;

    return Return_NoOriginal;
}

void SpectateModule::OnPlayerDamage(class CBasePlayer* player, const PlayerState& state, int damage)
{
    Ray_t ray;
    CTraceFilterWorldAndPropsOnly filter;
    trace_t trace;
    
    ray.Init(player->Origin() + Vector(0,0,1), player->Origin() - Vector(0,0,m_min_airshot_height));
    Interfaces::trace->TraceRay(ray, MASK_PLAYERSOLID, &filter, &trace);
    
    bool is_in_air = !trace.DidHit();
    if (is_in_air && (damage >= m_min_airshot_damage || -damage >= m_min_airshot_heal) )
    {
        Vector pos = player->Origin();
        std::stringstream ss;
        ss << "Airshot: " << damage << " damage to " << state.player_info.name << '\n';
        ss << "  at " << pos.x << ", " << pos.y << ", " << pos.z << '\n';
        ss << "  tick " << Interfaces::engine_tool->ClientTick() << "time" << Interfaces::engine_tool->ClientTime() << '\n';
        AppendLog(ss.str());
    }
}