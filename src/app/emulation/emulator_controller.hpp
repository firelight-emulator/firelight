#pragma once

namespace firelight::emulation {

class IEmulatorController {
public:
  virtual ~IEmulatorController() = default;

  [[nodiscard]] virtual float playbackMultiplier() const = 0;
  virtual void setPlaybackMultiplier(float multiplier) = 0;

  [[nodiscard]] virtual bool paused() const = 0;
  virtual void setPaused(bool paused) = 0;

  virtual void advanceOneFrame() = 0;

  virtual void writeSuspendPoint(int index) = 0;
  virtual void loadSuspendPoint(int index) = 0;

  virtual void captureScreenshot() = 0;
  virtual void captureVideoClip() = 0;
};

} // namespace firelight::emulation
