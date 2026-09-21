#pragma once

#include "cwsdk.h"
#include <cstdint>

namespace TogetherSpawn {
namespace Features {

    class NetworkOptimizer {
    public:
        static NetworkOptimizer& Instance();

        int HandleP2PRequest(uint64_t steamID);
        void Update(cube::Game* game);

    private:
        NetworkOptimizer() = default;
        ~NetworkOptimizer() = default;

        uint64_t m_tickCounter{0};
    };

} // namespace Features
} // namespace TogetherSpawn
