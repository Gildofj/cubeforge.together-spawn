#include "NetworkOptimizer.h"
#include "../../core/Config.h"
#include "../../utils/Logger.h"
#include "../../utils/ModUtils.h"

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

    void NetworkOptimizer::Update(cube::Game* game) {
        if (!game) return;

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
