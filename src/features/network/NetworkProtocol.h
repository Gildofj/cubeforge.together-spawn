#pragma once

#include "cwsdk.h"
#include <cstdint>
#include <cstring>

namespace TogetherSpawn {
namespace Features {
namespace Network {

    static constexpr int TOGETHER_SPAWN_P2P_CHANNEL = 42;
    static constexpr uint32_t PROTOCOL_MAGIC = 0x43575453; // "CWTS" (CubeWorld TogetherSpawn)
    static constexpr uint16_t PROTOCOL_VERSION = 1;

    enum class PacketType : uint8_t {
        Unknown = 0,
        SpawnRequest = 1,
        SpawnResponse = 2,
        TeleportRequest = 3,
        TeleportResponse = 4,
        PlayerLocationSync = 5
    };

    #pragma pack(push, 1)

    struct PacketHeader {
        uint32_t magic{PROTOCOL_MAGIC};
        uint16_t version{PROTOCOL_VERSION};
        PacketType type{PacketType::Unknown};
        uint8_t flags{0};

        bool IsValid() const {
            return magic == PROTOCOL_MAGIC && version == PROTOCOL_VERSION;
        }
    };

    struct SpawnRequestPacket {
        PacketHeader header{PROTOCOL_MAGIC, PROTOCOL_VERSION, PacketType::SpawnRequest, 0};
        uint64_t clientSteamID{0};
        int32_t worldSeed{0};
        int32_t characterSlot{0};
        char characterName[32]{0};

        SpawnRequestPacket() {
            header.type = PacketType::SpawnRequest;
        }
    };

    struct SpawnResponsePacket {
        PacketHeader header{PROTOCOL_MAGIC, PROTOCOL_VERSION, PacketType::SpawnResponse, 0};
        uint64_t hostSteamID{0};
        LongVector3 spawnPos{0, 0, 0};
        uint8_t success{0}; // 1 = success, 0 = failed
        char message[64]{0};

        SpawnResponsePacket() {
            header.type = PacketType::SpawnResponse;
        }
    };

    struct TeleportRequestPacket {
        PacketHeader header{PROTOCOL_MAGIC, PROTOCOL_VERSION, PacketType::TeleportRequest, 0};
        uint64_t requesterSteamID{0};
        uint64_t targetSteamID{0}; // 0 if searching by name
        char targetName[32]{0};

        TeleportRequestPacket() {
            header.type = PacketType::TeleportRequest;
        }
    };

    struct TeleportResponsePacket {
        PacketHeader header{PROTOCOL_MAGIC, PROTOCOL_VERSION, PacketType::TeleportResponse, 0};
        uint64_t targetSteamID{0};
        LongVector3 targetPos{0, 0, 0};
        uint8_t success{0}; // 1 = success, 0 = failed
        char targetName[32]{0};
        char errorMessage[64]{0};

        TeleportResponsePacket() {
            header.type = PacketType::TeleportResponse;
        }
    };

    #pragma pack(pop)

} // namespace Network
} // namespace Features
} // namespace TogetherSpawn
