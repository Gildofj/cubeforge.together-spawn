#include "cwsdk.h"
#include "TogetherSpawnMod.h"

EXPORT GenericMod* MakeMod() {
    return new TogetherSpawnMod();
}
