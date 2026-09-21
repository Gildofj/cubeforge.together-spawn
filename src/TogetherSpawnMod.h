#pragma once

#include "cwsdk.h"
#include <string>

class TogetherSpawnMod : public GenericMod {
public:
    TogetherSpawnMod();
    virtual ~TogetherSpawnMod() = default;

    virtual void Initialize() override;
    virtual void OnGameTick(cube::Game* game) override;
    virtual int OnChat(std::wstring* message) override;
    virtual int OnP2PRequest(uint64_t steamID) override;
    virtual void OnCreatureArmorCalculated(cube::Creature* creature, float* armor) override;
    virtual void OnCreatureResistanceCalculated(cube::Creature* creature, float* resistance) override;
    virtual void OnGetKeyboardState(BYTE* diKeys) override;
    virtual void OnDrawImGui() override;
};
