#include "SpawnManager.h"
#include "../../core/Config.h"
#include "../../core/SessionState.h"
#include "../../utils/ModUtils.h"
#include "../../utils/MathUtils.h"
#include "../../utils/Logger.h"
#include "cube/constants.h"

namespace TogetherSpawn {
namespace Features {

    using cube::DOTS_PER_BLOCK;

    SpawnManager& SpawnManager::Instance() {
        static SpawnManager instance;
        return instance;
    }

    void SpawnManager::Initialize() {
        // Unlock regional spawn restrictions so multiplayer players can spawn freely in any region
        cube::Game::SetRestrictedSpawnRegions(false);
        Utils::Logger::Info("Disabled restricted spawn regions for open multiplayer world roaming.");
    }

    cube::Creature* SpawnManager::FindHostCreature(cube::Game* game) {
        if (!game || !game->world) return nullptr;

        uint64_t hostSteamID = game->client.host_steam_id.ConvertToUint64();
        if (hostSteamID != 0) {
            cube::Creature* host = Utils::FindPlayerBySteamID(hostSteamID);
            if (host) return host;
        }

        // Fallback: If hostSteamID is not explicitly matched in creature list, search for any other player
        for (cube::Creature* c : game->world->creatures) {
            if (c && c != game->world->local_creature &&
                c->entity_data.hostility_type == cube::Creature::EntityBehaviour::Player) {
                return c;
            }
        }

        return nullptr;
    }

    std::optional<LongVector3> SpawnManager::CalculateSafeGroundPosition(cube::Game* game, const LongVector3& targetPos, float preferredRadiusBlocks) {
        if (!game || !game->world) return std::nullopt;

        i64 targetBlockX = Utils::MathUtils::WorldToBlock(targetPos.x);
        i64 targetBlockY = Utils::MathUtils::WorldToBlock(targetPos.y);
        i64 targetBlockZ = Utils::MathUtils::WorldToBlock(targetPos.z);

        // Try 8 radial directions around the target
        const int numAngles = 8;
        for (int i = 0; i < numAngles; ++i) {
            float angle = static_cast<float>(i * (2.0 * Utils::MathUtils::PI / numAngles));
            i64 offsetXBlocks = static_cast<i64>(std::cos(angle) * preferredRadiusBlocks);
            i64 offsetYBlocks = static_cast<i64>(std::sin(angle) * preferredRadiusBlocks);

            i64 candidateBlockX = targetBlockX + offsetXBlocks;
            i64 candidateBlockY = targetBlockY + offsetYBlocks;

            // Search vertically for solid ground with 2 air blocks above
            for (i64 z = targetBlockZ + 6; z >= targetBlockZ - 12; --z) {
                cube::Block groundBlock = game->world->GetBlockInterpolated(candidateBlockX, candidateBlockY, z - 1);
                cube::Block feetBlock   = game->world->GetBlockInterpolated(candidateBlockX, candidateBlockY, z);
                cube::Block headBlock   = game->world->GetBlockInterpolated(candidateBlockX, candidateBlockY, z + 1);

                bool isGroundSolid = (groundBlock.type != cube::Block::Type::Air &&
                                      groundBlock.type != cube::Block::Type::Water &&
                                      groundBlock.type != cube::Block::Type::Lava);

                bool isClearAir = (feetBlock.type == cube::Block::Type::Air &&
                                   headBlock.type == cube::Block::Type::Air);

                if (isGroundSolid && isClearAir) {
                    LongVector3 safePos(
                        Utils::MathUtils::BlockToWorld(candidateBlockX),
                        Utils::MathUtils::BlockToWorld(candidateBlockY),
                        Utils::MathUtils::BlockToWorld(z)
                    );
                    return safePos;
                }
            }
        }

        // If specific raycast failed, return offset with target elevation + 1 block
        LongVector3 fallbackPos = Utils::MathUtils::CalculateRadialOffset(targetPos, preferredRadiusBlocks, 0.0f);
        fallbackPos.z += DOTS_PER_BLOCK;
        return fallbackPos;
    }

    bool SpawnManager::ExecuteSpawnNearHost(cube::Game* game, bool force) {
        if (!game || !game->world) return false;

        cube::Creature* localPlayer = game->world->local_creature;
        if (!localPlayer) return false;

        auto& session = Core::SessionState::Instance();
        const auto& settings = Core::Config::Instance().GetSettings();

        if (session.GetRole() != Core::SessionRole::Client) {
            if (force) {
                Utils::PrintChat(L"[TogetherSpawn] Voce e o Host desta sessao.", Utils::Colors::Gold);
            }
            return false;
        }

        if (!force && session.HasCurrentSessionSpawned()) {
            return false;
        }

        cube::Creature* hostCreature = FindHostCreature(game);
        if (!hostCreature) {
            Utils::Logger::Debug("Host creature not yet loaded in creature list.");
            return false;
        }

        // Calculate safe ground spot near host
        auto safePosOpt = CalculateSafeGroundPosition(game, hostCreature->entity_data.position, settings.spawnRadiusInBlocks);
        if (!safePosOpt.has_value()) {
            Utils::Logger::Warn("Could not find safe terrain around host yet.");
            return false;
        }

        LongVector3 safePos = safePosOpt.value();
        if (Utils::TeleportCreature(localPlayer, safePos)) {
            session.SetCurrentSessionSpawned(true);
            Core::Config::Instance().MarkSessionAsSpawned(session.GetHostSteamID(), session.GetWorldSeed(), session.GetCharacterSlot());

            // Grant safe-landing temporary invulnerability
            session.GrantInvulnerability(settings.invulnerabilitySecondsAfterSpawn);

            // Announce to local chat
            Utils::PrintChat(L"--------------------------------------------------", Utils::Colors::Cyan);
            Utils::PrintChat(L"[TogetherSpawn] Voce spawnou com sucesso proximo ao Host!", Utils::Colors::Emerald);
            Utils::PrintChat(L"[TogetherSpawn] Protecao de aterrissagem ativa temporariamente.", Utils::Colors::Gold);
            Utils::PrintChat(L"--------------------------------------------------", Utils::Colors::Cyan);

            Utils::Logger::Info("First-time spawn near host completed successfully.");
            return true;
        }

        return false;
    }

    bool SpawnManager::TeleportToPlayer(cube::Game* game, cube::Creature* target) {
        if (!game || !game->world || !target) return false;

        cube::Creature* localPlayer = game->world->local_creature;
        if (!localPlayer) return false;

        const auto& settings = Core::Config::Instance().GetSettings();
        auto safePosOpt = CalculateSafeGroundPosition(game, target->entity_data.position, settings.spawnRadiusInBlocks);

        LongVector3 dest = safePosOpt.value_or(target->entity_data.position);
        if (Utils::TeleportCreature(localPlayer, dest)) {
            Core::SessionState::Instance().GrantInvulnerability(settings.invulnerabilitySecondsAfterSpawn);
            Core::SessionState::Instance().RecordTeleportCommandUsed();

            std::string targetName(target->entity_data.name);
            Utils::PrintChat(L"[TogetherSpawn] Teleportado para: " + Utils::Utf8ToWide(targetName), Utils::Colors::Green);
            return true;
        }

        return false;
    }

    void SpawnManager::Update(cube::Game* game) {
        if (!game || !game->world || !game->world->local_creature) return;

        // Ensure spawn regions remain unrestricted
        cube::Game::SetRestrictedSpawnRegions(false);

        auto& session = Core::SessionState::Instance();
        const auto& settings = Core::Config::Instance().GetSettings();

        // Check if auto-spawn is enabled and player is a client needing first spawn
        if (settings.autoSpawnNearHostOnFirstJoin &&
            session.GetRole() == Core::SessionRole::Client &&
            !session.HasCurrentSessionSpawned()) {

            // Rate-limit attempt every 30 ticks (approx 0.5s) to allow world chunks to populate
            m_spawnAttemptCooldownTicks++;
            if (m_spawnAttemptCooldownTicks >= 30) {
                m_spawnAttemptCooldownTicks = 0;
                ExecuteSpawnNearHost(game, false);
            }
        }
    }

} // namespace Features
} // namespace TogetherSpawn
