#pragma once

#include "cwsdk.h"
#include <cstdint>

namespace TogetherSpawn {
namespace Features {

    class NetworkOptimizer {
    public:
        static NetworkOptimizer& Instance();

        /**
         * @brief Intercepts Steam P2P connection requests to ensure smooth multiplayer handshakes.
         * @param steamID The 64-bit Steam ID of the connecting player.
         * @return 1 to accept and allow connection, 0 for default game handling.
         */
        int HandleP2PRequest(uint64_t steamID);

        /**
         * @brief Called on each game update to monitor network session health.
         */
        void Update(cube::Game* game);

    private:
        NetworkOptimizer() = default;
        ~NetworkOptimizer() = default;

        uint64_t m_tickCounter{0};
    };

} // namespace Features
} // namespace TogetherSpawn
