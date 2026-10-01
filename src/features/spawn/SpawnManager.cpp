#include "SpawnManager.h"
#include "../network/NetworkOptimizer.h"
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
        cube::Game::SetRestrictedSpawnRegions(false);
        Utils::Logger::Info("Disabled restricted spawn regions for open multiplayer world roaming.");
    }

    void SpawnManager::ResetCooldown() {
        m_spawnAttemptCooldownTicks = 0;
        m_spawnRequestPending = false;
    }

    cube::Creature* SpawnManager::FindHostCreature(cube::Game* game) {
        if (!game || !game->world) return nullptr;

        uint64_t hostSteamID = Core::SessionState::Instance().GetHostSteamID();
        if (hostSteamID == 0) {
            hostSteamID = game->client.host_steam_id.ConvertToUint64();
        }

        if (hostSteamID != 0) {
            cube::Creature* host = Utils::FindPlayerBySteamID(hostSteamID);
            if (host && host != game->world->local_creature) return host;
        }

        return nullptr;
    }

    std::optional<LongVector3> SpawnManager::CalculateSafeGroundPosition(cube::Game* game, const LongVector3& targetPos, float preferredRadiusBlocks) {
        if (!game || !game->world) return std::nullopt;

        i64 targetBlockX = Utils::MathUtils::WorldToBlock(targetPos.x);
        i64 targetBlockY = Utils::MathUtils::WorldToBlock(targetPos.y);
        i64 targetBlockZ = Utils::MathUtils::WorldToBlock(targetPos.z);

        const int numAngles = 8;
        for (int i = 0; i < numAngles; ++i) {
            float angle = static_cast<float>(i * (2.0 * Utils::MathUtils::PI / numAngles));
            i64 offsetXBlocks = static_cast<i64>(std::cos(angle) * preferredRadiusBlocks);
            i64 offsetYBlocks = static_cast<i64>(std::sin(angle) * preferredRadiusBlocks);

            i64 candidateBlockX = targetBlockX + offsetXBlocks;
            i64 candidateBlockY = targetBlockY + offsetYBlocks;

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

        LongVector3 fallbackPos = Utils::MathUtils::CalculateRadialOffset(targetPos, preferredRadiusBlocks, 0.0f);
        fallbackPos.z += (DOTS_PER_BLOCK * 2);
        return fallbackPos;
    }

    void SpawnManager::RequestSpawnFromHost(cube::Game* game, bool force) {
        if (!game) return;

        auto& session = Core::SessionState::Instance();
        if (session.GetRole() != Core::SessionRole::Client) {
            if (force) {
                Utils::PrintChat(L"[TogetherSpawn] Voce e o Host desta sessao.", Utils::Colors::Gold);
            }
            return;
        }

        if (!force && session.HasCurrentSessionSpawned()) {
            return;
        }

        uint64_t hostSteamID = session.GetHostSteamID();
        if (hostSteamID == 0) {
            hostSteamID = game->client.host_steam_id.ConvertToUint64();
        }

        if (hostSteamID != 0) {
            NetworkOptimizer::Instance().SendSpawnRequest(game, hostSteamID);
            m_spawnRequestPending = true;
        }
    }

    bool SpawnManager::OnSpawnCoordinatesReceived(cube::Game* game, const LongVector3& spawnPos) {
        if (!game || !game->world || !game->world->local_creature) return false;

        cube::Creature* localPlayer = game->world->local_creature;
        auto& session = Core::SessionState::Instance();
        const auto& settings = Core::Config::Instance().GetSettings();

        if (Utils::TeleportCreature(localPlayer, spawnPos)) {
            session.SetCurrentSessionSpawned(true);
            Core::Config::Instance().MarkSessionAsSpawned(session.GetHostSteamID(), session.GetWorldSeed(), session.GetCharacterName(), session.GetCharacterSlot());
            session.GrantInvulnerability(settings.invulnerabilitySecondsAfterSpawn);

            Utils::PrintChat(L"--------------------------------------------------", Utils::Colors::Cyan);
            Utils::PrintChat(L"[TogetherSpawn] Voce spawnou com sucesso junto ao Host!", Utils::Colors::Emerald);
            Utils::PrintChat(L"[TogetherSpawn] Protecao de aterrissagem ativa temporariamente.", Utils::Colors::Gold);
            Utils::PrintChat(L"--------------------------------------------------", Utils::Colors::Cyan);

            Utils::Logger::Info("P2P First-time spawn near host completed successfully at (" +
                                std::to_string(spawnPos.x) + ", " +
                                std::to_string(spawnPos.y) + ", " +
                                std::to_string(spawnPos.z) + ") for character: " + session.GetCharacterName());
            m_spawnRequestPending = false;
            return true;
        }

        return false;
    }

    void SpawnManager::RequestTeleportToHost(cube::Game* game) {
        if (!game) return;

        auto& session = Core::SessionState::Instance();
        if (session.GetRole() != Core::SessionRole::Client) {
            Utils::PrintChat(L"[TogetherSpawn] Voce e o Host ou esta em Singleplayer.", Utils::Colors::Gold);
            return;
        }

        if (!session.CanUseTeleportCommand()) {
            int rem = session.GetRemainingCooldownSeconds();
            Utils::PrintChat(L"[TogetherSpawn] Comando em cooldown. Aguarde " + std::to_wstring(rem) + L"s.", Utils::Colors::Orange);
            return;
        }

        // Check if host creature is already in local vision
        cube::Creature* host = FindHostCreature(game);
        if (host) {
            if (TeleportToPlayer(game, host)) {
                session.RecordTeleportCommandUsed();
                Utils::PrintChat(L"[TogetherSpawn] Teleportado com sucesso para o Host!", Utils::Colors::Emerald);
            }
            return;
        }

        // Host is far away in another region: request remote coordinates via P2P
        uint64_t hostSteamID = session.GetHostSteamID();
        if (hostSteamID != 0) {
            Utils::PrintChat(L"[TogetherSpawn] Localizando Host no servidor...", Utils::Colors::Cyan);
            NetworkOptimizer::Instance().SendTeleportRequest(hostSteamID, "", hostSteamID);
        } else {
            Utils::PrintChat(L"[TogetherSpawn] Host nao encontrado.", Utils::Colors::Red);
        }
    }

    void SpawnManager::RequestTeleportToPlayer(cube::Game* game, const std::string& targetName) {
        if (!game || targetName.empty()) return;

        auto& session = Core::SessionState::Instance();
        if (!session.CanUseTeleportCommand()) {
            int rem = session.GetRemainingCooldownSeconds();
            Utils::PrintChat(L"[TogetherSpawn] Comando em cooldown. Aguarde " + std::to_wstring(rem) + L"s.", Utils::Colors::Orange);
            return;
        }

        // Check if player is loaded locally
        cube::Creature* target = Utils::FindPlayerByName(targetName);
        if (target) {
            if (Utils::IsLocalPlayer(target)) {
                Utils::PrintChat(L"[TogetherSpawn] Voce ja esta na sua propria posicao.", Utils::Colors::Orange);
                return;
            }
            if (TeleportToPlayer(game, target)) {
                session.RecordTeleportCommandUsed();
            }
            return;
        }

        // If client and player is remote: query Host via P2P
        if (session.GetRole() == Core::SessionRole::Client) {
            uint64_t hostSteamID = session.GetHostSteamID();
            if (hostSteamID != 0) {
                Utils::PrintChat(L"[TogetherSpawn] Buscando coordenadas do jogador via Host...", Utils::Colors::Cyan);
                NetworkOptimizer::Instance().SendTeleportRequest(hostSteamID, targetName, 0);
                return;
            }
        }

        Utils::PrintChat(L"[TogetherSpawn] Jogador '" + Utils::Utf8ToWide(targetName) + L"' nao encontrado.", Utils::Colors::Red);
    }

    bool SpawnManager::OnTeleportCoordinatesReceived(cube::Game* game, const Network::TeleportResponsePacket& response) {
        if (!game) return false;

        if (response.success) {
            std::string targetLabel = (response.targetName[0] != '\0') ? response.targetName : "Jogador";
            if (TeleportToPosition(game, response.targetPos, targetLabel)) {
                Core::SessionState::Instance().RecordTeleportCommandUsed();
                return true;
            }
        } else {
            std::string errMsg = (response.errorMessage[0] != '\0') ? response.errorMessage : "Falha ao localizar destino.";
            Utils::PrintChat(L"[TogetherSpawn] " + Utils::Utf8ToWide(errMsg), Utils::Colors::Red);
        }

        return false;
    }

    bool SpawnManager::TeleportToPosition(cube::Game* game, const LongVector3& targetPos, const std::string& destinationLabel) {
        if (!game || !game->world || !game->world->local_creature) return false;

        cube::Creature* localPlayer = game->world->local_creature;
        const auto& settings = Core::Config::Instance().GetSettings();

        if (Utils::TeleportCreature(localPlayer, targetPos)) {
            Core::SessionState::Instance().GrantInvulnerability(settings.invulnerabilitySecondsAfterSpawn);
            if (!destinationLabel.empty()) {
                Utils::PrintChat(L"[TogetherSpawn] Teleportado para: " + Utils::Utf8ToWide(destinationLabel), Utils::Colors::Green);
            }
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
        std::string targetName(target->entity_data.name);
        return TeleportToPosition(game, dest, targetName);
    }

    bool SpawnManager::ExecuteSpawnNearHost(cube::Game* game, bool force) {
        RequestSpawnFromHost(game, force);
        return true;
    }

    void SpawnManager::Update(cube::Game* game) {
        if (!game || !game->world || !game->world->local_creature) return;

        cube::Game::SetRestrictedSpawnRegions(false);

        auto& session = Core::SessionState::Instance();
        const auto& settings = Core::Config::Instance().GetSettings();

        if (settings.autoSpawnNearHostOnFirstJoin &&
            session.GetRole() == Core::SessionRole::Client &&
            !session.HasCurrentSessionSpawned()) {

            m_spawnAttemptCooldownTicks++;
            // Trigger quickly upon load (15 ticks = 0.25s) and retry every 60 ticks (1s)
            if (m_spawnAttemptCooldownTicks == 15 || m_spawnAttemptCooldownTicks >= 60) {
                if (m_spawnAttemptCooldownTicks >= 60) {
                    m_spawnAttemptCooldownTicks = 0;
                }
                RequestSpawnFromHost(game, false);
            }
        }
    }

} // namespace Features
} // namespace TogetherSpawn
