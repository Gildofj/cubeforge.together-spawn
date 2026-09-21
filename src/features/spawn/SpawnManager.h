#pragma once

#include "cwsdk.h"
#include <optional>

namespace TogetherSpawn {
namespace Features {

    class SpawnManager {
    public:
        static SpawnManager& Instance();

        /**
         * @brief Called on mod initialization to apply memory patches (e.g., spawn region restrictions).
         */
        void Initialize();

        /**
         * @brief Called every frame to monitor session state and trigger first-time spawn when ready.
         */
        void Update(cube::Game* game);

        /**
         * @brief Attempts to find the host creature in the current session.
         */
        cube::Creature* FindHostCreature(cube::Game* game);

        /**
         * @brief Finds a safe ground position near a target entity.
         * @param game Pointer to Game instance.
         * @param targetPos Target reference position.
         * @param preferredRadiusBlocks Preferred distance in blocks.
         * @return Safe ground LongVector3 or std::nullopt if terrain is not yet loaded.
         */
        std::optional<LongVector3> CalculateSafeGroundPosition(cube::Game* game, const LongVector3& targetPos, float preferredRadiusBlocks);

        /**
         * @brief Executes first-time spawn near host for the local player.
         * @param force If true, bypasses first-time check (e.g. for /spawn command).
         * @return true if teleported successfully, false otherwise.
         */
        bool ExecuteSpawnNearHost(cube::Game* game, bool force = false);

        /**
         * @brief Teleports the local player directly to a target player.
         */
        bool TeleportToPlayer(cube::Game* game, cube::Creature* target);

    private:
        SpawnManager() = default;
        ~SpawnManager() = default;

        int m_spawnAttemptCooldownTicks{0};
    };

} // namespace Features
} // namespace TogetherSpawn
