#pragma once

#include "cwsdk.h"
#include <string>
#include <vector>

namespace TogetherSpawn {
namespace Features {

    class CommandManager {
    public:
        static CommandManager& Instance();

        int HandleChat(std::wstring* message);

    private:
        CommandManager() = default;
        ~CommandManager() = default;

        bool ProcessCommand(cube::Game* game, const std::vector<std::wstring>& args);
        void HandleTogetherCommand(cube::Game* game, const std::vector<std::wstring>& args);
        void HandleTpHostCommand(cube::Game* game);
        void HandleTpPlayerCommand(cube::Game* game, const std::wstring& targetName);
        void HandleSetTeamSpawnCommand(cube::Game* game);
        void PrintHelp();
    };

} // namespace Features
} // namespace TogetherSpawn
