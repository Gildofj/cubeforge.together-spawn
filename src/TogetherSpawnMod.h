#pragma once

#include "cwsdk.h"
#include <string>

/**
 * @brief TogetherSpawnMod - Specialized Steam Multiplayer Spawning & Connection Mod for Cube World.
 */
class TogetherSpawnMod : public GenericMod {
public:
    TogetherSpawnMod();
    virtual ~TogetherSpawnMod() = default;

    // -------------------------------------------------------------------------
    // Mod Lifecycle
    // -------------------------------------------------------------------------
    virtual void Initialize() override;

    // -------------------------------------------------------------------------
    // Frame & Logic Updates
    // -------------------------------------------------------------------------
    virtual void OnGameTick(cube::Game* game) override;

    // -------------------------------------------------------------------------
    // Chat & Commands
    // -------------------------------------------------------------------------
    virtual int OnChat(std::wstring* message) override;

    // -------------------------------------------------------------------------
    // Steam P2P Networking
    // -------------------------------------------------------------------------
    virtual int OnP2PRequest(uint64_t steamID) override;

    // -------------------------------------------------------------------------
    // Combat / Safe Landing Protection
    // -------------------------------------------------------------------------
    virtual void OnCreatureArmorCalculated(cube::Creature* creature, float* armor) override;
    virtual void OnCreatureResistanceCalculated(cube::Creature* creature, float* resistance) override;

    // -------------------------------------------------------------------------
    // Input & Hotkeys
    // -------------------------------------------------------------------------
    virtual void OnGetKeyboardState(BYTE* diKeys) override;

    // -------------------------------------------------------------------------
    // GUI / ImGui Overlay
    // -------------------------------------------------------------------------
    virtual void OnDrawImGui() override;
};
