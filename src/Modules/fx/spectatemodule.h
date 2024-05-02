#pragma once
#include <Modules/BaseModule.h>
#include <string>
#include <string_view>
#include <mutex>
#include <unordered_map>
#include <SDK/cdll_int.h>

class SpectateModule : CModule
{
public:

protected:
    void StartListening() override;

private:
    struct PlayerState
    {
        player_info_t player_info;
        bool is_alive;
        int health;
        int team;
    };

    int OnTabBar();
    int OnFrameStageNotify(ClientFrameStage_t stage);
    int OnOverrideView(CViewSetup* view_setup);
    void AppendLog(std::string_view text) {
        std::scoped_lock lock(m_log_mutex);
        m_log += text;
    }
    /// @brief Whether the demo should be actively scanned
    bool ShouldScan() const { return m_scan_airshots || m_spectating; }
    /// @brief Handle a player that received damage or healing.
    /// @param damage Use negative value for healing. Use INT_MAX for death.
    void OnPlayerDamage(class CBasePlayer* player, const PlayerState& state, int damage);

    bool m_was_reset = true;
    bool m_scan_airshots = false;
    bool m_spectating = false;
    float m_spec_cam_dist = 150;
    float m_spec_cam_angle_off[3] = {0,0,0};
    float m_spec_cam_origin_off[3] = {0,0,0};
    int m_spectate_target = 0;
    int m_min_airshot_height = 200;
    int m_min_airshot_damage = 30;
    int m_min_airshot_heal = 15;
    int m_observer_mode = 4;

    std::string m_log;
    std::mutex m_log_mutex;
    std::mutex m_playerstates_mutex;
    /// @brief Map a player's server-specific userid to a PlayerState
    std::unordered_map<int, PlayerState> m_playerstates;
};

inline SpectateModule g_spectate;