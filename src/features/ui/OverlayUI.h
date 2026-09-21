#pragma once

#include "cwsdk.h"

namespace TogetherSpawn {
namespace Features {

    class OverlayUI {
    public:
        static OverlayUI& Instance();

        void HandleKeyboardState(BYTE* diKeys);
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
