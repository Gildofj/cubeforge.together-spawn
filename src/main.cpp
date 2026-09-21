#include "cwsdk.h"
#include "TogetherSpawnMod.h"

/**
 * @file main.cpp
 * @brief Dynamic-link library (DLL) entry point and mod factory for Cube World.
 */

EXPORT GenericMod* MakeMod() {
    return new TogetherSpawnMod();
}
