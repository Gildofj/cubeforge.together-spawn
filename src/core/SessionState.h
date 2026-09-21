#pragma once

#include "cwsdk.h"
#include <chrono>
#include <map>
#include <string>

namespace TogetherSpawn {
namespace Core {

    enum class SessionRole {
        SinglePlayer,
        Host,
        Client
    };

    class SessionState {
    public:
        static SessionState& Instance();

        void Update(cube::Game* game);
        void Reset();

        SessionRole GetRole() const { return m_role; }
        uint64_t GetHostSteamID() const { return m_hostSteamID; }
        int GetWorldSeed() const { return m_worldSeed; }
        int GetCharacterSlot() const { return m_characterSlot; }

        bool IsInMultiplayerSession() const {
            return m_role == SessionRole::Host || m_role == SessionRole::Client;
        }

        void GrantInvulnerability(float durationSeconds);
        bool IsInvulnerable() const;

        bool CanUseTeleportCommand() const;
        void RecordTeleportCommandUsed();
        int GetRemainingCooldownSeconds() const;

        bool HasCurrentSessionSpawned() const { return m_currentSessionSpawned; }
        void SetCurrentSessionSpawned(bool spawned) { m_currentSessionSpawned = spawned; }

    private:
        SessionState() = default;
        ~SessionState() = default;

        SessionRole m_role{SessionRole::SinglePlayer};
        uint64_t m_hostSteamID{0};
        int m_worldSeed{0};
        int m_characterSlot{0};
        bool m_currentSessionSpawned{false};

        std::chrono::steady_clock::time_point m_invulnerabilityEndTime;
        std::chrono::steady_clock::time_point m_lastTeleportTime;
    };

} // namespace Core
} // namespace TogetherSpawn
