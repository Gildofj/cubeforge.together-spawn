#pragma once

#include "cwsdk.h"
#include <string>
#include <set>
#include <mutex>

namespace TogetherSpawn {
namespace Core {

    struct ModSettings {
        bool autoSpawnNearHostOnFirstJoin{true};
        float spawnRadiusInBlocks{4.0f};
        bool autoAcceptP2PRequests{true};
        float invulnerabilitySecondsAfterSpawn{5.0f};
        bool allowTeleportCommands{true};
        int teleportCooldownSeconds{10};
        bool customTeamSpawnEnabled{false};
        LongVector3 customTeamSpawnPos{0, 0, 0};
        bool showOverlayUI{false};
    };

    class Config {
    public:
        static Config& Instance();

        void Load();
        void Save();

        ModSettings& GetSettings();
        const ModSettings& GetSettings() const;

        bool HasSpawnedInSession(uint64_t hostSteamID, int worldSeed, int characterSlot) const;
        void MarkSessionAsSpawned(uint64_t hostSteamID, int worldSeed, int characterSlot);
        void ResetSessionSpawn(uint64_t hostSteamID, int worldSeed, int characterSlot);
        void ClearAllSpawnHistory();

    private:
        Config();
        ~Config() = default;

        std::string MakeSessionKey(uint64_t hostSteamID, int worldSeed, int characterSlot) const;
        std::string GetConfigPath() const;

        ModSettings m_settings;
        std::set<std::string> m_spawnedSessions;
        mutable std::mutex m_mutex;
    };

} // namespace Core
} // namespace TogetherSpawn
