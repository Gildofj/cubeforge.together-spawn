#pragma once

#include "cwsdk.h"
#include <optional>

namespace TogetherSpawn {
namespace Features {

    class SpawnManager {
    public:
        static SpawnManager& Instance();

        void Initialize();
        void Update(cube::Game* game);
        cube::Creature* FindHostCreature(cube::Game* game);
        std::optional<LongVector3> CalculateSafeGroundPosition(cube::Game* game, const LongVector3& targetPos, float preferredRadiusBlocks);
        bool ExecuteSpawnNearHost(cube::Game* game, bool force = false);
        bool TeleportToPlayer(cube::Game* game, cube::Creature* target);

    private:
        SpawnManager() = default;
        ~SpawnManager() = default;

        int m_spawnAttemptCooldownTicks{0};
    };

} // namespace Features
} // namespace TogetherSpawn
