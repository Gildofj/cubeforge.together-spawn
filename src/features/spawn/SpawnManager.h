#pragma once

#include "cwsdk.h"
#include "../network/NetworkProtocol.h"
#include <optional>
#include <string>

namespace TogetherSpawn {
namespace Features {

    class SpawnManager {
    public:
        static SpawnManager& Instance();

        void Initialize();
        void Update(cube::Game* game);

        cube::Creature* FindHostCreature(cube::Game* game);
        std::optional<LongVector3> CalculateSafeGroundPosition(cube::Game* game, const LongVector3& targetPos, float preferredRadiusBlocks);

        void RequestSpawnFromHost(cube::Game* game, bool force = false);
        bool OnSpawnCoordinatesReceived(cube::Game* game, const LongVector3& spawnPos);

        void RequestTeleportToHost(cube::Game* game);
        void RequestTeleportToPlayer(cube::Game* game, const std::string& targetName);
        bool OnTeleportCoordinatesReceived(cube::Game* game, const Network::TeleportResponsePacket& response);

        bool ExecuteSpawnNearHost(cube::Game* game, bool force = false);
        bool TeleportToPlayer(cube::Game* game, cube::Creature* target);
        bool TeleportToPosition(cube::Game* game, const LongVector3& targetPos, const std::string& destinationLabel = "");

    private:
        SpawnManager() = default;
        ~SpawnManager() = default;

        int m_spawnAttemptCooldownTicks{0};
        bool m_spawnRequestPending{false};
    };

} // namespace Features
} // namespace TogetherSpawn
