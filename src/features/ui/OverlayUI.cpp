#include "OverlayUI.h"
#include "../../core/Config.h"
#include "../../core/SessionState.h"
#include "../../utils/ModUtils.h"
#include "../../utils/MathUtils.h"

namespace TogetherSpawn {
namespace Features {

    OverlayUI& OverlayUI::Instance() {
        static OverlayUI instance;
        return instance;
    }

    void OverlayUI::ToggleVisibility() {
        m_visible = !m_visible;
        if (m_visible) {
            Utils::PrintChat(L"[TogetherSpawn] Menu de Configuracoes aberto [F6].", Utils::Colors::Cyan);
        } else {
            Utils::PrintChat(L"[TogetherSpawn] Menu de Configuracoes fechado.", Utils::Colors::Orange);
        }
    }

    void OverlayUI::HandleKeyboardState(BYTE* diKeys) {
        if (!diKeys) return;

        bool f6Pressed = (diKeys[DIK_F6] & 0x80) != 0;
        if (f6Pressed && !m_f6WasPressed) {
            ToggleVisibility();
        }
        m_f6WasPressed = f6Pressed;
    }

    void OverlayUI::DrawImGui() {
        if (!m_visible) return;
    }

} // namespace Features
} // namespace TogetherSpawn
