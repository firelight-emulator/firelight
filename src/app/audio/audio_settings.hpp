// TODO: NEEDS REVIEW
#pragma once

namespace firelight::audio {

// The setting keys AudioManager reads, in their own header so that code which
// only needs to *name* one — the mute hotkey — doesn't drag Qt Multimedia in
// with it. Both are declared in data/settings_catalog.json, and a test pins
// each to its declaration so they can't quietly drift apart

// Which output to play through; "" means the system default
inline constexpr auto OUTPUT_DEVICE_KEY = "audio-output-device";

// The user's mute. A setting rather than emulator state, so it survives
// switching games and works with none running
inline constexpr auto MUTED_KEY = "audio-muted";

// TODO
/** How much sound is buffered ahead, in milliseconds */
inline constexpr auto LATENCY_KEY = "audio-latency";

// TODO
/** What to buffer when the setting is missing or unreadable. RetroArch ships the same figure */
inline constexpr int DEFAULT_LATENCY_MS = 64;

// TODO
/**
 * The least that can be asked for. Half the buffer is what paces audio-synced emulation and what
 * rate control steers toward, and a device reports what it has consumed only in whole periods —
 * 10.67 ms on CoreAudio — so a half-buffer smaller than a couple of those has nothing to work with
 */
inline constexpr int MIN_LATENCY_MS = 32;
inline constexpr int MAX_LATENCY_MS = 256;

// TODO
/**
 * How full the sink is filled before playback starts, as a fraction of its capacity.
 *
 * Above the level rate control steers to, because the first seconds of a game are when audio arrives
 * least evenly — a recompiler is still translating and a hardware renderer is still building
 * pipelines — and a buffer started on target has only half of itself to absorb that. Rate control
 * brings the extra back down over the seconds that follow
 */
inline constexpr double PRIMING_FILL_FRACTION = 0.75;

// TODO
/**
 * Bytes of interleaved stereo 16-bit audio that hold the given milliseconds at the given rate
 */
[[nodiscard]] inline int bufferBytesForLatency(const int latencyMs, const int sampleRate) {
  constexpr int BYTES_PER_FRAME = 4;
  return latencyMs * sampleRate / 1000 * BYTES_PER_FRAME;
}

// Playback loudness, 0-100. Separate from mute so that unmuting returns you to
// the volume you had rather than to full
inline constexpr auto VOLUME_KEY = "volume";

// TODO
// Loudness of the interface's own sounds, 0-100. Deliberately its own key so
// muting or quietening a game leaves menu feedback alone
inline constexpr auto UI_SOUND_VOLUME_KEY = "ui-sound-volume";

} // namespace firelight::audio
