#include "CommandManager.h"
#include "../spawn/SpawnManager.h"
#include "../../core/Config.h"
#include "../../core/SessionState.h"
#include "../../utils/ModUtils.h"
#include "../../utils/MathUtils.h"
#include "../../utils/Logger.h"
#include <iomanip>
#include <sstream>

namespace TogetherSpawn {
namespace Features {

    CommandManager& CommandManager::Instance() {
        static CommandManager instance;
        return instance;
    }

    int CommandManager::HandleChat(std::wstring* message) {
        if (!message || message->empty()) return 0;
        if ((*message)[0] != L'/') return 0;

        cube::Game* game = cube::GetGame();
        if (!game) return 0;

        std::vector<std::wstring> args = Utils::Split(*message, L' ');
        if (args.empty()) return 0;

        bool handled = ProcessCommand(game, args);
        return handled ? 1 : 0;
    }

    bool CommandManager::ProcessCommand(cube::Game* game, const std::vector<std::wstring>& args) {
        const std::wstring& cmd = args[0];

        if (cmd == L"/together" || cmd == L"/togetherspawn") {
            HandleTogetherCommand(game, args);
            return true;
        }

        if (cmd == L"/tphost" || cmd == L"/spawn") {
            HandleTpHostCommand(game);
            return true;
        }

        if (cmd == L"/tp") {
            if (args.size() > 1) {
                HandleTpPlayerCommand(game, args[1]);
            } else {
                Utils::PrintChat(L"[TogetherSpawn] Uso: /tp <nome_do_jogador>", Utils::Colors::Orange);
            }
            return true;
        }

        if (cmd == L"/setteamspawn") {
            HandleSetTeamSpawnCommand(game);
            return true;
        }

        return false;
    }

    void CommandManager::HandleTogetherCommand(cube::Game* game, const std::vector<std::wstring>& args) {
        if (args.size() == 1 || args[1] == L"help") {
            PrintHelp();
            return;
        }

        const std::wstring& subCmd = args[1];
        auto& session = Core::SessionState::Instance();
        auto& config = Core::Config::Instance();

        if (subCmd == L"status") {
            Utils::PrintChat(L"--- [TogetherSpawn: Status da Sessao] ---", Utils::Colors::Cyan);

            std::wstring roleStr = L"SinglePlayer";
            if (session.GetRole() == Core::SessionRole::Host) roleStr = L"Host (Servidor)";
            else if (session.GetRole() == Core::SessionRole::Client) roleStr = L"Client (Conectado)";

            Utils::PrintChat(L" Modo: " + roleStr, Utils::Colors::White);
            Utils::PrintChat(L" Seed do Mundo: " + std::to_wstring(session.GetWorldSeed()), Utils::Colors::White);

            if (session.GetRole() == Core::SessionRole::Client) {
                Utils::PrintChat(L" Host SteamID: " + std::to_wstring(session.GetHostSteamID()), Utils::Colors::White);
                Utils::PrintChat(L" Spawn inicial concluido: " + std::wstring(session.HasCurrentSessionSpawned() ? L"Sim" : L"Nao"), Utils::Colors::Gold);
            }

            auto players = Utils::GetOnlinePlayers(true);
            Utils::PrintChat(L" Jogadores no mundo: " + std::to_wstring(players.size()), Utils::Colors::Emerald);

            cube::Creature* local = game->world ? game->world->local_creature : nullptr;
            for (cube::Creature* p : players) {
                if (!p) continue;
                std::string pName(p->entity_data.name);
                double dist = (local && local != p) ? Utils::MathUtils::DistanceInBlocks(local->entity_data.position, p->entity_data.position) : 0.0;
                std::wstring line = L"  * " + Utils::Utf8ToWide(pName);
                if (p == local) line += L" (Voce)";
                else line += L" [Distancia: " + std::to_wstring(static_cast<int>(dist)) + L" blocos]";
                Utils::PrintChat(line, Utils::Colors::White);
            }
            return;
        }

        if (subCmd == L"sync" || subCmd == L"spawn") {
            if (session.GetRole() == Core::SessionRole::Client) {
                Utils::PrintChat(L"[TogetherSpawn] Re-executando sincronizacao de spawn com o Host...", Utils::Colors::Cyan);
                if (!SpawnManager::Instance().ExecuteSpawnNearHost(game, true)) {
                    Utils::PrintChat(L"[TogetherSpawn] Host ainda nao encontrado ou terreno carregando.", Utils::Colors::Orange);
                }
            } else {
                Utils::PrintChat(L"[TogetherSpawn] Voce e o Host da sessao.", Utils::Colors::Gold);
            }
            return;
        }

        if (subCmd == L"reset") {
            config.ResetSessionSpawn(session.GetHostSteamID(), session.GetWorldSeed(), session.GetCharacterSlot());
            session.SetCurrentSessionSpawned(false);
            Utils::PrintChat(L"[TogetherSpawn] Historico de spawn desta sessao resetado. O auto-spawn sera acionado na proxima verificacao.", Utils::Colors::Green);
            return;
        }

        if (subCmd == L"radius" && args.size() > 2) {
            try {
                float rad = std::stof(args[2]);
                if (rad >= 1.0f && rad <= 50.0f) {
                    config.GetSettings().spawnRadiusInBlocks = rad;
                    config.Save();
                    Utils::PrintChat(L"[TogetherSpawn] Raio de spawn ajustado para " + std::to_wstring((int)rad) + L" blocos.", Utils::Colors::Green);
                } else {
                    Utils::PrintChat(L"[TogetherSpawn] O raio deve ser entre 1 e 50 blocos.", Utils::Colors::Orange);
                }
            } catch (...) {
                Utils::PrintChat(L"[TogetherSpawn] Valor invalido.", Utils::Colors::Red);
            }
            return;
        }

        PrintHelp();
    }

    void CommandManager::HandleTpHostCommand(cube::Game* game) {
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

        cube::Creature* host = SpawnManager::Instance().FindHostCreature(game);
        if (!host) {
            Utils::PrintChat(L"[TogetherSpawn] Host nao encontrado no mundo.", Utils::Colors::Red);
            return;
        }

        if (SpawnManager::Instance().TeleportToPlayer(game, host)) {
            session.RecordTeleportCommandUsed();
            Utils::PrintChat(L"[TogetherSpawn] Teleportado com sucesso para o Host!", Utils::Colors::Emerald);
        } else {
            Utils::PrintChat(L"[TogetherSpawn] Falha ao calcular posicao segura para teleporte.", Utils::Colors::Red);
        }
    }

    void CommandManager::HandleTpPlayerCommand(cube::Game* game, const std::wstring& targetName) {
        auto& session = Core::SessionState::Instance();

        if (!session.CanUseTeleportCommand()) {
            int rem = session.GetRemainingCooldownSeconds();
            Utils::PrintChat(L"[TogetherSpawn] Comando em cooldown. Aguarde " + std::to_wstring(rem) + L"s.", Utils::Colors::Orange);
            return;
        }

        std::string nameUtf8 = Utils::WideToUtf8(targetName);
        cube::Creature* target = Utils::FindPlayerByName(nameUtf8);

        if (!target) {
            Utils::PrintChat(L"[TogetherSpawn] Jogador '" + targetName + L"' nao encontrado.", Utils::Colors::Red);
            return;
        }

        if (Utils::IsLocalPlayer(target)) {
            Utils::PrintChat(L"[TogetherSpawn] Voce ja esta na sua propria posicao.", Utils::Colors::Orange);
            return;
        }

        if (SpawnManager::Instance().TeleportToPlayer(game, target)) {
            session.RecordTeleportCommandUsed();
        } else {
            Utils::PrintChat(L"[TogetherSpawn] Falha ao teletransportar para o jogador.", Utils::Colors::Red);
        }
    }

    void CommandManager::HandleSetTeamSpawnCommand(cube::Game* game) {
        if (!game || !game->world || !game->world->local_creature) return;

        auto& session = Core::SessionState::Instance();
        if (session.GetRole() != Core::SessionRole::Host) {
            Utils::PrintChat(L"[TogetherSpawn] Apenas o Host pode definir o spawn do time.", Utils::Colors::Red);
            return;
        }

        auto& settings = Core::Config::Instance().GetSettings();
        settings.customTeamSpawnEnabled = true;
        settings.customTeamSpawnPos = game->world->local_creature->entity_data.position;
        Core::Config::Instance().Save();

        Utils::PrintChat(L"[TogetherSpawn] Ponto de Spawn do Time definido na sua posicao atual!", Utils::Colors::Emerald);
    }

    void CommandManager::PrintHelp() {
        Utils::PrintChat(L"--- [TogetherSpawn: Comandos] ---", Utils::Colors::Cyan);
        Utils::PrintChat(L" /together status        - Exibe informacoes e lista de jogadores da sessao", Utils::Colors::White);
        Utils::PrintChat(L" /together sync          - Sincroniza e teleporta proximo ao Host", Utils::Colors::White);
        Utils::PrintChat(L" /together reset         - Reseta o historico de primeiro spawn da sessao", Utils::Colors::White);
        Utils::PrintChat(L" /together radius <N>    - Configura o raio de spawn (padrao: 4 blocos)", Utils::Colors::White);
        Utils::PrintChat(L" /tphost (ou /spawn)     - Teleporta para o Host da sessao", Utils::Colors::Gold);
        Utils::PrintChat(L" /tp <nome>              - Teleporta para um amigo conectado", Utils::Colors::Gold);
        Utils::PrintChat(L" /setteamspawn           - [Host] Define ponto de spawn customizado", Utils::Colors::Gold);
        Utils::PrintChat(L" [F6]                    - Abre/fecha o menu de configuracoes ImGui", Utils::Colors::Yellow);
    }

} // namespace Features
} // namespace TogetherSpawn
