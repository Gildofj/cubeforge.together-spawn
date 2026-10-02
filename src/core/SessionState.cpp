#include "SessionState.h"
#include "Config.h"
#include "../features/spawn/SpawnManager.h"
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
        m_localSteamID = 0;
        m_worldSeed = 0;
        m_characterSlot = -1;
        m_characterName = "";
        m_localCreature = nullptr;
        m_characterLevel = 0;
        m_characterXP = 0;
        m_currentSessionSpawned = false;
        m_invulnerabilityEndTime = std::chrono::steady_clock::time_point{};
        m_lastTeleportTime = std::chrono::steady_clock::time_point{};
    }

    void SessionState::Update(cube::Game* game) {
        if (!game || !game->world || !game->world->local_creature) {
            if (m_role != SessionRole::SinglePlayer || m_worldSeed != 0 || !m_characterName.empty()) {
                Reset();
            }
            return;
        }

        int prevSeed = m_worldSeed;
        uint64_t prevHost = m_hostSteamID;
        SessionRole prevRole = m_role;
        int prevSlot = m_characterSlot;
        std::string prevName = m_characterName;
        cube::Creature* prevCreature = m_localCreature;

        m_worldSeed = game->seed;
        m_characterSlot = game->current_character_slot;
        m_localCreature = game->world->local_creature;

        if (m_localCreature) {
            m_characterName = std::string(m_localCreature->entity_data.name, strnlen(m_localCreature->entity_data.name, 16));
            m_characterLevel = static_cast<int>(m_localCreature->entity_data.level);
            m_characterXP = m_localCreature->entity_data.XP;
        } else {
            m_characterName = "";
            m_characterLevel = 0;
            m_characterXP = 0;
        }

        // Resolve Local Steam ID
        uint64_t mySteamID = 0;
        ISteamUser* steamUser = cube::SteamUser();
        if (steamUser) {
            mySteamID = steamUser->GetSteamID().ConvertToUint64();
        }
        if (mySteamID == 0 && m_localCreature) {
            mySteamID = static_cast<uint64_t>(m_localCreature->entity_data.steam_id);
        }
        m_localSteamID = mySteamID;

        // Check remote host steam ID in client subsystem
        uint64_t clientHostSteamID = game->client.host_steam_id.ConvertToUint64();

        SessionRole newRole;
        uint64_t newHostSteamID;

        // 1. If game->client.host_steam_id is set and differs from our own SteamID:
        //    We are a CLIENT connected to a remote Host!
        if (clientHostSteamID != 0 && (mySteamID == 0 || clientHostSteamID != mySteamID)) {
            newRole = SessionRole::Client;
            newHostSteamID = clientHostSteamID;
        }
        // 2. If we have active incoming client connections:
        //    We are the HOST with other players connected!
        else if (!game->host.connections.empty()) {
            newRole = SessionRole::Host;
            newHostSteamID = mySteamID;
        }
        // 3. If local host server is running and we didn't join a remote host:
        //    We are the HOST of our local world.
        else if (game->host.running || game->client.host != nullptr) {
            newRole = SessionRole::Host;
            newHostSteamID = mySteamID;
        }
        // 4. Default fallback
        else {
            newRole = SessionRole::SinglePlayer;
            newHostSteamID = mySteamID;
        }

        m_role = newRole;
        m_hostSteamID = newHostSteamID;

        bool sessionChanged = (m_worldSeed != 0) && (
            prevSeed != m_worldSeed ||
            prevHost != m_hostSteamID ||
            prevRole != m_role ||
            prevSlot != m_characterSlot ||
            prevName != m_characterName ||
            prevCreature != m_localCreature
        );

        if (sessionChanged) {
            Features::SpawnManager::Instance().ResetCooldown();

            if (m_role == SessionRole::Client && m_hostSteamID != 0) {
                // If it's a newly created fresh character (level <= 1 and XP == 0), always treat as NOT yet spawned
                bool isFreshChar = (m_characterLevel <= 1 && m_characterXP == 0);
                if (isFreshChar) {
                    m_currentSessionSpawned = false;
                } else {
                    m_currentSessionSpawned = Config::Instance().HasSpawnedInSession(m_hostSteamID, m_worldSeed, m_characterName, m_characterSlot);
                }

                Utils::Logger::Info("Connected as CLIENT to Host SteamID: " + std::to_string(m_hostSteamID) +
                                    ", Seed: " + std::to_string(m_worldSeed) +
                                    ", Slot: " + std::to_string(m_characterSlot) +
                                    ", Name: '" + m_characterName + "'" +
                                    ", Level: " + std::to_string(m_characterLevel) +
                                    ", Fresh Char: " + (isFreshChar ? "Yes" : "No") +
                                    ", Already spawned before: " + (m_currentSessionSpawned ? "Yes" : "No"));
            } else if (m_role == SessionRole::Host) {
                m_currentSessionSpawned = true;
                Utils::Logger::Info("Acting as HOST (SteamID: " + std::to_string(m_hostSteamID) +
                                    ", Seed: " + std::to_string(m_worldSeed) +
                                    ", Character: '" + m_characterName + "')");
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
