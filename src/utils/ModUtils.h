#pragma once

#include "cwsdk.h"
#include <string>
#include <vector>
#include <optional>

namespace TogetherSpawn {
namespace Utils {

    namespace Colors {
        inline const FloatRGBA White(1.0f, 1.0f, 1.0f, 1.0f);
        inline const FloatRGBA Green(0.2f, 1.0f, 0.2f, 1.0f);
        inline const FloatRGBA Emerald(0.0f, 0.9f, 0.4f, 1.0f);
        inline const FloatRGBA Red(1.0f, 0.3f, 0.3f, 1.0f);
        inline const FloatRGBA Cyan(0.3f, 0.85f, 1.0f, 1.0f);
        inline const FloatRGBA Gold(1.0f, 0.84f, 0.0f, 1.0f);
        inline const FloatRGBA Yellow(1.0f, 0.9f, 0.2f, 1.0f);
        inline const FloatRGBA Orange(1.0f, 0.6f, 0.1f, 1.0f);
        inline const FloatRGBA Gray(0.7f, 0.7f, 0.7f, 1.0f);
    }

    /**
     * @brief Sends a formatted message to in-game chat.
     */
    void PrintChat(const std::wstring& message, const FloatRGBA& color = Colors::White);
    void PrintChat(const std::string& utf8Message, const FloatRGBA& color = Colors::White);

    /**
     * @brief Splits wide strings by a delimiter.
     */
    std::vector<std::wstring> Split(const std::wstring& str, wchar_t delimiter = L' ');
    std::vector<std::string> Split(const std::string& str, char delimiter = ' ');

    /**
     * @brief Converts between UTF-8 and Wide String.
     */
    std::wstring Utf8ToWide(const std::string& str);
    std::string WideToUtf8(const std::wstring& wstr);

    /**
     * @brief Gets the local player creature safely.
     */
    cube::Creature* GetLocalPlayer();

    /**
     * @brief Checks if a creature is the local player.
     */
    bool IsLocalPlayer(cube::Creature* creature);

    /**
     * @brief Retrieves all player creatures in the world (excluding local player if specified).
     */
    std::vector<cube::Creature*> GetOnlinePlayers(bool includeLocal = false);

    /**
     * @brief Finds a player creature by Steam ID.
     */
    cube::Creature* FindPlayerBySteamID(uint64_t steamID);

    /**
     * @brief Finds a player creature by character/account name (case-insensitive substring match).
     */
    cube::Creature* FindPlayerByName(const std::string& name);

    /**
     * @brief Safely teleports a creature to new world coordinates.
     */
    bool TeleportCreature(cube::Creature* creature, const LongVector3& targetPos);

} // namespace Utils
} // namespace TogetherSpawn
