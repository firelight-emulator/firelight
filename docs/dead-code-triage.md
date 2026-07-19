# Commented-out code triage

Contiguous `//` runs of 2+ lines where at least one line looks like code.

Mark each file **delete** or **keep** (or note a mixed case) and I'll execute it.

Deliberately NOT auto-deleted: disabled tests, API-reference blocks and parked UI
experiments are indistinguishable to a regex but mean very different things.

**Regenerated against the current tree.** The previous version of this file counted
7193 lines across 521 blocks in 284 files; much of that has since been removed, so
those numbers were stale. Current totals: **3265 lines** across **252 blocks** in
**164 files**.

Of that, **2107 lines across 8 files are already decided by your earlier calls** —
"keep the disabled tests" and "keep the sample blob" — and are pre-marked below.
That leaves roughly **1158 lines across 156 files** actually needing a decision, and
most of those files carry only a handful of lines each.

`Kind` is a guess from the block contents: `disabled-test` contains a commented
`TEST_F`/`TEST`, `data-sample` looks like a JSON payload. Blank means neither.

| File | Kind | Dead lines | Blocks | Biggest | Decision |
|---|---|---|---|---|---|
| `libs/firelight/achievements/src/rcheevos/patch_response.hpp` | `data-sample` | 647 | 3 | 637 | keep (sample) |
| `libs/firelight/achievements/src/rcheevos/achievement_set_response.hpp` | `data-sample` | 512 | 2 | 507 | keep (sample) |
| `tests/app/platforms/platform_service_test.cpp` | `disabled-test` | 475 | 1 | 475 | keep (disabled test) |
| `tests/app/achievements/achievement_service_test.cpp` | `disabled-test` | 278 | 11 | 41 | keep (disabled test) |
| `src/app/libretro/virtual_filesystem.hpp` |  | 164 | 1 | 164 |  |
| `qml_tests/components/tst_IconButton.qml` |  | 158 | 2 | 147 |  |
| `tests/app/rcheevos/rcheevos_offline_client_test.cpp` | `disabled-test` | 126 | 2 | 106 | keep (disabled test) |
| `tests/app/emulation/emulator_instance_test.cpp` | `disabled-test` | 42 | 2 | 40 | keep (disabled test) |
| `qml/pages/NewEmulatorPage.qml` |  | 30 | 5 | 17 |  |
| `src/main.cpp` |  | 26 | 5 | 7 |  |
| `qml/components/library/LibraryPage.qml` |  | 23 | 1 | 23 |  |
| `libs/firelight/settings/include/firelight/settings/settings_catalog.hpp` |  | 21 | 2 | 16 |  |
| `qml/components/v2/settings/SettingsScreen.qml` | `data-sample` | 20 | 1 | 20 | keep (sample) |
| `qml/Main3.qml` |  | 20 | 4 | 12 |  |
| `src/app/libretro/core_environment.cpp` |  | 18 | 6 | 6 |  |
| `src/app/emulator_vulkan_renderer.cpp` |  | 18 | 2 | 13 |  |
| `src/app/emulator_item.cpp` |  | 18 | 3 | 12 |  |
| `src/app/libretro/core_registry.hpp` |  | 16 | 4 | 7 |  |
| `src/app/audio/SfxPlayer.hpp` |  | 15 | 1 | 15 |  |
| `qml_tests/components/tst_FLFocusHighlight.qml` |  | 15 | 1 | 15 |  |
| `qml/controllers/KeyboardProfilePage.qml` |  | 15 | 5 | 6 |  |
| `libs/firelight/library/include/firelight/library/smart_folder.hpp` |  | 14 | 2 | 9 |  |
| `src/http2config.hpp` |  | 13 | 1 | 13 |  |
| `libs/firelight/settings/include/firelight/settings/setting_definition.hpp` |  | 13 | 5 | 3 |  |
| `src/app/audio/audio_manager.hpp` |  | 12 | 3 | 4 |  |
| `src/gui/qt_game_art_proxy.hpp` |  | 10 | 4 | 3 |  |
| `qml/components/settings/SettingsPage.qml` |  | 10 | 1 | 10 |  |
| `libs/firelight/library/src/sqlite_user_library_repository.cpp` |  | 10 | 4 | 3 |  |
| `include/firelight/input/gamepad_input.hpp` |  | 10 | 3 | 5 |  |
| `src/app/libretro/core_configuration.cpp` |  | 9 | 1 | 9 |  |
| `src/app/library/gui/entry_list_model.hpp` |  | 9 | 3 | 4 |  |
| `src/app/emulator_item_renderer.hpp` |  | 9 | 2 | 7 |  |
| `src/app/emulation/emulator_instance.hpp` |  | 9 | 4 | 3 |  |
| `qml/controllers/ControllerProfilePage.qml` |  | 9 | 2 | 6 |  |
| `src/app/netplay/direct_lobby_backend.hpp` |  | 8 | 1 | 8 |  |
| `src/app/emulation/shortcut_dispatcher.hpp` |  | 8 | 1 | 8 |  |
| `src/app/emulation/emulator_instance.cpp` |  | 8 | 2 | 5 |  |
| `src/app/emulation/emulation_service.hpp` |  | 8 | 3 | 3 |  |
| `qml/screens/EmulatorScreen.qml` |  | 8 | 3 | 4 |  |
| `libs/firelight/input/include/firelight/input/input_service.hpp` |  | 8 | 2 | 4 |  |
| `include/firelight/libretro/pointer_input_provider.hpp` |  | 8 | 2 | 6 |  |
| `tests/app/patching/util_test.cpp` |  | 7 | 2 | 4 |  |
| `src/cli/launch_config.hpp` | `data-sample` | 7 | 1 | 7 | keep (sample) |
| `src/app/library/gui/playlist_item_model.hpp` |  | 6 | 2 | 4 |  |
| `qml/components/v2/buttons/FLIconButton.qml` |  | 6 | 1 | 6 |  |
| `qml/components/v2/buttons/FLButton.qml` |  | 6 | 1 | 6 |  |
| `qml/components/FLThing.qml` |  | 6 | 1 | 6 |  |
| `qml/components/FLImageCarousel.qml` |  | 6 | 1 | 6 |  |
| `qml/components/FLGameActivityPage.qml` |  | 6 | 3 | 2 |  |
| `qml/common/ImageViewer.qml` |  | 6 | 1 | 6 |  |
| `libs/firelight/settings/include/firelight/settings/settings_service.hpp` |  | 6 | 1 | 6 |  |
| `libs/firelight/platforms/include/firelight/platforms/controller_type.hpp` |  | 6 | 1 | 6 |  |
| `libs/firelight/library/src/firelight/library/patch_associator.hpp` |  | 6 | 1 | 6 |  |
| `include/firelight/libretro/icore.hpp` |  | 6 | 1 | 6 |  |
| `src/app/emulator_item_renderer.cpp` |  | 5 | 2 | 3 |  |
| `qml/settings/ControllerSettings.qml` |  | 5 | 1 | 5 |  |
| `qml/components/v2/surfaces/FLPanel.qml` |  | 5 | 1 | 5 |  |
| `qml/components/v2/surfaces/FLListRow.qml` |  | 5 | 1 | 5 |  |
| `qml/components/v2/library/LibraryPageV2.qml` |  | 5 | 2 | 3 |  |
| `qml/components/v2/RouteView.qml` |  | 5 | 2 | 3 |  |
| `libs/firelight/settings/include/firelight/settings/settings_index.hpp` |  | 5 | 2 | 3 |  |
| `libs/firelight/media/include/firelight/media/stream_encoder.hpp` |  | 5 | 1 | 5 |  |
| `libs/firelight/library/src/firelight/library/content_extensions.hpp` |  | 5 | 1 | 5 |  |
| `libs/firelight/achievements/include/firelight/achievement_service.hpp` |  | 5 | 1 | 5 |  |
| `include/firelight/libretro/retropad.hpp` |  | 5 | 1 | 5 |  |
| `tests/app/library/library_ingest_service_test.cpp` |  | 4 | 2 | 2 |  |
| `src/gui/qt_emulation_service_proxy.hpp` |  | 4 | 1 | 4 |  |
| `src/cli/cli_app.hpp` |  | 4 | 2 | 2 |  |
| `src/app/netplay/netplay_service.hpp` |  | 4 | 2 | 2 |  |
| `src/app/libretro/libretro_dll.cpp` |  | 4 | 1 | 4 |  |
| `src/app/input/gui/analog_settings_model.hpp` |  | 4 | 1 | 4 |  |
| `src/app/emulation/game_loader.hpp` |  | 4 | 1 | 4 |  |
| `src/app/emulation/emulation_service.cpp` |  | 4 | 1 | 4 |  |
| `qml/pages/ControllersPage.qml` |  | 4 | 2 | 2 |  |
| `qml/library/GameGridItemDelegate.qml` |  | 4 | 1 | 4 |  |
| `qml/components/v2/buttons/FLTabBar.qml` |  | 4 | 1 | 4 |  |
| `qml/components/v2/buttons/FLSegmentedControl.qml` |  | 4 | 1 | 4 |  |
| `qml/components/library/SmartFolderDialog.qml` |  | 4 | 1 | 4 |  |
| `qml/components/Icon.qml` |  | 4 | 1 | 4 |  |
| `qml/components/HelpArticle.qml` |  | 4 | 1 | 4 |  |
| `qml/Theme.qml` |  | 4 | 1 | 4 |  |
| `qml/MaterialSymbols.qml` |  | 4 | 1 | 4 |  |
| `libs/firelight/settings/src/settings_catalog.cpp` |  | 4 | 2 | 2 |  |
| `libs/firelight/platforms/src/platform_service.cpp` |  | 4 | 1 | 4 |  |
| `libs/firelight/media/tests/clip_recorder_test.cpp` |  | 4 | 2 | 2 |  |
| `libs/firelight/media/include/firelight/media/clip_recorder.hpp` |  | 4 | 2 | 2 |  |
| `libs/firelight/library/include/firelight/library/library_events.hpp` |  | 4 | 1 | 4 |  |
| `libs/firelight/library/include/firelight/library/folder_info.hpp` |  | 4 | 2 | 2 |  |
| `libs/firelight/library/include/firelight/library/content_file.hpp` |  | 4 | 1 | 4 |  |
| `tests/app/gui/settings_model_test.cpp` |  | 3 | 1 | 3 |  |
| `tests/app/emulation/core_registry_test.cpp` |  | 3 | 1 | 3 |  |
| `src/gui/qt_core_registry_proxy.hpp` |  | 3 | 1 | 3 |  |
| `src/gui/eventhandlers/windows_frame_filter.cpp` |  | 3 | 1 | 3 |  |
| `src/cli/startup_options.hpp` |  | 3 | 1 | 3 |  |
| `src/cli/single_instance.hpp` |  | 3 | 1 | 3 |  |
| `src/app/netplay/tee_audio_output.hpp` |  | 3 | 1 | 3 |  |
| `src/app/metadata/cpr_http_client.hpp` |  | 3 | 1 | 3 |  |
| `src/app/libretro/core_registry.cpp` |  | 3 | 1 | 3 |  |
| `src/app/emulation/emulation_context.hpp` |  | 3 | 1 | 3 |  |
| `qml/settings/LibrarySettings.qml` |  | 3 | 1 | 3 |  |
| `qml/platforms/GameBoySettings.qml` |  | 3 | 1 | 3 |  |
| `qml/platforms/GameBoyColorSettings.qml` |  | 3 | 1 | 3 |  |
| `qml/pages/ShopItemPage.qml` |  | 3 | 1 | 3 |  |
| `qml/components/v2/library/LibraryNavigationMenuItem.qml` |  | 3 | 1 | 3 |  |
| `qml/components/v2/library/GameView.qml` |  | 3 | 1 | 3 |  |
| `qml/components/v2/MainNavigationMenuItem.qml` |  | 3 | 1 | 3 |  |
| `qml/components/settings/TextSettingItem.qml` |  | 3 | 1 | 3 |  |
| `qml/components/settings/StepperSettingItem.qml` |  | 3 | 1 | 3 |  |
| `qml/components/settings/SegmentedSettingItem.qml` |  | 3 | 1 | 3 |  |
| `qml/components/settings/MultiSelectSettingItem.qml` |  | 3 | 1 | 3 |  |
| `qml/components/controllers/ControllerInputMappingView.qml` |  | 3 | 1 | 3 |  |
| `qml/components/QuickMenu.qml` |  | 3 | 1 | 3 |  |
| `qml/components/EmulationSettingsSurface.qml` |  | 3 | 1 | 3 |  |
| `qml/components/CoreSelector.qml` |  | 3 | 1 | 3 |  |
| `qml/achievements/AchievementList.qml` |  | 3 | 1 | 3 |  |
| `libs/firelight/settings/include/firelight/settings/settings_repository.hpp` |  | 3 | 1 | 3 |  |
| `libs/firelight/netplay/include/firelight/netplay/sync_strategy.hpp` |  | 3 | 1 | 3 |  |
| `libs/firelight/media/include/firelight/media/media_service.hpp` |  | 3 | 1 | 3 |  |
| `libs/firelight/library/src/firelight/library/disc_inspector.hpp` |  | 3 | 1 | 3 |  |
| `libs/firelight/input/src/sdl_controller.cpp` |  | 3 | 1 | 3 |  |
| `libs/firelight/input/src/firelight/input/keyboard_input_handler.hpp` |  | 3 | 1 | 3 |  |
| `libs/firelight/input/include/firelight/input/shortcut_mapping.hpp` |  | 3 | 1 | 3 |  |
| `include/firelight/image.hpp` |  | 3 | 1 | 3 |  |
| `tests/qml_main.cpp` |  | 2 | 1 | 2 |  |
| `tests/app/libretro/core_input_router_test.cpp` |  | 2 | 1 | 2 |  |
| `tests/app/library/sqlite_user_library_test.cpp` |  | 2 | 1 | 2 |  |
| `tests/app/library/library_scanner_test.cpp` |  | 2 | 1 | 2 |  |
| `tests/app/library/content_loader_test.cpp` |  | 2 | 1 | 2 |  |
| `tests/app/emulation/hotkeys_disabled_test.cpp` |  | 2 | 1 | 2 |  |
| `tests/app/emulation/emulator_instance_e2e_test.cpp` |  | 2 | 1 | 2 |  |
| `src/gui/qt_settings_catalog_proxy.cpp` |  | 2 | 1 | 2 |  |
| `src/gui/qt_game_art_proxy.cpp` |  | 2 | 1 | 2 |  |
| `src/gui/models/settings_model.hpp` |  | 2 | 1 | 2 |  |
| `src/gui/models/core_options_model.hpp` |  | 2 | 1 | 2 |  |
| `src/gui/game_image_provider.hpp` |  | 2 | 1 | 2 |  |
| `src/gui/game_image_provider.cpp` |  | 2 | 1 | 2 |  |
| `src/cli/scan_command.cpp` |  | 2 | 1 | 2 |  |
| `src/cli/data_dirs.hpp` |  | 2 | 1 | 2 |  |
| `src/cli/cli_app.cpp` |  | 2 | 1 | 2 |  |
| `src/app/netplay/gui/netplay_slots_model.hpp` |  | 2 | 1 | 2 |  |
| `src/app/metadata/metadata_service.hpp` |  | 2 | 1 | 2 |  |
| `src/app/metadata/cpr_http_client.cpp` |  | 2 | 1 | 2 |  |
| `src/app/libretro/libretro_dll.hpp` |  | 2 | 1 | 2 |  |
| `src/app/libretro/core.hpp` |  | 2 | 1 | 2 |  |
| `src/app/libretro/core.cpp` |  | 2 | 1 | 2 |  |
| `src/app/library/gui/library_entry_item.hpp` |  | 2 | 1 | 2 |  |
| `src/app/library/gui/entry_list_model.cpp` |  | 2 | 1 | 2 |  |
| `src/app/emulation/game_loader.cpp` |  | 2 | 1 | 2 |  |
| `src/app/audio/audio_manager.cpp` |  | 2 | 1 | 2 |  |
| `qml/components/v2/library/GameListView.qml` |  | 2 | 1 | 2 |  |
| `qml/components/v2/library/GameDetailPanel.qml` |  | 2 | 1 | 2 |  |
| `qml/components/v2/RouteOverlay.qml` |  | 2 | 1 | 2 |  |
| `qml/components/v2/GameplayLayer.qml` |  | 2 | 1 | 2 |  |
| `qml/components/settings/SliderSettingItem.qml` |  | 2 | 1 | 2 |  |
| `qml/components/controllers/InputPromptDialog.qml` |  | 2 | 1 | 2 |  |
| `libs/firelight/saves/include/firelight/saves/suspend_point_metadata.hpp` |  | 2 | 1 | 2 |  |
| `libs/firelight/netplay/src/rtc_transport.cpp` |  | 2 | 1 | 2 |  |
| `libs/firelight/netplay/include/firelight/netplay/transport.hpp` |  | 2 | 1 | 2 |  |
| `libs/firelight/media/src/clip_recorder.cpp` |  | 2 | 1 | 2 |  |
| `libs/firelight/media/src/clip_muxer.cpp` |  | 2 | 1 | 2 |  |
| `libs/firelight/library/include/firelight/library/content_directory.hpp` |  | 2 | 1 | 2 |  |
| `libs/firelight/input/src/shortcut_engine.cpp` |  | 2 | 1 | 2 |  |
| `libs/firelight/input/src/sdl_input_service.cpp` |  | 2 | 1 | 2 |  |
| `libs/firelight/audio/include/firelight/audio/audio_resampler.hpp` |  | 2 | 1 | 2 |  |
