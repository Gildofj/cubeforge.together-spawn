#include "ModUtils.h"
#include "Logger.h"
#include "cube/constants.h"
#include <sstream>
#include <algorithm>
#include <windows.h>

namespace TogetherSpawn {
namespace Utils {

    using cube::DOTS_PER_BLOCK;

    void PrintChat(const std::wstring& message, const FloatRGBA& color) {
        cube::Game* game = cube::GetGame();
        if (game && game->gui.chat_widget) {
            FloatRGBA clr = color;
            game->PrintMessage(message.c_str(), &clr);
        }
    }

    void PrintChat(const std::string& utf8Message, const FloatRGBA& color) {
        if (utf8Message.empty()) return;
        std::wstring wmsg = Utf8ToWide(utf8Message);
        PrintChat(wmsg, color);
    }

    std::vector<std::wstring> Split(const std::wstring& str, wchar_t delimiter) {
        std::vector<std::wstring> tokens;
        std::wstringstream ss(str);
        std::wstring item;
        while (std::getline(ss, item, delimiter)) {
            if (!item.empty()) {
                tokens.push_back(item);
            }
        }
        return tokens;
    }

    std::vector<std::string> Split(const std::string& str, char delimiter) {
        std::vector<std::string> tokens;
        std::stringstream ss(str);
        std::string item;
        while (std::getline(ss, item, delimiter)) {
            if (!item.empty()) {
                tokens.push_back(item);
            }
        }
        return tokens;
    }

    std::wstring Utf8ToWide(const std::string& str) {
        if (str.empty()) return std::wstring();
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), NULL, 0);
        std::wstring wstr(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), &wstr[0], size_needed);
        return wstr;
    }

    std::string WideToUtf8(const std::wstring& wstr) {
        if (wstr.empty()) return std::string();
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.size()), NULL, 0, NULL, NULL);
        std::string str(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.size()), &str[0], size_needed, NULL, NULL);
        return str;
    }

    cube::Creature* GetLocalPlayer() {
        cube::Game* game = cube::GetGame();
        if (!game || !game->world) return nullptr;
        return game->world->local_creature;
    }

    bool IsLocalPlayer(cube::Creature* creature) {
        if (!creature) return false;
        return creature == GetLocalPlayer();
    }

    std::vector<cube::Creature*> GetOnlinePlayers(bool includeLocal) {
        std::vector<cube::Creature*> players;
        cube::Game* game = cube::GetGame();
        if (!game || !game->world) return players;

        cube::Creature* local = game->world->local_creature;

        if (includeLocal && local) {
            players.push_back(local);
        }

        for (cube::Creature* c : game->world->creatures) {
            if (!c) continue;
            if (c->entity_data.hostility_type == cube::Creature::EntityBehaviour::Player) {
                if (c != local) {
                    players.push_back(c);
                }
            }
        }
        return players;
    }

    cube::Creature* FindPlayerBySteamID(uint64_t steamID) {
        if (steamID == 0) return nullptr;
        cube::Game* game = cube::GetGame();
        if (!game || !game->world) return nullptr;

        for (cube::Creature* c : game->world->creatures) {
            if (c && static_cast<uint64_t>(c->entity_data.steam_id) == steamID) {
                return c;
            }
        }

        if (game->world->local_creature && static_cast<uint64_t>(game->world->local_creature->entity_data.steam_id) == steamID) {
            return game->world->local_creature;
        }

        return nullptr;
    }

    cube::Creature* FindPlayerByName(const std::string& name) {
        if (name.empty()) return nullptr;
        std::string lowerTarget = name;
        std::transform(lowerTarget.begin(), lowerTarget.end(), lowerTarget.begin(), ::tolower);

        auto players = GetOnlinePlayers(true);
        for (cube::Creature* c : players) {
            if (!c) continue;
            std::string cName(c->entity_data.name);
            std::string lowerCName = cName;
            std::transform(lowerCName.begin(), lowerCName.end(), lowerCName.begin(), ::tolower);

            if (lowerCName.find(lowerTarget) != std::string::npos) {
                return c;
            }
        }
        return nullptr;
    }

    bool TeleportCreature(cube::Creature* creature, const LongVector3& targetPos) {
        if (!creature) return false;

        creature->entity_data.position = targetPos;
        creature->entity_data.velocity = FloatVector3(0.0f, 0.0f, 0.0f);
        creature->entity_data.retreat = FloatVector3(0.0f, 0.0f, 0.0f);

        i64 blockX = targetPos.x / DOTS_PER_BLOCK;
        i64 blockY = targetPos.y / DOTS_PER_BLOCK;
        creature->entity_data.current_region.x = static_cast<int>(blockX / 1024);
        creature->entity_data.current_region.y = static_cast<int>(blockY / 1024);
        creature->entity_data.some_zone_position.x = static_cast<int>(blockX / 64);
        creature->entity_data.some_zone_position.y = static_cast<int>(blockY / 64);

        Logger::Info("Teleported creature " + std::string(creature->entity_data.name) +
                     " to (" + std::to_string(targetPos.x) + ", " +
                     std::to_string(targetPos.y) + ", " +
                     std::to_string(targetPos.z) + ")");
        return true;
    }

} // namespace Utils
} // namespace TogetherSpawn
