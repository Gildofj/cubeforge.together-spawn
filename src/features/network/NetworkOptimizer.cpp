#include "NetworkOptimizer.h"
#include "../spawn/SpawnManager.h"
#include "../../core/Config.h"
#include "../../core/SessionState.h"
#include "../../utils/Logger.h"
#include "../../utils/ModUtils.h"
#include <vector>
#include <algorithm>

namespace TogetherSpawn {
namespace Features {

    NetworkOptimizer& NetworkOptimizer::Instance() {
        static NetworkOptimizer instance;
        return instance;
    }

    int NetworkOptimizer::HandleP2PRequest(uint64_t steamID) {
        const auto& settings = Core::Config::Instance().GetSettings();
        if (settings.autoAcceptP2PRequests) {
            Utils::Logger::Info("P2P session request automatically accepted for SteamID: " + std::to_string(steamID));

            ISteamNetworking* steamNet = cube::SteamNetworking();
            if (steamNet) {
                CSteamID userSteamID;
                userSteamID.SetFromUint64(steamID);
                steamNet->AcceptP2PSessionWithUser(userSteamID);
            }

            return 1;
        }

        return 0;
    }

    bool NetworkOptimizer::SendPacket(uint64_t targetSteamID, const void* data, uint32_t size) {
        if (targetSteamID == 0 || !data || size == 0) return false;

        ISteamNetworking* steamNet = cube::SteamNetworking();
        if (!steamNet) {
            Utils::Logger::Error("SteamNetworking interface is not available.");
            return false;
        }

        CSteamID target;
        target.SetFromUint64(targetSteamID);
        return steamNet->SendP2PPacket(target, data, size, k_EP2PSendReliable, Network::TOGETHER_SPAWN_P2P_CHANNEL);
    }

    void NetworkOptimizer::BroadcastToClients(cube::Game* game, const void* data, uint32_t size) {
        if (!game || !game->host.running || !data || size == 0) return;

        ISteamNetworking* steamNet = cube::SteamNetworking();
        if (!steamNet) return;

        for (const auto& [steamId, conn] : game->host.connections) {
            uint64_t rawSteamID = steamId.ConvertToUint64();
            if (rawSteamID != 0) {
                steamNet->SendP2PPacket(steamId, data, size, k_EP2PSendReliable, Network::TOGETHER_SPAWN_P2P_CHANNEL);
            }
        }
    }

    void NetworkOptimizer::SendSpawnRequest(cube::Game* game, uint64_t hostSteamID) {
        if (hostSteamID == 0) return;

        Network::SpawnRequestPacket packet;
        uint64_t mySteamID = Core::SessionState::Instance().GetLocalSteamID();
        if (mySteamID == 0 && cube::SteamUser()) {
            mySteamID = cube::SteamUser()->GetSteamID().ConvertToUint64();
        }
        if (mySteamID == 0 && game && game->world && game->world->local_creature) {
            mySteamID = static_cast<uint64_t>(game->world->local_creature->entity_data.steam_id);
        }

        packet.clientSteamID = mySteamID;
        if (game && game->world && game->world->local_creature) {
            std::string name(game->world->local_creature->entity_data.name);
            strncpy_s(packet.characterName, name.c_str(), sizeof(packet.characterName) - 1);
        }
        packet.worldSeed = game ? game->seed : 0;
        packet.characterSlot = game ? game->current_character_slot : 0;

        Utils::Logger::Info("Sending P2P SpawnRequest to Host SteamID: " + std::to_string(hostSteamID) +
                            " from Client SteamID: " + std::to_string(mySteamID));
        SendPacket(hostSteamID, &packet, sizeof(packet));
    }

    void NetworkOptimizer::SendSpawnResponse(uint64_t clientSteamID, const LongVector3& spawnPos, bool success, const std::string& message) {
        if (clientSteamID == 0) return;

        Network::SpawnResponsePacket packet;
        packet.hostSteamID = Core::SessionState::Instance().GetHostSteamID();
        packet.spawnPos = spawnPos;
        packet.success = success ? 1 : 0;
        if (!message.empty()) {
            strncpy_s(packet.message, message.c_str(), sizeof(packet.message) - 1);
        }

        Utils::Logger::Info("Sending P2P SpawnResponse to Client SteamID: " + std::to_string(clientSteamID) +
                            " (Success: " + (success ? "True" : "False") + ")");
        SendPacket(clientSteamID, &packet, sizeof(packet));
    }

    void NetworkOptimizer::SendTeleportRequest(uint64_t hostSteamID, const std::string& targetName, uint64_t targetSteamID) {
        if (hostSteamID == 0) return;

        Network::TeleportRequestPacket packet;
        auto local = Utils::GetLocalPlayer();
        if (local) {
            packet.requesterSteamID = static_cast<uint64_t>(local->entity_data.steam_id);
        }
        packet.targetSteamID = targetSteamID;
        if (!targetName.empty()) {
            strncpy_s(packet.targetName, targetName.c_str(), sizeof(packet.targetName) - 1);
        }

        Utils::Logger::Info("Sending P2P TeleportRequest to Host for target: " + targetName);
        SendPacket(hostSteamID, &packet, sizeof(packet));
    }

    void NetworkOptimizer::SendTeleportResponse(uint64_t requesterSteamID, const LongVector3& targetPos, bool success, const std::string& targetName, const std::string& errorMsg) {
        if (requesterSteamID == 0) return;

        Network::TeleportResponsePacket packet;
        packet.targetSteamID = requesterSteamID;
        packet.targetPos = targetPos;
        packet.success = success ? 1 : 0;
        if (!targetName.empty()) {
            strncpy_s(packet.targetName, targetName.c_str(), sizeof(packet.targetName) - 1);
        }
        if (!errorMsg.empty()) {
            strncpy_s(packet.errorMessage, errorMsg.c_str(), sizeof(packet.errorMessage) - 1);
        }

        Utils::Logger::Info("Sending P2P TeleportResponse to Requester SteamID: " + std::to_string(requesterSteamID) +
                            " (Success: " + (success ? "True" : "False") + ")");
        SendPacket(requesterSteamID, &packet, sizeof(packet));
    }

    void NetworkOptimizer::ProcessIncomingPackets(cube::Game* game) {
        if (!game) return;

        ISteamNetworking* steamNet = cube::SteamNetworking();
        if (!steamNet) return;

        uint32 msgSize = 0;
        while (steamNet->IsP2PPacketAvailable(&msgSize, Network::TOGETHER_SPAWN_P2P_CHANNEL)) {
            if (msgSize == 0 || msgSize > 4096) {
                // Discard invalid packet size
                uint32 bytesRead = 0;
                CSteamID dummy;
                char discardBuf[1];
                steamNet->ReadP2PPacket(discardBuf, 0, &bytesRead, &dummy, Network::TOGETHER_SPAWN_P2P_CHANNEL);
                continue;
            }

            std::vector<uint8_t> buffer(msgSize);
            uint32 bytesRead = 0;
            CSteamID senderSteamID;

            if (steamNet->ReadP2PPacket(buffer.data(), msgSize, &bytesRead, &senderSteamID, Network::TOGETHER_SPAWN_P2P_CHANNEL)) {
                if (bytesRead < sizeof(Network::PacketHeader)) continue;

                const auto* header = reinterpret_cast<const Network::PacketHeader*>(buffer.data());
                if (!header->IsValid()) {
                    Utils::Logger::Warn("Received TogetherSpawn P2P packet with invalid magic/version.");
                    continue;
                }

                uint64_t senderRaw = senderSteamID.ConvertToUint64();

                switch (header->type) {
                    case Network::PacketType::SpawnRequest:
                        if (bytesRead >= sizeof(Network::SpawnRequestPacket)) {
                            const auto* pkt = reinterpret_cast<const Network::SpawnRequestPacket*>(buffer.data());
                            HandleSpawnRequest(game, *pkt, senderRaw);
                        }
                        break;

                    case Network::PacketType::SpawnResponse:
                        if (bytesRead >= sizeof(Network::SpawnResponsePacket)) {
                            const auto* pkt = reinterpret_cast<const Network::SpawnResponsePacket*>(buffer.data());
                            HandleSpawnResponse(game, *pkt, senderRaw);
                        }
                        break;

                    case Network::PacketType::TeleportRequest:
                        if (bytesRead >= sizeof(Network::TeleportRequestPacket)) {
                            const auto* pkt = reinterpret_cast<const Network::TeleportRequestPacket*>(buffer.data());
                            HandleTeleportRequest(game, *pkt, senderRaw);
                        }
                        break;

                    case Network::PacketType::TeleportResponse:
                        if (bytesRead >= sizeof(Network::TeleportResponsePacket)) {
                            const auto* pkt = reinterpret_cast<const Network::TeleportResponsePacket*>(buffer.data());
                            HandleTeleportResponse(game, *pkt, senderRaw);
                        }
                        break;

                    default:
                        break;
                }
            }
        }
    }

    void NetworkOptimizer::HandleSpawnRequest(cube::Game* game, const Network::SpawnRequestPacket& packet, uint64_t senderSteamID) {
        uint64_t targetClientSteamID = (senderSteamID != 0) ? senderSteamID : packet.clientSteamID;

        Utils::Logger::Info("Received SpawnRequest from SteamID: " + std::to_string(targetClientSteamID) +
                            " (Player: " + std::string(packet.characterName) + ")");

        if (!game || !game->world || !game->world->local_creature) {
            SendSpawnResponse(targetClientSteamID, {0, 0, 0}, false, "Host world is not ready");
            return;
        }

        const auto& settings = Core::Config::Instance().GetSettings();
        LongVector3 spawnPos;

        if (settings.customTeamSpawnEnabled) {
            spawnPos = settings.customTeamSpawnPos;
            Utils::Logger::Info("Using configured Custom Team Spawn coordinates.");
        } else {
            LongVector3 hostPos = game->world->local_creature->entity_data.position;
            auto safePosOpt = SpawnManager::Instance().CalculateSafeGroundPosition(game, hostPos, settings.spawnRadiusInBlocks);
            spawnPos = safePosOpt.value_or(hostPos);
        }

        SendSpawnResponse(targetClientSteamID, spawnPos, true, "Spawn location assigned");

        std::string clientName = (packet.characterName[0] != '\0') ? packet.characterName : ("SteamID " + std::to_string(targetClientSteamID));
        Utils::PrintChat(L"[TogetherSpawn] Jogador " + Utils::Utf8ToWide(clientName) + L" sincronizado e spawnado no seu mundo.", Utils::Colors::Emerald);
    }

    void NetworkOptimizer::HandleSpawnResponse(cube::Game* game, const Network::SpawnResponsePacket& packet, uint64_t senderSteamID) {
        Utils::Logger::Info("Received SpawnResponse from Host SteamID: " + std::to_string(senderSteamID) +
                            " (Success: " + (packet.success ? "True" : "False") + ")");

        if (packet.success) {
            SpawnManager::Instance().OnSpawnCoordinatesReceived(game, packet.spawnPos);
        } else {
            std::string errMsg = (packet.message[0] != '\0') ? packet.message : "Host falhou ao gerar coordenadas de spawn.";
            Utils::PrintChat(L"[TogetherSpawn] " + Utils::Utf8ToWide(errMsg), Utils::Colors::Orange);
        }
    }

    void NetworkOptimizer::HandleTeleportRequest(cube::Game* game, const Network::TeleportRequestPacket& packet, uint64_t senderSteamID) {
        if (!game || !game->world) return;

        Utils::Logger::Info("Received TeleportRequest from SteamID: " + std::to_string(senderSteamID) +
                            " for target: '" + std::string(packet.targetName) + "'");

        cube::Creature* targetCreature = nullptr;

        // 1. Check if target is Host by SteamID or by name
        if (packet.targetSteamID != 0) {
            if (game->world->local_creature &&
                static_cast<uint64_t>(game->world->local_creature->entity_data.steam_id) == packet.targetSteamID) {
                targetCreature = game->world->local_creature;
            } else {
                targetCreature = Utils::FindPlayerBySteamID(packet.targetSteamID);
            }
        }

        // 2. If not found, search by name
        if (!targetCreature && packet.targetName[0] != '\0') {
            std::string searchName = packet.targetName;
            std::transform(searchName.begin(), searchName.end(), searchName.begin(), ::tolower);

            if (game->world->local_creature) {
                std::string hostName(game->world->local_creature->entity_data.name);
                std::string lowerHost = hostName;
                std::transform(lowerHost.begin(), lowerHost.end(), lowerHost.begin(), ::tolower);
                if (lowerHost.find(searchName) != std::string::npos) {
                    targetCreature = game->world->local_creature;
                }
            }

            if (!targetCreature) {
                targetCreature = Utils::FindPlayerByName(packet.targetName);
            }
        }

        if (targetCreature) {
            const auto& settings = Core::Config::Instance().GetSettings();
            auto safePosOpt = SpawnManager::Instance().CalculateSafeGroundPosition(game, targetCreature->entity_data.position, settings.spawnRadiusInBlocks);
            LongVector3 destPos = safePosOpt.value_or(targetCreature->entity_data.position);

            std::string tName(targetCreature->entity_data.name);
            SendTeleportResponse(senderSteamID, destPos, true, tName, "");
        } else {
            SendTeleportResponse(senderSteamID, {0, 0, 0}, false, packet.targetName, "Jogador nao encontrado no servidor.");
        }
    }

    void NetworkOptimizer::HandleTeleportResponse(cube::Game* game, const Network::TeleportResponsePacket& packet, uint64_t senderSteamID) {
        SpawnManager::Instance().OnTeleportCoordinatesReceived(game, packet);
    }

    void NetworkOptimizer::Update(cube::Game* game) {
        if (!game) return;

        ProcessIncomingPackets(game);

        m_tickCounter++;
        if (m_tickCounter % 300 == 0) {
            if (game->host.running) {
                size_t numClients = game->host.connections.size();
                Utils::Logger::Debug("Host Watchdog: " + std::to_string(numClients) + " active P2P client connections.");
            } else if (game->client.host_steam_id.ConvertToUint64() != 0) {
                Utils::Logger::Debug("Client Watchdog: Connected to host SteamID " +
                                     std::to_string(game->client.host_steam_id.ConvertToUint64()));
            }
        }
    }

} // namespace Features
} // namespace TogetherSpawn
