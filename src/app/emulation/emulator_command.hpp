// TODO: NEEDS REVIEW
#pragma once

namespace firelight::emulation {

/**
 * Something asked of a running emulator, queued rather than done where it was asked.
 *
 * The queue is drained on the thread that runs frames, so a command never lands in the middle of
 * one — a state serialized halfway through a frame would be a state of nothing.
 */
enum class EmulatorCommandType {
  /** Run a single frame now, whether or not the emulator is paused */
  RunFrame,
  WriteRewindPoint,
  EmitRewindPoints,
  LoadRewindPoint,
  WriteSuspendPoint,
  LoadSuspendPoint,
  UndoLoadSuspendPoint,
  SetPlaybackMultiplier,
  CaptureScreenshot,
  CaptureVideoClip
};

/**
 * One queued request and whatever it needs to carry
 */
struct EmulatorCommand {
  EmulatorCommandType type = EmulatorCommandType::RunFrame;
  int suspendPointIndex = 0;
  int rewindPointIndex = 0;
  float playbackMultiplier = 1.0F;

  // Set when a capture was held back a frame to force a fresh framebuffer readback, for a hardware
  // core that was idle. Prevents re-deferring it forever
  bool deferred = false;
};

} // namespace firelight::emulation
