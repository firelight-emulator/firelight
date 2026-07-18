#pragma once

namespace firelight::emulation {

// TODO
// What a shortcut needs from the running emulator
//
// Implemented by EmulatorItem, which genuinely owns this state rather than
// forwarding it: the playback multiplier and paused flag are read by the frame
// pacing thread and bound in QML, so writing them straight to the renderer would
// leave those stale. The rest are one-line forwards onto the renderer's command
// queue, and are here so an action has one thing to talk to
//
// Abstract so ShortcutActions can be tested without a QML item — the same reason
// ICore exists
//
// GUI thread only. ShortcutActions holds one of these, or nullptr while no game
// is running (which is why every action guards on it: screenshot, mute and
// fullscreen are ScopeAlways and can fire in the library)
class IEmulatorController {
public:
  virtual ~IEmulatorController() = default;

  // Named as EmulatorItem already names them, which is also the QML contract
  [[nodiscard]] virtual float playbackMultiplier() const = 0;
  virtual void setPlaybackMultiplier(float multiplier) = 0;

  [[nodiscard]] virtual bool paused() const = 0;
  virtual void setPaused(bool paused) = 0;

  // Runs exactly one frame, leaving the emulator paused afterwards
  virtual void advanceOneFrame() = 0;

  virtual void writeSuspendPoint(int index) = 0;
  virtual void loadSuspendPoint(int index) = 0;

  virtual void captureScreenshot() = 0;
  virtual void captureVideoClip() = 0;
};

} // namespace firelight::emulation
