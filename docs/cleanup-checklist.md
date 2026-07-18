# Cleanup checklist

Prose pass over the branch: every file below still carries generated comments, names or
structure. Tick one once it reads as yours.

Formatting is handled mechanically per batch *before* you start that batch, so you are never
editing text that is about to be reflowed. See the plan for the per-batch loop.

**Formatting status: done for every batch below.** clang-format has run over all C++ (libs,
`include/`, `src/`, `tests/`) and qmlformat over all 215 QML files, verified by build, the full
test suite and an app launch. Nothing here is waiting on a reflow — every remaining tick is prose.

Scope is the substantive diff against `main` as of `6cc7daa` (the commit before line-ending
normalization), so files that only changed line endings are excluded. Vendored trees
(`thirdparty/`, `libs/rcheevos`, `libs/discord`, `include/{rcheevos,libretro,discord}`) are excluded.

`L` = total lines, `C` = comment lines.

## Summary

| Batch | Files | Lines | Comments | Comment % |
|---|---|---|---|---|
| 01 libs - small (voice-setting) | 43 | 4032 | 293 | 7% |
| 02 libs - input | 45 | 6921 | 494 | 7% |
| 03 libs - library | 36 | 4091 | 321 | 8% |
| 04 libs - netplay | 30 | 3508 | 151 | 4% |
| 05 libs - achievements | 27 | 4843 | 1321 | 27% |
| 06 libs - media | 24 | 2543 | 143 | 6% |
| 07 libs - saves | 21 | 1447 | 58 | 4% |
| 08 libs - metadata | 17 | 1258 | 133 | 11% |
| 09 libs - other | 8 | 737 | 19 | 3% |
| 10 include (ours) | 20 | 950 | 165 | 17% |
| 11 src/app - library | 10 | 1439 | 73 | 5% |
| 12 src/app - input | 15 | 1608 | 79 | 5% |
| 13 src/app - netplay | 17 | 1976 | 74 | 4% |
| 14 src/app - libretro | 14 | 3117 | 259 | 8% |
| 15 src/app - emulation | 14 | 2033 | 287 | 14% |
| 16 src/app - saves | 7 | 414 | 4 | 1% |
| 17 src/app - achievements | 6 | 382 | 0 | 0% |
| 18 src/app - other | 26 | 4743 | 523 | 11% |
| 19 src/gui | 48 | 5149 | 423 | 8% |
| 20 src/cli | 19 | 1061 | 89 | 8% |
| 21 src - root | 1 | 998 | 157 | 16% |
| 22 qml - components/v2 | 61 | 8624 | 603 | 7% |
| 23 qml - components/settings | 16 | 1449 | 97 | 7% |
| 24 qml - components | 37 | 7705 | 1512 | 20% |
| 25 qml - common | 17 | 1677 | 391 | 23% |
| 26 qml - other | 61 | 12021 | 1826 | 15% |
| 27 tests | 48 | 13071 | 2149 | 16% |
| 28 misc | 1 | 174 | 9 | 5% |
| **total** | **689** | **97971** | **11653** | **12%** |


## 01 libs - small (voice-setting)

- [ ] `libs/firelight/activity/include/firelight/activity/activity_log.hpp` — 25 L / 3 C
- [ ] `libs/firelight/activity/include/firelight/activity/play_session.hpp` — 14 L / 0 C
- [ ] `libs/firelight/activity/src/firelight/activity/sqlite_activity_log.hpp` — 25 L / 0 C
- [ ] `libs/firelight/activity/src/sqlite_activity_log.cpp` — 189 L / 4 C
- [ ] `libs/firelight/audio/include/firelight/audio/audio_rate_controller.hpp` — 25 L / 10 C
- [ ] `libs/firelight/audio/include/firelight/audio/audio_resampler.hpp` — 82 L / 39 C
- [ ] `libs/firelight/audio/src/audio_rate_controller.cpp` — 84 L / 3 C
- [ ] `libs/firelight/audio/src/audio_resampler.cpp` — 109 L / 1 C
- [ ] `libs/firelight/audio/tests/audio_rate_controller_test.cpp` — 44 L / 1 C
- [ ] `libs/firelight/audio/tests/main.cpp` — 6 L / 0 C
- [ ] `libs/firelight/cheats/include/firelight/cheats/cheat.hpp` — 49 L / 10 C
- [ ] `libs/firelight/cheats/include/firelight/cheats/cheat_engine.hpp` — 33 L / 8 C
- [ ] `libs/firelight/cheats/include/firelight/cheats/cheat_repository.hpp` — 31 L / 5 C
- [ ] `libs/firelight/cheats/include/firelight/cheats/sqlite_cheat_repository.hpp` — 28 L / 0 C
- [ ] `libs/firelight/cheats/src/cheat_engine.cpp` — 43 L / 2 C
- [ ] `libs/firelight/cheats/src/sqlite_cheat_repository.cpp` — 175 L / 4 C
- [ ] `libs/firelight/cheats/tests/cheat_engine_test.cpp` — 51 L / 0 C
- [ ] `libs/firelight/cheats/tests/cheat_repository_test.cpp` — 72 L / 5 C
- [ ] `libs/firelight/cheats/tests/fake_ram_core.hpp` — 72 L / 4 C
- [ ] `libs/firelight/cheats/tests/main.cpp` — 6 L / 0 C
- [ ] `libs/firelight/mods/include/firelight/mods/mod_info.hpp` — 21 L / 0 C
- [ ] `libs/firelight/mods/include/firelight/mods/mod_repository.hpp` — 13 L / 0 C
- [ ] `libs/firelight/mods/src/firelight/mods/sqlite_mod_repository.hpp` — 16 L / 0 C
- [ ] `libs/firelight/mods/src/sqlite_mod_repository.cpp` — 76 L / 0 C
- [ ] `libs/firelight/platforms/include/firelight/platforms/controller_input_descriptor.hpp` — 24 L / 0 C
- [ ] `libs/firelight/platforms/include/firelight/platforms/controller_type.hpp` — 40 L / 6 C
- [ ] `libs/firelight/platforms/include/firelight/platforms/platform.hpp` — 51 L / 3 C
- [ ] `libs/firelight/platforms/include/firelight/platforms/platform_service.hpp` — 64 L / 4 C
- [ ] `libs/firelight/platforms/src/platform_service.cpp` — 829 L / 9 C
- [ ] `libs/firelight/settings/include/firelight/settings/core_option.hpp` — 28 L / 5 C
- [ ] `libs/firelight/settings/include/firelight/settings/core_option_repository.hpp` — 26 L / 5 C
- [ ] `libs/firelight/settings/include/firelight/settings/setting_definition.hpp` — 161 L / 52 C
- [ ] `libs/firelight/settings/include/firelight/settings/settings_catalog.hpp` — 111 L / 42 C
- [ ] `libs/firelight/settings/include/firelight/settings/settings_index.hpp` — 62 L / 14 C
- [ ] `libs/firelight/settings/include/firelight/settings/settings_repository.hpp` — 34 L / 3 C
- [ ] `libs/firelight/settings/include/firelight/settings/settings_service.hpp` — 111 L / 17 C
- [ ] `libs/firelight/settings/src/firelight/settings/sqlite_core_option_repository.hpp` — 27 L / 0 C
- [ ] `libs/firelight/settings/src/firelight/settings/sqlite_settings_repository.hpp` — 54 L / 5 C
- [ ] `libs/firelight/settings/src/settings_catalog.cpp` — 486 L / 14 C
- [ ] `libs/firelight/settings/src/settings_index.cpp` — 161 L / 3 C
- [ ] `libs/firelight/settings/src/settings_service.cpp` — 147 L / 1 C
- [ ] `libs/firelight/settings/src/sqlite_core_option_repository.cpp` — 134 L / 7 C
- [ ] `libs/firelight/settings/src/sqlite_settings_repository.cpp` — 193 L / 4 C

## 02 libs - input

- [ ] `libs/firelight/input/include/firelight/input/analog_settings.hpp` — 142 L / 9 C
- [ ] `libs/firelight/input/include/firelight/input/binding.hpp` — 74 L / 4 C
- [ ] `libs/firelight/input/include/firelight/input/controller_repository.hpp` — 74 L / 16 C
- [ ] `libs/firelight/input/include/firelight/input/device_identifier.hpp` — 18 L / 3 C
- [ ] `libs/firelight/input/include/firelight/input/device_info.hpp` — 9 L / 0 C
- [ ] `libs/firelight/input/include/firelight/input/gamepad_profile.hpp` — 76 L / 10 C
- [ ] `libs/firelight/input/include/firelight/input/gamepad_type.hpp` — 19 L / 2 C
- [ ] `libs/firelight/input/include/firelight/input/igamepad.hpp` — 49 L / 4 C
- [ ] `libs/firelight/input/include/firelight/input/input_frame.hpp` — 97 L / 12 C
- [ ] `libs/firelight/input/include/firelight/input/input_mapping.hpp` — 83 L / 10 C
- [ ] `libs/firelight/input/include/firelight/input/input_service.hpp` — 93 L / 21 C
- [ ] `libs/firelight/input/include/firelight/input/input_source.hpp` — 56 L / 2 C
- [ ] `libs/firelight/input/include/firelight/input/keyboard_keycodes.hpp` — 23 L / 6 C
- [ ] `libs/firelight/input/include/firelight/input/shortcut_action.hpp` — 69 L / 24 C
- [ ] `libs/firelight/input/include/firelight/input/shortcut_engine.hpp` — 53 L / 19 C
- [ ] `libs/firelight/input/include/firelight/input/shortcut_mapping.hpp` — 38 L / 3 C
- [ ] `libs/firelight/input/include/firelight/input/shortcut_registry.hpp` — 76 L / 24 C
- [ ] `libs/firelight/input/src/firelight/input/keyboard_input_handler.hpp` — 81 L / 8 C
- [ ] `libs/firelight/input/src/firelight/input/sdl_controller.hpp` — 95 L / 8 C
- [ ] `libs/firelight/input/src/firelight/input/sdl_input_service.hpp` — 172 L / 38 C
- [ ] `libs/firelight/input/src/firelight/input/sqlite_controller_repository.hpp` — 71 L / 4 C
- [ ] `libs/firelight/input/src/gamepad_input.cpp` — 45 L / 4 C
- [ ] `libs/firelight/input/src/gamepad_profile.cpp` — 81 L / 0 C
- [ ] `libs/firelight/input/src/input_mapping.cpp` — 142 L / 2 C
- [ ] `libs/firelight/input/src/input_service.cpp` — 1 L / 0 C
- [ ] `libs/firelight/input/src/keyboard_input_handler.cpp` — 316 L / 3 C
- [ ] `libs/firelight/input/src/keyboard_keycodes.cpp` — 124 L / 5 C
- [ ] `libs/firelight/input/src/sdl_controller.cpp` — 524 L / 26 C
- [ ] `libs/firelight/input/src/sdl_input_service.cpp` — 660 L / 33 C
- [ ] `libs/firelight/input/src/shortcut_engine.cpp` — 204 L / 23 C
- [ ] `libs/firelight/input/src/shortcut_mapping.cpp` — 83 L / 0 C
- [ ] `libs/firelight/input/src/shortcut_registry.cpp` — 300 L / 7 C
- [ ] `libs/firelight/input/src/sqlite_controller_repository.cpp` — 740 L / 25 C
- [ ] `libs/firelight/input/tests/analog_cursor_test.cpp` — 98 L / 4 C
- [ ] `libs/firelight/input/tests/controller_repository_test.cpp` — 312 L / 21 C
- [ ] `libs/firelight/input/tests/gamepad_input_test.cpp` — 58 L / 7 C
- [ ] `libs/firelight/input/tests/gamepad_profile_test.cpp` — 69 L / 3 C
- [ ] `libs/firelight/input/tests/input_frame_test.cpp` — 84 L / 3 C
- [ ] `libs/firelight/input/tests/input_model_test.cpp` — 238 L / 9 C
- [ ] `libs/firelight/input/tests/input_service_impl_test.cpp` — 591 L / 41 C
- [ ] `libs/firelight/input/tests/main.cpp` — 10 L / 2 C
- [ ] `libs/firelight/input/tests/shortcut_registry_test.cpp` — 207 L / 17 C
- [ ] `libs/firelight/input/tests/shortcut_test.cpp` — 455 L / 32 C
- [ ] `libs/firelight/input/tests/test_gamepad.cpp` — 70 L / 0 C
- [ ] `libs/firelight/input/tests/test_gamepad.hpp` — 41 L / 0 C

## 03 libs - library

- [ ] `libs/firelight/library/include/firelight/library/content_directory.hpp` — 18 L / 2 C
- [ ] `libs/firelight/library/include/firelight/library/content_file.hpp` — 37 L / 10 C
- [ ] `libs/firelight/library/include/firelight/library/content_loader.hpp` — 32 L / 6 C
- [ ] `libs/firelight/library/include/firelight/library/disc_member.hpp` — 19 L / 4 C
- [ ] `libs/firelight/library/include/firelight/library/entry.hpp` — 37 L / 2 C
- [ ] `libs/firelight/library/include/firelight/library/entry_resolver.hpp` — 37 L / 4 C
- [ ] `libs/firelight/library/include/firelight/library/folder_entry_info.hpp` — 8 L / 0 C
- [ ] `libs/firelight/library/include/firelight/library/folder_info.hpp` — 43 L / 14 C
- [ ] `libs/firelight/library/include/firelight/library/library_events.hpp` — 46 L / 4 C
- [ ] `libs/firelight/library/include/firelight/library/patch_file.hpp` — 45 L / 0 C
- [ ] `libs/firelight/library/include/firelight/library/run_configuration.hpp` — 19 L / 0 C
- [ ] `libs/firelight/library/include/firelight/library/smart_folder.hpp` — 79 L / 31 C
- [ ] `libs/firelight/library/include/firelight/library/user_library_repository.hpp` — 94 L / 1 C
- [ ] `libs/firelight/library/include/firelight/library/user_library_service.hpp` — 48 L / 11 C
- [ ] `libs/firelight/library/src/archive_reader.cpp` — 175 L / 1 C
- [ ] `libs/firelight/library/src/content_hasher.cpp` — 179 L / 4 C
- [ ] `libs/firelight/library/src/content_identifier.cpp` — 95 L / 2 C
- [ ] `libs/firelight/library/src/content_loader.cpp` — 58 L / 4 C
- [ ] `libs/firelight/library/src/disc_inspector.cpp` — 323 L / 16 C
- [ ] `libs/firelight/library/src/entry_resolver.cpp` — 72 L / 2 C
- [ ] `libs/firelight/library/src/firelight/library/archive_reader.hpp` — 53 L / 11 C
- [ ] `libs/firelight/library/src/firelight/library/content_extensions.hpp` — 34 L / 9 C
- [ ] `libs/firelight/library/src/firelight/library/content_hasher.hpp` — 31 L / 8 C
- [ ] `libs/firelight/library/src/firelight/library/content_identifier.hpp` — 56 L / 10 C
- [ ] `libs/firelight/library/src/firelight/library/disc_inspector.hpp` — 68 L / 14 C
- [ ] `libs/firelight/library/src/firelight/library/file_bytes.hpp` — 36 L / 4 C
- [ ] `libs/firelight/library/src/firelight/library/library_ingest_service.hpp` — 25 L / 6 C
- [ ] `libs/firelight/library/src/firelight/library/library_scanner2.hpp` — 115 L / 29 C
- [ ] `libs/firelight/library/src/firelight/library/patch_associator.hpp` — 27 L / 8 C
- [ ] `libs/firelight/library/src/firelight/library/sqlite_user_library.hpp` — 102 L / 0 C
- [ ] `libs/firelight/library/src/library_ingest_service.cpp` — 63 L / 8 C
- [ ] `libs/firelight/library/src/library_scanner2.cpp` — 420 L / 26 C
- [ ] `libs/firelight/library/src/patch_file.cpp` — 49 L / 0 C
- [ ] `libs/firelight/library/src/smart_folder.cpp` — 245 L / 8 C
- [ ] `libs/firelight/library/src/sqlite_user_library_repository.cpp` — 1205 L / 58 C
- [ ] `libs/firelight/library/src/user_library_service.cpp` — 98 L / 4 C

## 04 libs - netplay

- [ ] `libs/firelight/netplay/include/firelight/netplay/base64.hpp` — 95 L / 0 C
- [ ] `libs/firelight/netplay/include/firelight/netplay/chat_log.hpp` — 38 L / 0 C
- [ ] `libs/firelight/netplay/include/firelight/netplay/lobby_backend.hpp` — 80 L / 10 C
- [ ] `libs/firelight/netplay/include/firelight/netplay/messages.hpp` — 96 L / 6 C
- [ ] `libs/firelight/netplay/include/firelight/netplay/protocol.hpp` — 21 L / 3 C
- [ ] `libs/firelight/netplay/include/firelight/netplay/rtc_transport.hpp` — 37 L / 7 C
- [ ] `libs/firelight/netplay/include/firelight/netplay/session.hpp` — 141 L / 26 C
- [ ] `libs/firelight/netplay/include/firelight/netplay/session_descriptor.hpp` — 22 L / 3 C
- [ ] `libs/firelight/netplay/include/firelight/netplay/session_phase.hpp` — 10 L / 3 C
- [ ] `libs/firelight/netplay/include/firelight/netplay/signal_chunker.hpp` — 42 L / 4 C
- [ ] `libs/firelight/netplay/include/firelight/netplay/slot_table.hpp` — 51 L / 5 C
- [ ] `libs/firelight/netplay/include/firelight/netplay/stream_packets.hpp` — 55 L / 8 C
- [ ] `libs/firelight/netplay/include/firelight/netplay/sync_strategy.hpp` — 10 L / 3 C
- [ ] `libs/firelight/netplay/include/firelight/netplay/transport.hpp` — 60 L / 10 C
- [ ] `libs/firelight/netplay/src/messages.cpp` — 229 L / 0 C
- [ ] `libs/firelight/netplay/src/protocol.cpp` — 23 L / 1 C
- [ ] `libs/firelight/netplay/src/rtc_transport.cpp` — 399 L / 10 C
- [ ] `libs/firelight/netplay/src/session.cpp` — 796 L / 11 C
- [ ] `libs/firelight/netplay/src/signal_chunker.cpp` — 70 L / 1 C
- [ ] `libs/firelight/netplay/src/slot_table.cpp` — 81 L / 0 C
- [ ] `libs/firelight/netplay/src/stream_packets.cpp` — 116 L / 0 C
- [ ] `libs/firelight/netplay/tests/fake_lobby_backend.hpp` — 143 L / 2 C
- [ ] `libs/firelight/netplay/tests/fake_transport.hpp` — 125 L / 5 C
- [ ] `libs/firelight/netplay/tests/main.cpp` — 6 L / 0 C
- [ ] `libs/firelight/netplay/tests/messages_test.cpp` — 97 L / 9 C
- [ ] `libs/firelight/netplay/tests/rtc_transport_test.cpp` — 125 L / 11 C
- [ ] `libs/firelight/netplay/tests/session_test.cpp` — 336 L / 11 C
- [ ] `libs/firelight/netplay/tests/signal_chunker_test.cpp` — 85 L / 2 C
- [ ] `libs/firelight/netplay/tests/slot_table_test.cpp` — 49 L / 0 C
- [ ] `libs/firelight/netplay/tests/stream_packets_test.cpp` — 70 L / 0 C

## 05 libs - achievements

- [ ] `libs/firelight/achievements/include/firelight/achievement.hpp` — 104 L / 0 C
- [ ] `libs/firelight/achievements/include/firelight/achievement_progress.hpp` — 10 L / 0 C
- [ ] `libs/firelight/achievements/include/firelight/achievement_service.hpp` — 152 L / 51 C
- [ ] `libs/firelight/achievements/include/firelight/achievement_service2.hpp` — 13 L / 0 C
- [ ] `libs/firelight/achievements/include/firelight/achievement_set.hpp` — 54 L / 2 C
- [ ] `libs/firelight/achievements/include/firelight/game.hpp` — 19 L / 0 C
- [ ] `libs/firelight/achievements/include/firelight/leaderboard.hpp` — 56 L / 0 C
- [ ] `libs/firelight/achievements/include/firelight/user.hpp` — 58 L / 0 C
- [ ] `libs/firelight/achievements/include/firelight/user_unlock.hpp` — 14 L / 0 C
- [ ] `libs/firelight/achievements/src/achievement_repository.hpp` — 310 L / 236 C
- [ ] `libs/firelight/achievements/src/achievement_service.cpp` — 447 L / 21 C
- [ ] `libs/firelight/achievements/src/rcheevos/achievement_set_response.hpp` — 70 L / 5 C
- [ ] `libs/firelight/achievements/src/rcheevos/award_achievement_response.hpp` — 19 L / 2 C
- [ ] `libs/firelight/achievements/src/rcheevos/gameid_response.hpp` — 13 L / 0 C
- [ ] `libs/firelight/achievements/src/rcheevos/login2_response.hpp` — 21 L / 0 C
- [ ] `libs/firelight/achievements/src/rcheevos/patch_response.hpp` — 796 L / 647 C
- [ ] `libs/firelight/achievements/src/rcheevos/ra_client.cpp` — 611 L / 12 C
- [ ] `libs/firelight/achievements/src/rcheevos/ra_client.hpp` — 183 L / 2 C
- [ ] `libs/firelight/achievements/src/rcheevos/ra_constants.h` — 10 L / 0 C
- [ ] `libs/firelight/achievements/src/rcheevos/ra_http_client.hpp` — 16 L / 0 C
- [ ] `libs/firelight/achievements/src/rcheevos/rcheevos_offline_client.cpp` — 486 L / 18 C
- [ ] `libs/firelight/achievements/src/rcheevos/rcheevos_offline_client.hpp` — 67 L / 0 C
- [ ] `libs/firelight/achievements/src/rcheevos/regular_http_client.cpp` — 35 L / 0 C
- [ ] `libs/firelight/achievements/src/rcheevos/regular_http_client.hpp` — 24 L / 1 C
- [ ] `libs/firelight/achievements/src/rcheevos/startsession_response.hpp` — 22 L / 0 C
- [ ] `libs/firelight/achievements/src/sqlite_achievement_repository.cpp` — 931 L / 92 C
- [ ] `libs/firelight/achievements/src/sqlite_achievement_repository.hpp` — 302 L / 232 C

## 06 libs - media

- [ ] `libs/firelight/media/include/firelight/media/clip_muxer.hpp` — 22 L / 7 C
- [ ] `libs/firelight/media/include/firelight/media/clip_recorder.hpp` — 58 L / 18 C
- [ ] `libs/firelight/media/include/firelight/media/clip_sink.hpp` — 32 L / 11 C
- [ ] `libs/firelight/media/include/firelight/media/clip_snapshot.hpp` — 36 L / 8 C
- [ ] `libs/firelight/media/include/firelight/media/clip_thumbnailer.hpp` — 16 L / 4 C
- [ ] `libs/firelight/media/include/firelight/media/game_capture.hpp` — 27 L / 3 C
- [ ] `libs/firelight/media/include/firelight/media/game_capture_repository.hpp` — 32 L / 4 C
- [ ] `libs/firelight/media/include/firelight/media/media_service.hpp` — 51 L / 15 C
- [ ] `libs/firelight/media/include/firelight/media/sqlite_game_capture_repository.hpp` — 35 L / 1 C
- [ ] `libs/firelight/media/include/firelight/media/stream_decoder.hpp` — 49 L / 6 C
- [ ] `libs/firelight/media/include/firelight/media/stream_encoder.hpp` — 60 L / 10 C
- [ ] `libs/firelight/media/src/clip_muxer.cpp` — 112 L / 2 C
- [ ] `libs/firelight/media/src/clip_recorder.cpp` — 398 L / 17 C
- [ ] `libs/firelight/media/src/clip_thumbnailer.cpp` — 114 L / 1 C
- [ ] `libs/firelight/media/src/media_service.cpp` — 136 L / 3 C
- [ ] `libs/firelight/media/src/sqlite_game_capture_repository.cpp` — 201 L / 7 C
- [ ] `libs/firelight/media/src/stream_decoder.cpp` — 251 L / 0 C
- [ ] `libs/firelight/media/src/stream_encoder.cpp` — 395 L / 4 C
- [ ] `libs/firelight/media/tests/clip_recorder_test.cpp` — 100 L / 10 C
- [ ] `libs/firelight/media/tests/clip_thumbnailer_test.cpp` — 66 L / 3 C
- [ ] `libs/firelight/media/tests/game_capture_repository_test.cpp` — 88 L / 0 C
- [ ] `libs/firelight/media/tests/main.cpp` — 6 L / 0 C
- [ ] `libs/firelight/media/tests/media_service_test.cpp` — 119 L / 3 C
- [ ] `libs/firelight/media/tests/stream_codec_test.cpp` — 139 L / 6 C

## 07 libs - saves

- [ ] `libs/firelight/saves/include/firelight/saves/isave_manager.hpp` — 48 L / 6 C
- [ ] `libs/firelight/saves/include/firelight/saves/save_database.hpp` — 40 L / 2 C
- [ ] `libs/firelight/saves/include/firelight/saves/save_events.hpp` — 23 L / 3 C
- [ ] `libs/firelight/saves/include/firelight/saves/save_manager.hpp` — 2 L / 0 C
- [ ] `libs/firelight/saves/include/firelight/saves/savefile.hpp` — 19 L / 0 C
- [ ] `libs/firelight/saves/include/firelight/saves/savefile_info.hpp` — 17 L / 0 C
- [ ] `libs/firelight/saves/include/firelight/saves/savefile_metadata.hpp` — 17 L / 2 C
- [ ] `libs/firelight/saves/include/firelight/saves/suspend_point.hpp` — 17 L / 0 C
- [ ] `libs/firelight/saves/include/firelight/saves/suspend_point_metadata.hpp` — 18 L / 2 C
- [ ] `libs/firelight/saves/src/firelight/saves/detail/md5.hpp` — 153 L / 3 C
- [ ] `libs/firelight/saves/src/firelight/saves/save_manager.hpp` — 6 L / 3 C
- [ ] `libs/firelight/saves/src/firelight/saves/save_manager_impl.hpp` — 57 L / 5 C
- [ ] `libs/firelight/saves/src/firelight/saves/sqlite_save_database.hpp` — 52 L / 4 C
- [ ] `libs/firelight/saves/src/save_manager.cpp` — 384 L / 10 C
- [ ] `libs/firelight/saves/src/savefile.cpp` — 17 L / 0 C
- [ ] `libs/firelight/saves/src/sqlite_save_database.cpp` — 240 L / 7 C
- [ ] `libs/firelight/saves/tests/fake_save_database.hpp` — 102 L / 4 C
- [ ] `libs/firelight/saves/tests/main.cpp` — 6 L / 0 C
- [ ] `libs/firelight/saves/tests/save_manager_test.cpp` — 102 L / 3 C
- [ ] `libs/firelight/saves/tests/savefile_metadata_test.cpp` — 18 L / 0 C
- [ ] `libs/firelight/saves/tests/sqlite_save_database_test.cpp` — 109 L / 4 C

## 08 libs - metadata

- [ ] `libs/firelight/metadata/include/firelight/metadata/art_provider.hpp` — 50 L / 20 C
- [ ] `libs/firelight/metadata/include/firelight/metadata/game_metadata.hpp` — 48 L / 10 C
- [ ] `libs/firelight/metadata/include/firelight/metadata/game_metadata_source.hpp` — 24 L / 8 C
- [ ] `libs/firelight/metadata/include/firelight/metadata/http_client.hpp` — 30 L / 2 C
- [ ] `libs/firelight/metadata/include/firelight/metadata/media_asset.hpp` — 57 L / 15 C
- [ ] `libs/firelight/metadata/include/firelight/metadata/media_asset_repository.hpp` — 71 L / 42 C
- [ ] `libs/firelight/metadata/include/firelight/metadata/sqlite_game_metadata_source.hpp` — 28 L / 3 C
- [ ] `libs/firelight/metadata/include/firelight/metadata/sqlite_media_asset_repository.hpp` — 44 L / 3 C
- [ ] `libs/firelight/metadata/include/firelight/metadata/steamgriddb_art_provider.hpp` — 55 L / 15 C
- [ ] `libs/firelight/metadata/src/media_asset.cpp` — 31 L / 0 C
- [ ] `libs/firelight/metadata/src/sqlite_game_metadata_source.cpp` — 75 L / 3 C
- [ ] `libs/firelight/metadata/src/sqlite_media_asset_repository.cpp` — 246 L / 11 C
- [ ] `libs/firelight/metadata/src/steamgriddb_art_provider.cpp` — 161 L / 0 C
- [ ] `libs/firelight/metadata/tests/game_metadata_source_test.cpp` — 84 L / 1 C
- [ ] `libs/firelight/metadata/tests/main.cpp` — 6 L / 0 C
- [ ] `libs/firelight/metadata/tests/media_asset_repository_test.cpp` — 104 L / 0 C
- [ ] `libs/firelight/metadata/tests/steamgriddb_art_provider_test.cpp` — 144 L / 0 C

## 09 libs - other

- [ ] `libs/firelight/discord/include/firelight/discord/discord_lobby_backend.hpp` — 64 L / 8 C
- [ ] `libs/firelight/discord/include/firelight/discord/discord_manager.hpp` — 21 L / 0 C
- [ ] `libs/firelight/discord/include/firelight/discord/token_store.hpp` — 36 L / 3 C
- [ ] `libs/firelight/discord/src/discord_lobby_backend.cpp` — 432 L / 7 C
- [ ] `libs/firelight/discord/src/discord_manager.cpp` — 110 L / 0 C
- [ ] `libs/firelight/discord/src/file_token_store.cpp` — 33 L / 0 C
- [ ] `libs/firelight/discord/src/firelight/discord/discord_manager_impl.hpp` — 37 L / 1 C
- [ ] `libs/firelight/libretro/include/core.hpp` — 4 L / 0 C

## 10 include (ours)

- [ ] `include/firelight/achievements/iachievement_client.hpp` — 24 L / 2 C
- [ ] `include/firelight/event_dispatcher.hpp` — 97 L / 12 C
- [ ] `include/firelight/image.hpp` — 17 L / 3 C
- [ ] `include/firelight/input/gamepad_input.hpp` — 241 L / 28 C
- [ ] `include/firelight/input/input_suppressor.hpp` — 67 L / 17 C
- [ ] `include/firelight/libretro/audio_input_provider.hpp` — 16 L / 0 C
- [ ] `include/firelight/libretro/audio_output.hpp` — 23 L / 4 C
- [ ] `include/firelight/libretro/camera_interface_provider.hpp` — 9 L / 0 C
- [ ] `include/firelight/libretro/configuration_provider.hpp` — 49 L / 5 C
- [ ] `include/firelight/libretro/core_run_config.hpp` — 21 L / 3 C
- [ ] `include/firelight/libretro/icore.hpp` — 131 L / 47 C
- [ ] `include/firelight/libretro/keyboard_input_provider.hpp` — 8 L / 0 C
- [ ] `include/firelight/libretro/location_data_provider.hpp` — 9 L / 0 C
- [ ] `include/firelight/libretro/pointer_input_provider.hpp` — 56 L / 24 C
- [ ] `include/firelight/libretro/retropad.hpp` — 75 L / 5 C
- [ ] `include/firelight/libretro/retropad_provider.hpp` — 17 L / 3 C
- [ ] `include/firelight/libretro/sensor_interface_provider.hpp` — 9 L / 0 C
- [ ] `include/firelight/libretro/system_info_provider.hpp` — 9 L / 1 C
- [ ] `include/firelight/libretro/video_data_receiver.hpp` — 37 L / 3 C
- [ ] `include/firelight/migrations/migration_runner.hpp` — 35 L / 8 C

## 11 src/app - library

- [ ] `src/app/library/gui/content_directory_model.cpp` — 92 L / 1 C
- [ ] `src/app/library/gui/content_directory_model.hpp` — 39 L / 0 C
- [ ] `src/app/library/gui/entry_list_model.cpp` — 531 L / 13 C
- [ ] `src/app/library/gui/entry_list_model.hpp` — 182 L / 35 C
- [ ] `src/app/library/gui/library_entry_item.cpp` — 58 L / 0 C
- [ ] `src/app/library/gui/library_entry_item.hpp` — 72 L / 5 C
- [ ] `src/app/library/gui/library_path_model.cpp` — 73 L / 1 C
- [ ] `src/app/library/gui/library_path_model.hpp` — 31 L / 0 C
- [ ] `src/app/library/gui/playlist_item_model.cpp` — 270 L / 3 C
- [ ] `src/app/library/gui/playlist_item_model.hpp` — 91 L / 15 C

## 12 src/app - input

- [ ] `src/app/input/controller_icons.hpp` — 40 L / 1 C
- [ ] `src/app/input/gui/analog_settings_model.cpp` — 116 L / 3 C
- [ ] `src/app/input/gui/analog_settings_model.hpp` — 106 L / 5 C
- [ ] `src/app/input/gui/binding_list_model.cpp` — 171 L / 0 C
- [ ] `src/app/input/gui/binding_list_model.hpp` — 75 L / 5 C
- [ ] `src/app/input/gui/controller_list_model.cpp` — 109 L / 2 C
- [ ] `src/app/input/gui/controller_list_model.hpp` — 55 L / 0 C
- [ ] `src/app/input/gui/gamepad_status_item.cpp` — 147 L / 50 C
- [ ] `src/app/input/gui/gamepad_status_item.hpp` — 69 L / 1 C
- [ ] `src/app/input/gui/input_mappings_model.cpp` — 333 L / 1 C
- [ ] `src/app/input/gui/input_mappings_model.hpp` — 86 L / 0 C
- [ ] `src/app/input/gui/platform_input_preferences.cpp` — 58 L / 0 C
- [ ] `src/app/input/gui/platform_input_preferences.hpp` — 28 L / 6 C
- [ ] `src/app/input/gui/profile_list_model.cpp` — 159 L / 0 C
- [ ] `src/app/input/gui/profile_list_model.hpp` — 56 L / 5 C

## 13 src/app - netplay

- [ ] `src/app/netplay/direct_lobby_backend.cpp` — 554 L / 7 C
- [ ] `src/app/netplay/direct_lobby_backend.hpp` — 86 L / 11 C
- [ ] `src/app/netplay/guest_stream_receiver.cpp` — 130 L / 0 C
- [ ] `src/app/netplay/guest_stream_receiver.hpp` — 63 L / 5 C
- [ ] `src/app/netplay/gui/netplay_chat_model.cpp` — 49 L / 0 C
- [ ] `src/app/netplay/gui/netplay_chat_model.hpp` — 37 L / 0 C
- [ ] `src/app/netplay/gui/netplay_slots_model.cpp` — 58 L / 0 C
- [ ] `src/app/netplay/gui/netplay_slots_model.hpp` — 44 L / 2 C
- [ ] `src/app/netplay/gui/netplay_stream_item.cpp` — 55 L / 0 C
- [ ] `src/app/netplay/gui/netplay_stream_item.hpp` — 42 L / 4 C
- [ ] `src/app/netplay/host_stream_sender.cpp` — 172 L / 3 C
- [ ] `src/app/netplay/host_stream_sender.hpp` — 69 L / 8 C
- [ ] `src/app/netplay/netplay_retropad_provider.hpp` — 64 L / 4 C
- [ ] `src/app/netplay/netplay_service.cpp` — 295 L / 5 C
- [ ] `src/app/netplay/netplay_service.hpp` — 100 L / 19 C
- [ ] `src/app/netplay/remote_retropad.hpp` — 91 L / 3 C
- [ ] `src/app/netplay/tee_audio_output.hpp` — 67 L / 3 C

## 14 src/app - libretro

- [ ] `src/app/libretro/core.cpp` — 389 L / 30 C
- [ ] `src/app/libretro/core.hpp` — 263 L / 20 C
- [ ] `src/app/libretro/core_configuration.cpp` — 133 L / 15 C
- [ ] `src/app/libretro/core_configuration.hpp` — 61 L / 3 C
- [ ] `src/app/libretro/core_environment.cpp` — 1014 L / 80 C
- [ ] `src/app/libretro/core_environment.hpp` — 16 L / 4 C
- [ ] `src/app/libretro/core_input_router.cpp` — 213 L / 12 C
- [ ] `src/app/libretro/core_input_router.hpp` — 72 L / 4 C
- [ ] `src/app/libretro/core_registry.cpp` — 303 L / 27 C
- [ ] `src/app/libretro/core_registry.hpp` — 116 L / 38 C
- [ ] `src/app/libretro/game.cpp` — 43 L / 1 C
- [ ] `src/app/libretro/libretro_dll.cpp` — 146 L / 4 C
- [ ] `src/app/libretro/libretro_dll.hpp` — 83 L / 7 C
- [ ] `src/app/libretro/platform_core_defaults.hpp` — 265 L / 14 C

## 15 src/app - emulation

- [ ] `src/app/emulation/core_settings_applier.cpp` — 114 L / 7 C
- [ ] `src/app/emulation/core_settings_applier.hpp` — 34 L / 3 C
- [ ] `src/app/emulation/emulation_context.hpp` — 68 L / 11 C
- [ ] `src/app/emulation/emulation_service.cpp` — 191 L / 16 C
- [ ] `src/app/emulation/emulation_service.hpp` — 147 L / 31 C
- [ ] `src/app/emulation/emulator_controller.hpp` — 40 L / 16 C
- [ ] `src/app/emulation/emulator_instance.cpp` — 587 L / 55 C
- [ ] `src/app/emulation/emulator_instance.hpp` — 224 L / 67 C
- [ ] `src/app/emulation/game_loader.cpp` — 143 L / 6 C
- [ ] `src/app/emulation/game_loader.hpp` — 57 L / 7 C
- [ ] `src/app/emulation/shortcut_actions.cpp` — 224 L / 13 C
- [ ] `src/app/emulation/shortcut_actions.hpp` — 89 L / 29 C
- [ ] `src/app/emulation/shortcut_dispatcher.cpp` — 57 L / 6 C
- [ ] `src/app/emulation/shortcut_dispatcher.hpp` — 58 L / 20 C

## 16 src/app - saves

- [ ] `src/app/saves/gui/save_files_item.cpp` — 58 L / 0 C
- [ ] `src/app/saves/gui/save_files_item.hpp` — 38 L / 0 C
- [ ] `src/app/saves/gui/savefile_list_model.hpp` — 35 L / 0 C
- [ ] `src/app/saves/gui/suspend_point_list_model.cpp` — 98 L / 0 C
- [ ] `src/app/saves/gui/suspend_point_list_model.hpp` — 60 L / 4 C
- [ ] `src/app/saves/gui/suspend_points_item.cpp` — 48 L / 0 C
- [ ] `src/app/saves/gui/suspend_points_item.hpp` — 77 L / 0 C

## 17 src/app - achievements

- [ ] `src/app/achievements/gui/AchievementSetItem.cpp` — 59 L / 0 C
- [ ] `src/app/achievements/gui/AchievementSetItem.hpp` — 65 L / 0 C
- [ ] `src/app/achievements/gui/achievement_list_model.cpp` — 93 L / 0 C
- [ ] `src/app/achievements/gui/achievement_list_model.hpp` — 47 L / 0 C
- [ ] `src/app/achievements/gui/retro_achievements_game_item.cpp` — 58 L / 0 C
- [ ] `src/app/achievements/gui/retro_achievements_game_item.hpp` — 60 L / 0 C

## 18 src/app - other

- [ ] `src/app/activity/gui/game_activity_item.cpp` — 43 L / 0 C
- [ ] `src/app/activity/gui/game_activity_item.hpp` — 40 L / 0 C
- [ ] `src/app/activity/gui/play_session_list_model.hpp` — 31 L / 0 C
- [ ] `src/app/audio/SfxPlayer.hpp` — 48 L / 16 C
- [ ] `src/app/audio/audio_manager.cpp` — 261 L / 26 C
- [ ] `src/app/audio/audio_manager.hpp` — 133 L / 46 C
- [ ] `src/app/audio/audio_settings.hpp` — 21 L / 9 C
- [ ] `src/app/audio/qt_microphone.cpp` — 106 L / 1 C
- [ ] `src/app/audio/qt_microphone.hpp` — 30 L / 3 C
- [ ] `src/app/emulator_item.cpp` — 661 L / 88 C
- [ ] `src/app/emulator_item.hpp` — 256 L / 31 C
- [ ] `src/app/emulator_item_renderer.cpp` — 771 L / 81 C
- [ ] `src/app/emulator_item_renderer.hpp` — 267 L / 36 C
- [ ] `src/app/emulator_vulkan_renderer.cpp` — 867 L / 80 C
- [ ] `src/app/emulator_vulkan_renderer.hpp` — 175 L / 19 C
- [ ] `src/app/media/gui/capture_list_model.cpp` — 133 L / 0 C
- [ ] `src/app/media/gui/capture_list_model.hpp` — 72 L / 5 C
- [ ] `src/app/metadata/cpr_http_client.cpp` — 28 L / 2 C
- [ ] `src/app/metadata/cpr_http_client.hpp` — 17 L / 3 C
- [ ] `src/app/metadata/metadata_service.cpp` — 192 L / 12 C
- [ ] `src/app/metadata/metadata_service.hpp` — 69 L / 18 C
- [ ] `src/app/mods/gui/ModInfoItem.cpp` — 135 L / 34 C
- [ ] `src/app/mods/gui/ModInfoItem.hpp` — 65 L / 0 C
- [ ] `src/app/service_accessor.cpp` — 136 L / 0 C
- [ ] `src/app/service_accessor.hpp` — 140 L / 7 C
- [ ] `src/app/util/fuzzy_string_matcher.hpp` — 46 L / 6 C

## 19 src/gui

- [ ] `src/gui/eventhandlers/windows_frame_filter.cpp` — 173 L / 28 C
- [ ] `src/gui/eventhandlers/windows_frame_filter.hpp` — 38 L / 4 C
- [ ] `src/gui/filesystem_utils.cpp` — 97 L / 18 C
- [ ] `src/gui/filesystem_utils.hpp` — 19 L / 3 C
- [ ] `src/gui/game_image_provider.cpp` — 66 L / 5 C
- [ ] `src/gui/game_image_provider.hpp` — 33 L / 4 C
- [ ] `src/gui/gamepad_profile_item.cpp` — 54 L / 0 C
- [ ] `src/gui/gamepad_profile_item.hpp` — 52 L / 3 C
- [ ] `src/gui/image_qt.hpp` — 34 L / 2 C
- [ ] `src/gui/image_utils.cpp` — 72 L / 3 C
- [ ] `src/gui/image_utils.hpp` — 18 L / 6 C
- [ ] `src/gui/models/activity_buckets_list_model.cpp` — 273 L / 9 C
- [ ] `src/gui/models/activity_buckets_list_model.hpp` — 92 L / 1 C
- [ ] `src/gui/models/core_options_model.cpp` — 271 L / 5 C
- [ ] `src/gui/models/core_options_model.hpp` — 107 L / 13 C
- [ ] `src/gui/models/game_activity_list_model.cpp` — 99 L / 3 C
- [ ] `src/gui/models/game_activity_list_model.hpp` — 38 L / 0 C
- [ ] `src/gui/models/search_results_list_model.cpp` — 169 L / 7 C
- [ ] `src/gui/models/search_results_list_model.hpp` — 117 L / 1 C
- [ ] `src/gui/models/setting_binding.cpp` — 127 L / 2 C
- [ ] `src/gui/models/setting_binding.hpp` — 69 L / 13 C
- [ ] `src/gui/models/settings_model.cpp` — 538 L / 27 C
- [ ] `src/gui/models/settings_model.hpp` — 185 L / 38 C
- [ ] `src/gui/models/settings_search_model.cpp` — 86 L / 0 C
- [ ] `src/gui/models/settings_search_model.hpp` — 59 L / 6 C
- [ ] `src/gui/models/shop/shop_item_model.cpp` — 154 L / 83 C
- [ ] `src/gui/models/shop/shop_item_model.hpp` — 64 L / 8 C
- [ ] `src/gui/platform_list_model.cpp` — 89 L / 4 C
- [ ] `src/gui/platform_list_model.hpp` — 40 L / 0 C
- [ ] `src/gui/qt_achievement_service_proxy.cpp` — 32 L / 0 C
- [ ] `src/gui/qt_achievement_service_proxy.hpp` — 37 L / 0 C
- [ ] `src/gui/qt_core_registry_proxy.cpp` — 66 L / 0 C
- [ ] `src/gui/qt_core_registry_proxy.hpp` — 36 L / 6 C
- [ ] `src/gui/qt_emulation_service_proxy.cpp` — 190 L / 2 C
- [ ] `src/gui/qt_emulation_service_proxy.hpp` — 88 L / 7 C
- [ ] `src/gui/qt_game_art_proxy.cpp` — 176 L / 11 C
- [ ] `src/gui/qt_game_art_proxy.hpp` — 105 L / 23 C
- [ ] `src/gui/qt_input_service_proxy.cpp` — 216 L / 16 C
- [ ] `src/gui/qt_input_service_proxy.hpp` — 78 L / 9 C
- [ ] `src/gui/qt_network_service_proxy.cpp` — 295 L / 2 C
- [ ] `src/gui/qt_network_service_proxy.hpp` — 106 L / 13 C
- [ ] `src/gui/qt_save_manager_proxy.cpp` — 29 L / 2 C
- [ ] `src/gui/qt_save_manager_proxy.hpp` — 32 L / 3 C
- [ ] `src/gui/qt_settings_catalog_proxy.cpp` — 23 L / 2 C
- [ ] `src/gui/qt_settings_catalog_proxy.hpp` — 21 L / 4 C
- [ ] `src/gui/settings_level_shim.hpp` — 34 L / 7 C
- [ ] `src/gui/shortcuts_model.cpp` — 255 L / 3 C
- [ ] `src/gui/shortcuts_model.hpp` — 97 L / 17 C

## 20 src/cli

- [ ] `src/cli/cli_app.cpp` — 124 L / 7 C
- [ ] `src/cli/cli_app.hpp` — 55 L / 9 C
- [ ] `src/cli/console.cpp` — 23 L / 2 C
- [ ] `src/cli/console.hpp` — 11 L / 4 C
- [ ] `src/cli/data_dirs.cpp` — 36 L / 0 C
- [ ] `src/cli/data_dirs.hpp` — 23 L / 5 C
- [ ] `src/cli/launch_config.cpp` — 142 L / 2 C
- [ ] `src/cli/launch_config.hpp` — 31 L / 12 C
- [ ] `src/cli/list_command.cpp` — 117 L / 0 C
- [ ] `src/cli/list_command.hpp` — 13 L / 4 C
- [ ] `src/cli/login_command.cpp` — 123 L / 4 C
- [ ] `src/cli/login_command.hpp` — 13 L / 4 C
- [ ] `src/cli/rom_launch.cpp` — 43 L / 0 C
- [ ] `src/cli/rom_launch.hpp` — 23 L / 4 C
- [ ] `src/cli/scan_command.cpp` — 57 L / 6 C
- [ ] `src/cli/scan_command.hpp` — 12 L / 3 C
- [ ] `src/cli/single_instance.cpp` — 89 L / 3 C
- [ ] `src/cli/single_instance.hpp` — 56 L / 9 C
- [ ] `src/cli/startup_options.hpp` — 70 L / 11 C

## 21 src - root

- [ ] `src/main.cpp` — 998 L / 157 C

## 22 qml - components/v2

- [ ] `qml/components/v2/ButtonBar.qml` — 135 L / 0 C
- [ ] `qml/components/v2/FLBadge.qml` — 31 L / 1 C
- [ ] `qml/components/v2/FLEmptyState.qml` — 43 L / 1 C
- [ ] `qml/components/v2/FLScrollBar.qml` — 37 L / 0 C
- [ ] `qml/components/v2/FLSectionHeader.qml` — 12 L / 1 C
- [ ] `qml/components/v2/FrostedPane.qml` — 68 L / 2 C
- [ ] `qml/components/v2/GameplayLayer.qml` — 221 L / 25 C
- [ ] `qml/components/v2/LeftNavigationBar2.qml` — 88 L / 9 C
- [ ] `qml/components/v2/MainNavigationMenuItem.qml` — 72 L / 4 C
- [ ] `qml/components/v2/MainWindow.qml` — 99 L / 1 C
- [ ] `qml/components/v2/RouteOverlay.qml` — 113 L / 12 C
- [ ] `qml/components/v2/RouteView.qml` — 306 L / 36 C
- [ ] `qml/components/v2/Toast.qml` — 52 L / 5 C
- [ ] `qml/components/v2/activity/ActivityPageV2.qml` — 534 L / 36 C
- [ ] `qml/components/v2/buttons/FLButton.qml` — 91 L / 8 C
- [ ] `qml/components/v2/buttons/FLIconButton.qml` — 69 L / 12 C
- [ ] `qml/components/v2/buttons/FLSegmentedControl.qml` — 60 L / 5 C
- [ ] `qml/components/v2/buttons/FLTabBar.qml` — 56 L / 4 C
- [ ] `qml/components/v2/buttons/IconButton.qml` — 58 L / 0 C
- [ ] `qml/components/v2/gallery/CaptureViewer.qml` — 84 L / 1 C
- [ ] `qml/components/v2/gallery/GalleryGridView.qml` — 100 L / 2 C
- [ ] `qml/components/v2/gallery/GalleryPage.qml` — 107 L / 2 C
- [ ] `qml/components/v2/inputs/FLComboBox.qml` — 77 L / 2 C
- [ ] `qml/components/v2/inputs/FLSearchField.qml` — 68 L / 3 C
- [ ] `qml/components/v2/inputs/FLSlider.qml` — 38 L / 2 C
- [ ] `qml/components/v2/inputs/FLTextField.qml` — 31 L / 3 C
- [ ] `qml/components/v2/inputs/FLToggle.qml` — 38 L / 2 C
- [ ] `qml/components/v2/library/AddToFolderDialog.qml` — 114 L / 2 C
- [ ] `qml/components/v2/library/GameArtPickerDialog.qml` — 452 L / 23 C
- [ ] `qml/components/v2/library/GameContextMenu.qml` — 50 L / 5 C
- [ ] `qml/components/v2/library/GameDetailPanel.qml` — 225 L / 7 C
- [ ] `qml/components/v2/library/GameGridView.qml` — 285 L / 43 C
- [ ] `qml/components/v2/library/GameListHeader.qml` — 13 L / 0 C
- [ ] `qml/components/v2/library/GameListView.qml` — 460 L / 8 C
- [ ] `qml/components/v2/library/GameTile.qml` — 49 L / 4 C
- [ ] `qml/components/v2/library/GameView.qml` — 762 L / 37 C
- [ ] `qml/components/v2/library/LibraryNavigationMenu.qml` — 122 L / 11 C
- [ ] `qml/components/v2/library/LibraryNavigationMenuItem.qml` — 164 L / 29 C
- [ ] `qml/components/v2/library/LibraryNavigationMenuSection.qml` — 163 L / 32 C
- [ ] `qml/components/v2/library/LibraryPageV2.qml` — 706 L / 81 C
- [ ] `qml/components/v2/library/ListViewColumnHeader.qml` — 52 L / 0 C
- [ ] `qml/components/v2/netplay/GuestStreamPage.qml` — 58 L / 2 C
- [ ] `qml/components/v2/netplay/LobbyChatPanel.qml` — 73 L / 0 C
- [ ] `qml/components/v2/netplay/LobbyChip.qml` — 47 L / 2 C
- [ ] `qml/components/v2/netplay/NetplayGamePickerDialog.qml` — 54 L / 0 C
- [ ] `qml/components/v2/netplay/NetplayLobbyPage.qml` — 249 L / 5 C
- [ ] `qml/components/v2/netplay/NetplayPage.qml` — 166 L / 2 C
- [ ] `qml/components/v2/netplay/ReadyCheckToast.qml` — 103 L / 2 C
- [ ] `qml/components/v2/netplay/SlotCard.qml` — 60 L / 0 C
- [ ] `qml/components/v2/settings/AppearanceSettingsPage.qml` — 7 L / 2 C
- [ ] `qml/components/v2/settings/NotificationSettings.qml` — 7 L / 2 C
- [ ] `qml/components/v2/settings/SettingsScreen.qml` — 509 L / 58 C
- [ ] `qml/components/v2/settings/SystemSettingsPage.qml` — 6 L / 1 C
- [ ] `qml/components/v2/surfaces/FLDivider.qml` — 16 L / 2 C
- [ ] `qml/components/v2/surfaces/FLListRow.qml` — 60 L / 5 C
- [ ] `qml/components/v2/surfaces/FLPanel.qml` — 24 L / 6 C
- [ ] `qml/components/v2/surfaces/FLScrollView.qml` — 13 L / 2 C
- [ ] `qml/components/v2/titlebar/TitleBar.qml` — 206 L / 25 C
- [ ] `qml/components/v2/titlebar/TitleBarProfileButton.qml` — 70 L / 0 C
- [ ] `qml/components/v2/titlebar/TitleBarSearchBar.qml` — 461 L / 22 C
- [ ] `qml/components/v2/titlebar/TitleBarUtilityButtons.qml` — 130 L / 4 C

## 23 qml - components/settings

- [ ] `qml/components/settings/BaseSettingItem.qml` — 221 L / 27 C
- [ ] `qml/components/settings/ColorSettingItem.qml` — 74 L / 4 C
- [ ] `qml/components/settings/FilePathSettingItem.qml` — 82 L / 4 C
- [ ] `qml/components/settings/KeyBindingSettingItem.qml` — 41 L / 3 C
- [ ] `qml/components/settings/MultiSelectSettingItem.qml` — 94 L / 3 C
- [ ] `qml/components/settings/PaletteSettingItem.qml` — 52 L / 6 C
- [ ] `qml/components/settings/RadioSettingItem.qml` — 83 L / 3 C
- [ ] `qml/components/settings/SegmentedSettingItem.qml` — 69 L / 4 C
- [ ] `qml/components/settings/SettingsGroup.qml` — 323 L / 12 C
- [ ] `qml/components/settings/SettingsPage.qml` — 51 L / 11 C
- [ ] `qml/components/settings/SettingsSection.qml` — 103 L / 12 C
- [ ] `qml/components/settings/SettingsSectionHeader.qml` — 37 L / 0 C
- [ ] `qml/components/settings/SliderSettingItem.qml` — 54 L / 2 C
- [ ] `qml/components/settings/StepperSettingItem.qml` — 81 L / 3 C
- [ ] `qml/components/settings/TextSettingItem.qml` — 37 L / 3 C
- [ ] `qml/components/settings/ToggleSettingItem.qml` — 47 L / 0 C

## 24 qml - components

- [ ] `qml/components/CoreSelector.qml` — 58 L / 3 C
- [ ] `qml/components/EmulationSettingsSurface.qml` — 201 L / 17 C
- [ ] `qml/components/FLDateTime.qml` — 51 L / 0 C
- [ ] `qml/components/FLFocusHighlight.qml` — 137 L / 4 C
- [ ] `qml/components/FLGameActivityPage.qml` — 328 L / 37 C
- [ ] `qml/components/FLIcon.qml` — 10 L / 4 C
- [ ] `qml/components/FLInputGuideBar.qml` — 147 L / 6 C
- [ ] `qml/components/FLModShopItemPanel.qml` — 231 L / 21 C
- [ ] `qml/components/FLSuspendPointCard.qml` — 182 L / 9 C
- [ ] `qml/components/FLThing.qml` — 489 L / 78 C
- [ ] `qml/components/FLToolTip.qml` — 42 L / 0 C
- [ ] `qml/components/FLTwoColumnMenu.qml` — 218 L / 18 C
- [ ] `qml/components/FLUserBackground.qml` — 108 L / 7 C
- [ ] `qml/components/FirelightDialog.qml` — 185 L / 13 C
- [ ] `qml/components/GameSettingsView.qml` — 69 L / 1 C
- [ ] `qml/components/HelpArticle.qml` — 46 L / 4 C
- [ ] `qml/components/Icon.qml` — 45 L / 13 C
- [ ] `qml/components/MainContent.qml` — 145 L / 0 C
- [ ] `qml/components/QuickMenu.qml` — 1552 L / 658 C
- [ ] `qml/components/RadioIconButton.qml` — 154 L / 3 C
- [ ] `qml/components/achievements/AchievementListButton.qml` — 170 L / 0 C
- [ ] `qml/components/activity/ActivityPage.qml` — 189 L / 89 C
- [ ] `qml/components/controllers/ControllerInputMappingView.qml` — 572 L / 202 C
- [ ] `qml/components/controllers/InputPromptDialog.qml` — 164 L / 3 C
- [ ] `qml/components/controllers/ShortcutInputPromptDialog.qml` — 199 L / 41 C
- [ ] `qml/components/emulation/EmulatorLoader.qml` — 59 L / 5 C
- [ ] `qml/components/library/CreateFolderDialog.qml` — 84 L / 8 C
- [ ] `qml/components/library/EditEntryDialog.qml` — 128 L / 66 C
- [ ] `qml/components/library/LibraryEntryListDelegate.qml` — 131 L / 20 C
- [ ] `qml/components/library/LibraryPage.qml` — 344 L / 42 C
- [ ] `qml/components/library/ManageSaveDataDialog.qml` — 156 L / 28 C
- [ ] `qml/components/library/SmartFolderDialog.qml` — 512 L / 20 C
- [ ] `qml/components/library/UpdateFolderDialog.qml` — 70 L / 7 C
- [ ] `qml/components/navigation/FLNavItem.qml` — 64 L / 0 C
- [ ] `qml/components/navigation/FLNavSection.qml` — 114 L / 58 C
- [ ] `qml/components/navigation/LeftNavigationBar.qml` — 271 L / 27 C
- [ ] `qml/components/navigation/LeftNavigationItem.qml` — 80 L / 0 C

## 25 qml - common

- [ ] `qml/common/ComboBoxOption2.qml` — 66 L / 3 C
- [ ] `qml/common/DetailsButton.qml` — 20 L / 0 C
- [ ] `qml/common/DirectoryOption.qml` — 143 L / 59 C
- [ ] `qml/common/FileOption.qml` — 147 L / 59 C
- [ ] `qml/common/FirelightButton.qml` — 100 L / 10 C
- [ ] `qml/common/LibraryEntryComboBox.qml` — 188 L / 82 C
- [ ] `qml/common/ListViewSectionDelegate.qml` — 16 L / 0 C
- [ ] `qml/common/MyComboBox.qml` — 10 L / 3 C
- [ ] `qml/common/MySlider.qml` — 168 L / 75 C
- [ ] `qml/common/NavigationTabBar.qml` — 67 L / 0 C
- [ ] `qml/common/Option.qml` — 88 L / 14 C
- [ ] `qml/common/OptionGroup.qml` — 71 L / 18 C
- [ ] `qml/common/RadioButtonGroup.qml` — 112 L / 17 C
- [ ] `qml/common/RightClickMenu.qml` — 103 L / 17 C
- [ ] `qml/common/RightClickMenuItem.qml` — 120 L / 23 C
- [ ] `qml/common/SelectLibraryEntryDialog.qml` — 143 L / 2 C
- [ ] `qml/common/ToggleOption.qml` — 115 L / 9 C

## 26 qml - other

- [ ] `qml/AppStyle.qml` — 77 L / 25 C
- [ ] `qml/AppearanceSettings.qml` — 78 L / 30 C
- [ ] `qml/Constants.qml` — 124 L / 2 C
- [ ] `qml/FirelightMenuItem.qml` — 70 L / 2 C
- [ ] `qml/GeneralSettings.qml` — 43 L / 9 C
- [ ] `qml/Main3.qml` — 682 L / 39 C
- [ ] `qml/Main4.qml` — 641 L / 458 C
- [ ] `qml/MaterialSymbols.qml` — 71 L / 4 C
- [ ] `qml/NavMenuButton.qml` — 68 L / 12 C
- [ ] `qml/Notification.qml` — 96 L / 27 C
- [ ] `qml/RewindMenu.qml` — 335 L / 149 C
- [ ] `qml/Router.qml` — 105 L / 8 C
- [ ] `qml/Theme.qml` — 122 L / 32 C
- [ ] `qml/WindowGeometry.qml` — 16 L / 3 C
- [ ] `qml/achievements/AchievementList.qml` — 130 L / 17 C
- [ ] `qml/achievements/AchievementListItem.qml` — 152 L / 3 C
- [ ] `qml/achievements/AchievementProgressIndicator.qml` — 196 L / 0 C
- [ ] `qml/achievements/AchievementUnlockIndicator.qml` — 226 L / 0 C
- [ ] `qml/achievements/ChallengeIndicatorList.qml` — 97 L / 0 C
- [ ] `qml/achievements/GameLaunchPopup.qml` — 133 L / 2 C
- [ ] `qml/achievements/RetroAchievementsAccountPane.qml` — 368 L / 19 C
- [ ] `qml/controllers/AnalogTuningPage.qml` — 281 L / 11 C
- [ ] `qml/controllers/BindingOptionsPage.qml` — 186 L / 5 C
- [ ] `qml/controllers/ControllerProfilePage.qml` — 300 L / 30 C
- [ ] `qml/controllers/ControllerTest.qml` — 146 L / 87 C
- [ ] `qml/controllers/KeyboardProfilePage.qml` — 514 L / 33 C
- [ ] `qml/controllers/ProfileManagementPage.qml` — 190 L / 4 C
- [ ] `qml/discover/AddedPopup.qml` — 82 L / 2 C
- [ ] `qml/library/EntrySummaryPage.qml` — 53 L / 1 C
- [ ] `qml/library/GameGridItemDelegate.qml` — 168 L / 10 C
- [ ] `qml/library/ManageSavefilesDialog.qml` — 255 L / 21 C
- [ ] `qml/library/ScanPopup.qml` — 70 L / 2 C
- [ ] `qml/pages/ControllersPage.qml` — 528 L / 124 C
- [ ] `qml/pages/DiscoverPage.qml` — 223 L / 6 C
- [ ] `qml/pages/GameDetailsPage.qml` — 161 L / 0 C
- [ ] `qml/pages/NewEmulatorPage.qml` — 571 L / 151 C
- [ ] `qml/pages/ShopLandingPage.qml` — 123 L / 27 C
- [ ] `qml/pages/StoreContent.qml` — 497 L / 167 C
- [ ] `qml/platforms/GameBoyColorSettings.qml` — 188 L / 9 C
- [ ] `qml/platforms/GameBoySettings.qml` — 453 L / 12 C
- [ ] `qml/platforms/GameGearSettings.qml` — 78 L / 0 C
- [ ] `qml/platforms/GbaSettings.qml` — 211 L / 49 C
- [ ] `qml/platforms/GenesisSettings.qml` — 78 L / 0 C
- [ ] `qml/platforms/MasterSystemSettings.qml` — 78 L / 0 C
- [ ] `qml/platforms/NesSettings.qml` — 78 L / 0 C
- [ ] `qml/platforms/Nintendo64Settings.qml` — 78 L / 0 C
- [ ] `qml/platforms/NintendoDsSettings.qml` — 78 L / 0 C
- [ ] `qml/platforms/PlatformEmulationSettingsPage.qml` — 79 L / 3 C
- [ ] `qml/platforms/PlatformSettingsPage.qml` — 115 L / 2 C
- [ ] `qml/platforms/SnesSettings.qml` — 78 L / 0 C
- [ ] `qml/screens/EmulatorScreen.qml` — 381 L / 19 C
- [ ] `qml/screens/HelpScreen.qml` — 70 L / 3 C
- [ ] `qml/screens/HomeScreen.qml` — 303 L / 46 C
- [ ] `qml/screens/NewUserScreen.qml` — 485 L / 32 C
- [ ] `qml/settings/AboutPage.qml` — 16 L / 0 C
- [ ] `qml/settings/ControllerSettings.qml` — 369 L / 18 C
- [ ] `qml/settings/DirectorySettings.qml` — 193 L / 39 C
- [ ] `qml/settings/GlobalEmulationSettings.qml` — 38 L / 3 C
- [ ] `qml/settings/LibrarySettings.qml` — 167 L / 47 C
- [ ] `qml/settings/RetroAchievementSettings.qml` — 118 L / 1 C
- [ ] `qml/shop/ShopGridItemDelegate.qml` — 111 L / 21 C

## 27 tests

- [ ] `tests/app/achievements/achievement_service_test.cpp` — 1088 L / 337 C
- [ ] `tests/app/achievements/sqlite_achievement_repository_test.cpp` — 1733 L / 141 C
- [ ] `tests/app/activity/sqlite_activity_log_test.cpp` — 84 L / 4 C
- [ ] `tests/app/cli/cli_app_test.cpp` — 153 L / 2 C
- [ ] `tests/app/cli/launch_config_test.cpp` — 119 L / 2 C
- [ ] `tests/app/cli/rom_launch_test.cpp` — 52 L / 3 C
- [ ] `tests/app/cli/single_instance_test.cpp` — 26 L / 1 C
- [ ] `tests/app/db/daos/play_session_test.cpp` — 19 L / 1 C
- [ ] `tests/app/emulation/core_configuration_test.cpp` — 113 L / 6 C
- [ ] `tests/app/emulation/core_registry_test.cpp` — 182 L / 19 C
- [ ] `tests/app/emulation/emulation_service_test.cpp` — 283 L / 49 C
- [ ] `tests/app/emulation/emulator_instance_e2e_test.cpp` — 220 L / 23 C
- [ ] `tests/app/emulation/emulator_instance_test.cpp` — 566 L / 159 C
- [ ] `tests/app/emulation/fake_core.hpp` — 215 L / 18 C
- [ ] `tests/app/emulation/fake_input_service.hpp` — 75 L / 4 C
- [ ] `tests/app/emulation/hotkeys_disabled_test.cpp` — 163 L / 8 C
- [ ] `tests/app/emulation/shortcut_actions_test.cpp` — 353 L / 24 C
- [ ] `tests/app/emulation/shortcut_dispatcher_test.cpp` — 134 L / 10 C
- [ ] `tests/app/event_dispatcher_test.cpp` — 51 L / 3 C
- [ ] `tests/app/gui/core_options_model_test.cpp` — 173 L / 10 C
- [ ] `tests/app/gui/filesystem_utils_test.cpp` — 39 L / 6 C
- [ ] `tests/app/gui/settings_model_test.cpp` — 547 L / 35 C
- [ ] `tests/app/gui/shortcuts_model_test.cpp` — 196 L / 15 C
- [ ] `tests/app/library/archive_reader_test.cpp` — 95 L / 1 C
- [ ] `tests/app/library/content_extensions_test.cpp` — 63 L / 9 C
- [ ] `tests/app/library/content_identifier_test.cpp` — 136 L / 15 C
- [ ] `tests/app/library/content_loader_test.cpp` — 218 L / 17 C
- [ ] `tests/app/library/entry_list_model_test.cpp` — 104 L / 8 C
- [ ] `tests/app/library/entry_resolver_test.cpp` — 48 L / 3 C
- [ ] `tests/app/library/library_ingest_service_test.cpp` — 134 L / 20 C
- [ ] `tests/app/library/library_scanner_test.cpp` — 291 L / 37 C
- [ ] `tests/app/library/smart_folder_test.cpp` — 246 L / 6 C
- [ ] `tests/app/library/sqlite_user_library_test.cpp` — 725 L / 29 C
- [ ] `tests/app/libretro/core_input_router_test.cpp` — 177 L / 6 C
- [ ] `tests/app/media/capture_list_model_test.cpp` — 88 L / 0 C
- [ ] `tests/app/metadata/metadata_service_test.cpp` — 219 L / 15 C
- [ ] `tests/app/migrations/migration_runner_test.cpp` — 59 L / 0 C
- [ ] `tests/app/netplay/direct_lobby_backend_test.cpp` — 155 L / 1 C
- [ ] `tests/app/netplay/netplay_service_test.cpp` — 173 L / 13 C
- [ ] `tests/app/platforms/platform_service_test.cpp` — 657 L / 492 C
- [ ] `tests/app/rcheevos/rcheevos_offline_client_test.cpp` — 526 L / 156 C
- [ ] `tests/app/settings/setting_definition_test.cpp` — 54 L / 1 C
- [ ] `tests/app/settings/settings_catalog_test.cpp` — 663 L / 51 C
- [ ] `tests/app/settings/settings_index_test.cpp` — 173 L / 11 C
- [ ] `tests/app/settings/settings_service_test.cpp` — 570 L / 154 C
- [ ] `tests/app/settings/sqlite_core_option_repository_test.cpp` — 103 L / 5 C
- [ ] `tests/app/settings/sqlite_settings_repository_test.cpp` — 771 L / 215 C
- [ ] `tests/main.cpp` — 39 L / 4 C

## 28 misc

- [ ] `qml_tests/tst_routing.qml` — 174 L / 9 C
