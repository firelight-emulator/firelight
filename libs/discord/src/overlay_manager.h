#pragma once

#include "types.h"

namespace discord {

class OverlayManager final {
public:
    ~OverlayManager() = default;

    void IsEnabled(bool* enabled);
    void IsLocked(bool* locked);
    void SetLocked(bool locked, std::function<void(Result)> callback);
    void OpenActivityInvite(ActivityActionType type, std::function<void(Result)> callback);
    void OpenGuildInvite(char const* code, std::function<void(Result)> callback);
    void OpenVoiceSettings(std::function<void(Result)> callback);
    Result InitDrawingDxgi(IDXGISwapChain* swapchain, bool useMessageForwarding);
    void OnPresent();
    void ForwardMessage(MSG* message);
    void KeyEvent(bool down, char const* keyCode, KeyVariant variant);
    void CharEvent(char const* character);
    void MouseButtonEvent(uint8_t down,
                          int32_t clickCount,
                          MouseButton which,
                          int32_t x,
                          int32_t y);
    void MouseMotionEvent(int32_t x, int32_t y);
    void ImeCommitText(char const* text);
    void ImeSetComposition(char const* text,
                           ImeUnderline* underlines,
                           uint32_t underlinesLength,
                           int32_t from,
                           int32_t to);
    void ImeCancelComposition();
    void SetImeCompositionRangeCallback(
      std::function<void(int32_t, int32_t, Rect*, uint32_t)>
        onImeCompositionRangeChanged);
    void SetImeSelectionBoundsCallback(
      std::function<void(Rect, Rect, bool)> onImeSelectionBoundsChanged);
    bool IsPointInsideClickZone(int32_t x, int32_t y);

    Event<bool> OnToggle;

private:
    friend class Core;

    OverlayManager() = default;
    OverlayManager(OverlayManager const& rhs) = delete;
    OverlayManager& operator=(OverlayManager const& rhs) = delete;
    OverlayManager(OverlayManager&& rhs) = delete;
    OverlayManager& operator=(OverlayManager&& rhs) = delete;

    IDiscordOverlayManager* internal_;
    static IDiscordOverlayEvents events_;
};

} // namespace discord
