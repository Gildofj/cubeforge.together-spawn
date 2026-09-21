#include "SessionState.h"
#include "Config.h"
#include "../utils/Logger.h"

namespace TogetherSpawn {
namespace Core {

    SessionState& SessionState::Instance() {
        static SessionState instance;
        return instance;
    }

    void SessionState::Reset() {
        m_role = SessionRole::SinglePlayer;
        m_hostSteamID = 0;
        m_worldSeed = 0;
        m_characterSlot = 0;
        m_currentSessionSpawned = false;
        m_invulnerabilityEndTime = std::chrono::steady_clock::time_point{};
        m_lastTeleportTime = std::chrono::steady_clock::time_point{};
    }

    void SessionState::Update(cube::Game* game) {
        if (!game) {
            Reset();
            return;
        }

        int prevSeed = m_worldSeed;
        uint64_t prevHost = m_hostSteamID;

        m_worldSeed = game->seed;
        m_characterSlot = game->current_character_slot;

        // Check if hosting or client
        if (game->host.running || !game->host.connections.empty()) {
            m_role = SessionRole::Host;
            if (game->world && game->world->local_creature) {
                m_hostSteamID = static_cast<uint64_t>(game->world->local_creature->entity_data.steam_id);
            }
        } else if (game->client.host_steam_id.ConvertToUint64() != 0) {
            m_role = SessionRole::Client;
            m_hostSteamID = game->client.host_steam_id.ConvertToUint64();
        } else {
            m_role = SessionRole::SinglePlayer;
            m_hostSteamID = 0;
        }

        // If session changed (e.g. joined different server / new seed), reload spawn state
        if (prevSeed != m_worldSeed || prevHost != m_hostSteamID) {
            if (m_role == SessionRole::Client && m_hostSteamID != 0) {
                m_currentSessionSpawned = Config::Instance().HasSpawnedInSession(m_hostSteamID, m_worldSeed, m_characterSlot);
                Utils::Logger::Info("Connected to Host SteamID: " + std::to_string(m_hostSteamID) +
                                    ", Seed: " + std::to_string(m_worldSeed) +
                                    ", Already spawned before: " + (m_currentSessionSpawned ? "Yes" : "No"));
            } else if (m_role == SessionRole::Host) {
                m_currentSessionSpawned = true;
            } else {
                m_currentSessionSpawned = false;
            }
        }
    }

    void SessionState::GrantInvulnerability(float durationSeconds) {
        auto now = std::chrono::steady_clock::now();
        m_invulnerabilityEndTime = now + std::chrono::milliseconds(static_cast<int>(durationSeconds * 1000.0f));
    }

    bool SessionState::IsInvulnerable() const {
        auto now = std::chrono::steady_clock::now();
        return now < m_invulnerabilityEndTime;
    }

    bool SessionState::CanUseTeleportCommand() const {
        const auto& settings = Config::Instance().GetSettings();
        if (!settings.allowTeleportCommands) return false;

        auto now = std::chrono::steady_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::seconds>(now - m_lastTeleportTime).count();
        return diff >= settings.teleportCooldownSeconds;
    }

    void SessionState::RecordTeleportCommandUsed() {
        m_lastTeleportTime = std::chrono::steady_clock::now();
    }

    int SessionState::GetRemainingCooldownSeconds() const {
        const auto& settings = Config::Instance().GetSettings();
        auto now = std::chrono::steady_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::seconds>(now - m_lastTeleportTime).count();
        int rem = settings.teleportCooldownSeconds - static_cast<int>(diff);
        return (rem > 0) ? rem : 0;
    }

} // namespace Core
} // namespace TogetherSpawn
