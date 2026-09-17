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
  CaptureVideoClip,
  /** Reboot the game */
  ResetGame,
  /** Tell the core which device sits on a port */
  SetControllerDevice,
  /** Put another disc of a multi-disc game in */
  SwapDisc,
  /** Rebuild the active cheat set from the repository */
  ApplyCheats
};

/**
 * One queued request and whatever it needs to carry
 */
struct EmulatorCommand {
  EmulatorCommandType type = EmulatorCommandType::RunFrame;
  int suspendPointIndex = 0;
  int rewindPointIndex = 0;
  float playbackMultiplier = 1.0F;
  unsigned port = 0;
  unsigned coreDeviceId = 0;
  unsigned discIndex = 0;

  // TODO
  /** The input class for the port, or -1 to leave it as it is */
  int deviceClass = -1;

  // Set when a capture was held back a frame to force a fresh framebuffer readback, for a hardware
  // core that was idle. Prevents re-deferring it forever
  bool deferred = false;
};

} // namespace firelight::emulation
