#include "spectatemodule.h"
#include "mainwindow.h"
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

#undef min
#undef max

constexpr int TEAM_SPEC = 1;
constexpr int TEAM_RED = 2;
constexpr int TEAM_BLU = 3;

void SpectateModule::StartListening()
{
    MainWindow::OnWindow.Listen(&SpectateModule::OnMenu, this);
    CClientHook::OnFrameStageNotify.Listen(&SpectateModule::OnFrameStageNotify, this);
    CClientHook::OnOverrideView.Listen(&SpectateModule::OnOverrideView, this);
}

int SpectateModule::OnMenu()
{
    if (!ImGui::CollapsingHeader("Spectate"))
        return 0;
    
    ImGui::PushID("SpectateModule");

    if (ImGui::Checkbox("Spectate", &m_spectating) && m_spectating)
        m_was_reset = true;
    ImGui::SliderFloat("Cam distance", &m_spec_cam_dist, 0, 300);
    ImGui::SliderFloat3("Cam angle", m_spec_cam_angle_off, -180, 180);
    ImGui::SliderFloat3("Cam offset", m_spec_cam_origin_off, -150, 150);
    //ImGui::InputInt("Spectate mode", &m_observer_mode);

    constexpr size_t NUM_COLUMNS = 3;
    if (ImGui::BeginTable("Players", NUM_COLUMNS, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
    {
        static const char* const column_names[NUM_COLUMNS] = {"RED", "BLU", "OTHER"};
        static std::vector<const PlayerState*> column_players[NUM_COLUMNS];
        constexpr size_t RED_COLUMN = 0, BLU_COLUMN = 1, OTHER_COLUMN = 2;

        for (size_t i = 0; i < NUM_COLUMNS; ++i)
            column_players[i].clear();

        std::scoped_lock lock(m_playerstates_mutex);

        // Sort players by team
        for (auto& pair : m_playerstates)
        {
            switch (pair.second.team)
            {
            case TEAM_RED: column_players[RED_COLUMN].push_back(&pair.second); break;
            case TEAM_BLU: column_players[BLU_COLUMN].push_back(&pair.second); break;
            default: column_players[OTHER_COLUMN].push_back(&pair.second); break;
            }
        }

        // Draw table headers and get the # of rows needed for all columns.
        size_t max_rows = 0;
        for (size_t i = 0; i < NUM_COLUMNS; ++i)
        {
            ImGui::TableSetupColumn(column_names[i]);
            max_rows = std::max(max_rows, column_players[i].size());
        }
        ImGui::TableHeadersRow();

        // Now fill the table.
        // Elements must advance horizontally, otherwise this would be written more intuitively.
        for (size_t row = 0; row < max_rows; ++row)
        {
            ImGui::TableNextRow();
            for (size_t col = 0; col < NUM_COLUMNS; ++col)
            {
                if (row >= column_players[col].size())
                    continue;

                const PlayerState* player = column_players[col][row];
                
                ImGui::TableSetColumnIndex(col);
                ImGui::PushID(player);
                if (ImGui::Selectable(player->player_info.name, player->player_info.userID == m_spectate_target))
                   m_spectate_target = player->player_info.userID;
                ImGui::PopID();
            }
        }

        ImGui::EndTable();
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

int SpectateModule::OnFrameStageNotify(ClientFrameStage_t stage)
{   
    if (stage == FRAME_START)
    {
        if (!Interfaces::engine->IsInGame())
        {
            std::scoped_lock lock(m_playerstates_mutex);
            m_was_reset = true;
            m_playerstates.clear();
        }
    }

    if (Interfaces::engine->IsInGame() && ShouldScan()
        && (stage == FRAME_NET_UPDATE_END || (stage == FRAME_START && m_was_reset))
    )
    {
        std::scoped_lock lock(m_playerstates_mutex);
        m_was_reset = false;

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

            // Check for changes in player info here
            bool took_damage = health != player_state.health || (!is_alive && player_state.is_alive);
            int damage = player_state.health - health;

            if (!is_alive)
                damage = INT_MAX;

            // Update player info here
            player_state.health = health;
            player_state.is_alive = is_alive;
            player_state.team = player->Team();

            if (m_scan_airshots)
            {
                if (took_damage && !ent->IsDormant())
                    OnPlayerDamage(player, player_state, damage);
            }
        }
    }

    return 0;
}

int SpectateModule::OnOverrideView(CViewSetup* view_setup)
{
    if (!m_spectating)
        return 0;
    
    int ent_index = Interfaces::engine->GetPlayerForUserID(m_spectate_target);
    if (ent_index <= 0)
        return 0;
    CBaseEntity* base_entity = Interfaces::entlist->GetClientEntity(ent_index);
    if (!base_entity)
        return 0;
    
    CBasePlayer* player = base_entity->ToPlayer();
    
    IClientRenderable* renderable = (IClientRenderable*)base_entity->Renderable();
    Vector cam_pos = renderable->GetRenderOrigin() + Vector(0,0,75);
    QAngle angle = renderable->GetRenderAngles();
    QAngle view_angle = renderable->GetRenderAngles() + *(QAngle*)&m_spec_cam_angle_off[0];
    QAngle forward_angle = QAngle(0, angle.y, 0);
    QAngle side_angle = QAngle(0, angle.y - 90, 0);

    Vector view_dir;
    Vector transform_forward;
    Vector transform_side;

    AngleVectors(view_angle, &view_dir);
    AngleVectors(forward_angle, &transform_forward);
    AngleVectors(side_angle, &transform_side);

    cam_pos -= view_dir * m_spec_cam_dist;
    cam_pos += transform_side * m_spec_cam_origin_off[0];
    cam_pos += transform_forward * m_spec_cam_origin_off[1];
    cam_pos.z += m_spec_cam_origin_off[2];
    view_setup->angles = view_angle;
    view_setup->origin = cam_pos;

    return EventReturnFlags::NoOriginal;
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