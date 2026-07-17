#pragma once

namespace firelight::audio {

// The setting keys AudioManager reads, in their own header so that code which
// only needs to *name* one — the mute hotkey — doesn't drag Qt Multimedia in
// with it. Both are declared in data/settings_catalog.json, and a test pins
// each to its declaration so they can't quietly drift apart.

// Which output to play through; "" means the system default.
inline constexpr auto OUTPUT_DEVICE_KEY = "audio-output-device";

// The user's mute. A setting rather than emulator state, so it survives
// switching games and works with none running.
inline constexpr auto MUTED_KEY = "audio-muted";

// Playback loudness, 0-100. Separate from mute so that unmuting returns you to
// the volume you had rather than to full.
inline constexpr auto VOLUME_KEY = "volume";

} // namespace firelight::audio
