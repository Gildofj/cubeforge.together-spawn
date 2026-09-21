#include "TogetherSpawnMod.h"
#include "core/Config.h"
#include "core/SessionState.h"
#include "features/spawn/SpawnManager.h"
#include "features/network/NetworkOptimizer.h"
#include "features/commands/CommandManager.h"
#include "features/ui/OverlayUI.h"
#include "utils/ModUtils.h"
#include "utils/Logger.h"

TogetherSpawnMod::TogetherSpawnMod() {
    // Set priority table
    OnChatPriority = HighPriority;           // Intercept slash commands early
    OnP2PRequestPriority = VeryHighPriority; // Fast P2P session handshake approval
    OnGameTickPriority = NormalPriority;
    OnCreatureArmorCalculatedPriority = NormalPriority;
    OnCreatureResistanceCalculatedPriority = NormalPriority;
    OnGetKeyboardStatePriority = NormalPriority;
    OnDrawImGuiPriority = NormalPriority;
}

void TogetherSpawnMod::Initialize() {
    TogetherSpawn::Utils::Logger::Info("Initializing CubeForge TogetherSpawn Mod v1.0.0...");

    // 1. Load configuration and spawn history
    TogetherSpawn::Core::Config::Instance().Load();

    // 2. Apply spawn restriction bypass memory patches
    TogetherSpawn::Features::SpawnManager::Instance().Initialize();

    TogetherSpawn::Utils::Logger::Info("TogetherSpawn initialized successfully.");
}

void TogetherSpawnMod::OnGameTick(cube::Game* game) {
    if (!game) return;

    // 1. Update session state (Host/Client detection, seed, character slot)
    TogetherSpawn::Core::SessionState::Instance().Update(game);

    // 2. Update network optimizer watchdog
    TogetherSpawn::Features::NetworkOptimizer::Instance().Update(game);

    // 3. Update spawn manager (handles first-time spawn teleportation)
    TogetherSpawn::Features::SpawnManager::Instance().Update(game);
}

int TogetherSpawnMod::OnChat(std::wstring* message) {
    return TogetherSpawn::Features::CommandManager::Instance().HandleChat(message);
}

int TogetherSpawnMod::OnP2PRequest(uint64_t steamID) {
    return TogetherSpawn::Features::NetworkOptimizer::Instance().HandleP2PRequest(steamID);
}

void TogetherSpawnMod::OnCreatureArmorCalculated(cube::Creature* creature, float* armor) {
    if (!creature || !armor) return;

    // If local player is under temporary safe-landing invulnerability, grant massive armor
    if (TogetherSpawn::Utils::IsLocalPlayer(creature) &&
        TogetherSpawn::Core::SessionState::Instance().IsInvulnerable()) {
        *armor += 999999.0f;
    }
}

void TogetherSpawnMod::OnCreatureResistanceCalculated(cube::Creature* creature, float* resistance) {
    if (!creature || !resistance) return;

    // If local player is under temporary safe-landing invulnerability, grant massive magic resistance
    if (TogetherSpawn::Utils::IsLocalPlayer(creature) &&
        TogetherSpawn::Core::SessionState::Instance().IsInvulnerable()) {
        *resistance += 999999.0f;
    }
}

void TogetherSpawnMod::OnGetKeyboardState(BYTE* diKeys) {
    TogetherSpawn::Features::OverlayUI::Instance().HandleKeyboardState(diKeys);
}

void TogetherSpawnMod::OnDrawImGui() {
    TogetherSpawn::Features::OverlayUI::Instance().DrawImGui();
}
