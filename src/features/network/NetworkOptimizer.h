#pragma once

#include "cwsdk.h"
#include "NetworkProtocol.h"
#include <cstdint>
#include <string>

namespace TogetherSpawn {
namespace Features {

    class NetworkOptimizer {
    public:
        static NetworkOptimizer& Instance();

        int HandleP2PRequest(uint64_t steamID);
        void Update(cube::Game* game);

        bool SendPacket(uint64_t targetSteamID, const void* data, uint32_t size);
        void BroadcastToClients(cube::Game* game, const void* data, uint32_t size);

        void SendSpawnRequest(cube::Game* game, uint64_t hostSteamID);
        void SendSpawnResponse(uint64_t clientSteamID, const LongVector3& spawnPos, bool success, const std::string& message = "");

        void SendTeleportRequest(uint64_t hostSteamID, const std::string& targetName, uint64_t targetSteamID = 0);
        void SendTeleportResponse(uint64_t requesterSteamID, const LongVector3& targetPos, bool success, const std::string& targetName, const std::string& errorMsg = "");

    private:
        NetworkOptimizer() = default;
        ~NetworkOptimizer() = default;

        void ProcessIncomingPackets(cube::Game* game);
        void HandleSpawnRequest(cube::Game* game, const Network::SpawnRequestPacket& packet, uint64_t senderSteamID);
        void HandleSpawnResponse(cube::Game* game, const Network::SpawnResponsePacket& packet, uint64_t senderSteamID);
        void HandleTeleportRequest(cube::Game* game, const Network::TeleportRequestPacket& packet, uint64_t senderSteamID);
        void HandleTeleportResponse(cube::Game* game, const Network::TeleportResponsePacket& packet, uint64_t senderSteamID);

        uint64_t m_tickCounter{0};
    };

} // namespace Features
} // namespace TogetherSpawn
