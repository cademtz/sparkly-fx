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
    };

    int OnMenu();
    int OnFrameStageNotify();
    int OnOverrideView();
    void AppendLog(std::string_view text) {
        std::scoped_lock lock(m_log_mutex);
        m_log += text;
    }
    /// @brief Whether the demo should be actively scanned
    bool ShouldScan() const { return m_scan_airshots || m_spectating; }
    /// @brief Handle a player that received damage or healing.
    /// @param damage Use negative value for healing. Use INT_MAX for death.
    void OnPlayerDamage(class CBasePlayer* player, const PlayerState& state, int damage);

    bool m_scan_airshots = false;
    bool m_spectating = false;
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