#pragma once

namespace firelight::emulation {

class Emulator {
public:
  void start();
  void stop();

  void runOneFrame();

  void pause();
  void resume();

  void setRewindEnabled(bool enabled);
  void setMuted(bool muted);
  void setEmulationSpeed(float multiplier);
  void speedUp();
  void slowDown();
};

} // namespace firelight::emulation
