#pragma once

#include "cwsdk.h"

namespace TogetherSpawn {
namespace Features {

    class OverlayUI {
    public:
        static OverlayUI& Instance();

        /**
         * @brief Intercepts DirectInput keyboard state to detect F6 hotkey toggle.
         */
        void HandleKeyboardState(BYTE* diKeys);

        /**
         * @brief Called in the DirectX 11 / ImGui rendering pass.
         */
        void DrawImGui();

        void ToggleVisibility();
        bool IsVisible() const { return m_visible; }

    private:
        OverlayUI() = default;
        ~OverlayUI() = default;

        bool m_visible{false};
        bool m_f6WasPressed{false};
    };

} // namespace Features
} // namespace TogetherSpawn
