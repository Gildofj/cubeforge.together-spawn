#include "Config.h"
#include "../utils/Logger.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <windows.h>
#include <shlobj.h>

namespace TogetherSpawn {
namespace Core {

    Config& Config::Instance() {
        static Config instance;
        return instance;
    }

    Config::Config() {
        Load();
    }

    ModSettings& Config::GetSettings() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_settings;
    }

    const ModSettings& Config::GetSettings() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_settings;
    }

    std::string Config::MakeSessionKey(uint64_t hostSteamID, int worldSeed, int characterSlot) const {
        return std::to_string(hostSteamID) + "_" + std::to_string(worldSeed) + "_" + std::to_string(characterSlot);
    }

    std::string Config::GetConfigPath() const {
        char appDataPath[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, appDataPath))) {
            std::filesystem::path dir = std::filesystem::path(appDataPath) / "CubeForge" / "TogetherSpawn";
            std::error_code ec;
            std::filesystem::create_directories(dir, ec);
            return (dir / "together_spawn_config.json").string();
        }
        return "together_spawn_config.json";
    }

    bool Config::HasSpawnedInSession(uint64_t hostSteamID, int worldSeed, int characterSlot) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::string key = MakeSessionKey(hostSteamID, worldSeed, characterSlot);
        return m_spawnedSessions.find(key) != m_spawnedSessions.end();
    }

    void Config::MarkSessionAsSpawned(uint64_t hostSteamID, int worldSeed, int characterSlot) {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            std::string key = MakeSessionKey(hostSteamID, worldSeed, characterSlot);
            m_spawnedSessions.insert(key);
        }
        Save();
    }

    void Config::ResetSessionSpawn(uint64_t hostSteamID, int worldSeed, int characterSlot) {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            std::string key = MakeSessionKey(hostSteamID, worldSeed, characterSlot);
            m_spawnedSessions.erase(key);
        }
        Save();
    }

    void Config::ClearAllSpawnHistory() {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_spawnedSessions.clear();
        }
        Save();
    }

    void Config::Load() {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::string path = GetConfigPath();

        std::ifstream file(path);
        if (!file.is_open()) {
            Utils::Logger::Info("Config file not found, creating default config at: " + path);
            return;
        }

        std::string line;
        bool inSpawnedSessionsArray = false;

        while (std::getline(file, line)) {
            // Trim leading whitespace
            size_t start = line.find_first_not_of(" \t\r\n");
            if (start == std::string::npos) continue;
            std::string trimmed = line.substr(start);

            if (trimmed.find("\"spawned_sessions\":") != std::string::npos) {
                inSpawnedSessionsArray = true;
                continue;
            }

            if (inSpawnedSessionsArray) {
                if (trimmed.find("]") != std::string::npos) {
                    inSpawnedSessionsArray = false;
                    continue;
                }
                size_t firstQuote = trimmed.find("\"");
                size_t secondQuote = trimmed.find("\"", firstQuote + 1);
                if (firstQuote != std::string::npos && secondQuote != std::string::npos) {
                    std::string sessionKey = trimmed.substr(firstQuote + 1, secondQuote - firstQuote - 1);
                    m_spawnedSessions.insert(sessionKey);
                }
                continue;
            }

            // Parse key-values
            if (trimmed.find("\"auto_spawn_near_host\":") != std::string::npos) {
                m_settings.autoSpawnNearHostOnFirstJoin = (trimmed.find("true") != std::string::npos);
            } else if (trimmed.find("\"spawn_radius_blocks\":") != std::string::npos) {
                size_t col = trimmed.find(":");
                if (col != std::string::npos) {
                    try {
                        m_settings.spawnRadiusInBlocks = std::stof(trimmed.substr(col + 1));
                    } catch (...) {}
                }
            } else if (trimmed.find("\"auto_accept_p2p\":") != std::string::npos) {
                m_settings.autoAcceptP2PRequests = (trimmed.find("true") != std::string::npos);
            } else if (trimmed.find("\"invulnerability_seconds\":") != std::string::npos) {
                size_t col = trimmed.find(":");
                if (col != std::string::npos) {
                    try {
                        m_settings.invulnerabilitySecondsAfterSpawn = std::stof(trimmed.substr(col + 1));
                    } catch (...) {}
                }
            } else if (trimmed.find("\"allow_teleport_commands\":") != std::string::npos) {
                m_settings.allowTeleportCommands = (trimmed.find("true") != std::string::npos);
            } else if (trimmed.find("\"teleport_cooldown_seconds\":") != std::string::npos) {
                size_t col = trimmed.find(":");
                if (col != std::string::npos) {
                    try {
                        m_settings.teleportCooldownSeconds = std::stoi(trimmed.substr(col + 1));
                    } catch (...) {}
                }
            }
        }

        Utils::Logger::Info("Loaded TogetherSpawn config from: " + path);
    }

    void Config::Save() {
        std::string path = GetConfigPath();
        std::ofstream file(path);
        if (!file.is_open()) {
            Utils::Logger::Error("Failed to open config file for writing: " + path);
            return;
        }

        file << "{\n";
        file << "  \"auto_spawn_near_host\": " << (m_settings.autoSpawnNearHostOnFirstJoin ? "true" : "false") << ",\n";
        file << "  \"spawn_radius_blocks\": " << m_settings.spawnRadiusInBlocks << ",\n";
        file << "  \"auto_accept_p2p\": " << (m_settings.autoAcceptP2PRequests ? "true" : "false") << ",\n";
        file << "  \"invulnerability_seconds\": " << m_settings.invulnerabilitySecondsAfterSpawn << ",\n";
        file << "  \"allow_teleport_commands\": " << (m_settings.allowTeleportCommands ? "true" : "false") << ",\n";
        file << "  \"teleport_cooldown_seconds\": " << m_settings.teleportCooldownSeconds << ",\n";
        file << "  \"spawned_sessions\": [\n";

        size_t idx = 0;
        for (const auto& key : m_spawnedSessions) {
            file << "    \"" << key << "\"" << (idx + 1 < m_spawnedSessions.size() ? "," : "") << "\n";
            idx++;
        }

        file << "  ]\n";
        file << "}\n";

        Utils::Logger::Debug("Saved TogetherSpawn config to: " + path);
    }

} // namespace Core
} // namespace TogetherSpawn
