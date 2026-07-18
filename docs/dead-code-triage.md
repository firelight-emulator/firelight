# Commented-out code triage

Contiguous `//` runs of 2+ lines where at least one line looks like code.

Mark each file **delete** or **keep** (or note a mixed case) and I'll execute it.

Deliberately NOT auto-deleted: disabled tests, API-reference blocks and parked UI
experiments are indistinguishable to a regex but mean very different things.

Totals: **7193 lines** across **521 blocks** in **284 files**.

| File | Dead lines | Blocks | Biggest | Decision |
|---|---|---|---|---|
| `qml/components/FLGameDetailsPanel.qml` | 660 | 3 | 495 | |
| `libs/firelight/achievements/src/rcheevos/patch_response.hpp` | 640 | 2 | 637 | |
| `qml/components/QuickMenu.qml` | 638 | 10 | 391 | |
| `tests/app/platforms/platform_service_test.cpp` | 475 | 1 | 475 | |
| `qml/Main4.qml` | 448 | 4 | 373 | |
| `tests/app/achievements/achievement_service_test.cpp` | 278 | 11 | 41 | |
| `qml/components/controllers/ControllerInputMappingView.qml` | 184 | 6 | 46 | |
| `qml/pages/StoreContent.qml` | 167 | 3 | 153 | |
| `src/app/libretro/virtual_filesystem.hpp` | 164 | 1 | 164 | |
| `qml/RewindMenu.qml` | 144 | 2 | 142 | |
| `qml/pages/NewEmulatorPage.qml` | 141 | 8 | 107 | |
| `tests/app/rcheevos/rcheevos_offline_client_test.cpp` | 126 | 2 | 106 | |
| `qml/pages/ControllersPage.qml` | 113 | 9 | 32 | |
| `qml/components/activity/ActivityPage.qml` | 89 | 1 | 89 | |
| `qml/controllers/ControllerTest.qml` | 85 | 2 | 56 | |
| `src/gui/models/shop/shop_item_model.cpp` | 83 | 1 | 83 | |
| `qml/common/LibraryEntryComboBox.qml` | 80 | 2 | 69 | |
| `qml/components/FLThing.qml` | 71 | 6 | 28 | |
| `qml/common/MySlider.qml` | 71 | 6 | 33 | |
| `qml/components/library/EditEntryDialog.qml` | 64 | 3 | 53 | |
| `src/main.cpp` | 62 | 16 | 11 | |
| `qml/components/navigation/FLNavSection.qml` | 56 | 1 | 56 | |
| `qml/common/FileOption.qml` | 56 | 2 | 49 | |
| `qml/common/DirectoryOption.qml` | 56 | 2 | 49 | |
| `qml/components/v2/library/LibraryPageV2.qml` | 50 | 3 | 28 | |
| `src/app/input/gui/gamepad_status_item.cpp` | 47 | 1 | 47 | |
| `qml/platforms/GbaSettings.qml` | 44 | 4 | 11 | |
| `qml/home/HomeContentPane.qml` | 44 | 1 | 44 | |
| `qml/settings/LibrarySettings.qml` | 43 | 4 | 23 | |
| `tests/app/emulation/emulator_instance_test.cpp` | 42 | 2 | 40 | |
| `src/app/emulator_item.cpp` | 42 | 10 | 12 | |
| `qml/screens/HomeScreen.qml` | 41 | 3 | 27 | |
| `qml/components/library/LibraryPage.qml` | 38 | 2 | 23 | |
| `qml/settings/DirectorySettings.qml` | 36 | 1 | 36 | |
| `src/app/mods/gui/ModInfoItem.cpp` | 34 | 1 | 34 | |
| `qml/components/v2/activity/ActivityPageV2.qml` | 31 | 1 | 31 | |
| `qml/components/controllers/ShortcutInputPromptDialog.qml` | 31 | 3 | 26 | |
| `qml/components/FLGameActivityPage.qml` | 31 | 1 | 31 | |
| `qml/components/v2/library/LibraryNavigationMenuSection.qml` | 28 | 3 | 15 | |
| `qml/components/v2/settings/SettingsScreen.qml` | 27 | 4 | 20 | |
| `qml/components/v2/library/GameGridView.qml` | 27 | 3 | 20 | |
| `qml/Main3.qml` | 26 | 6 | 12 | |
| `qml/screens/NewUserScreen.qml` | 25 | 3 | 11 | |
| `qml/pages/ShopLandingPage.qml` | 25 | 2 | 18 | |
| `qml/components/navigation/LeftNavigationBar.qml` | 25 | 2 | 14 | |
| `qml/components/library/ManageSaveDataDialog.qml` | 25 | 2 | 20 | |
| `qml/pages/ShopItemPage.qml` | 23 | 2 | 20 | |
| `qml/pages/DashboardTesting.qml` | 23 | 2 | 12 | |
| `libs/firelight/settings/include/firelight/settings/settings_catalog.hpp` | 23 | 3 | 16 | |
| `qml/Notification.qml` | 22 | 1 | 22 | |
| `qml/library/ManageSavefilesDialog.qml` | 21 | 2 | 17 | |
| `qml/components/v2/RouteView.qml` | 21 | 4 | 14 | |
| `src/app/libretro/core_environment.cpp` | 19 | 8 | 4 | |
| `src/app/emulation/emulator_instance.cpp` | 19 | 5 | 5 | |
| `qml/controllers/ControllerProfilePage.qml` | 19 | 4 | 7 | |
| `src/app/emulation/emulator_instance.hpp` | 18 | 8 | 3 | |
| `qml/components/library/LibraryEntryListDelegate.qml` | 18 | 1 | 18 | |
| `qml/components/FLModShopItemPanel.qml` | 18 | 2 | 9 | |
| `qml/common/OptionGroup.qml` | 18 | 1 | 18 | |
| `src/gui/filesystem_utils.cpp` | 17 | 2 | 15 | |
| `qml/controllers/KeyboardProfilePage.qml` | 17 | 3 | 8 | |
| `qml/components/v2/titlebar/TitleBarSearchBar.qml` | 17 | 2 | 10 | |
| `qml/components/v2/library/LibraryNavigationMenuItem.qml` | 17 | 3 | 11 | |
| `qml/achievements/AchievementList.qml` | 17 | 3 | 8 | |
| `src/http2config.hpp` | 16 | 2 | 13 | |
| `src/app/libretro/core_registry.hpp` | 16 | 3 | 7 | |
| `src/app/audio/audio_manager.hpp` | 15 | 5 | 4 | |
| `src/app/audio/SfxPlayer.hpp` | 15 | 1 | 15 | |
| `qml/components/v2/GameplayLayer.qml` | 15 | 2 | 13 | |
| `qml/common/RightClickMenuItem.qml` | 15 | 2 | 13 | |
| `qml/achievements/RetroAchievementsAccountPane.qml` | 15 | 1 | 15 | |
| `qml/components/FLImageCarousel.qml` | 14 | 2 | 8 | |
| `qml/common/ImageViewer.qml` | 14 | 2 | 8 | |
| `qml/AppStyle.qml` | 14 | 3 | 9 | |
| `libs/firelight/netplay/include/firelight/netplay/session.hpp` | 14 | 4 | 5 | |
| `libs/firelight/input/include/firelight/input/shortcut_action.hpp` | 13 | 2 | 7 | |
| `src/app/emulation/shortcut_actions.hpp` | 12 | 1 | 12 | |
| `qml/components/v2/library/GameView.qml` | 12 | 5 | 3 | |
| `src/gui/models/settings_model.hpp` | 11 | 2 | 8 | |
| `src/app/libretro/core.cpp` | 11 | 4 | 4 | |
| `qml/components/settings/SettingsGroup.qml` | 11 | 1 | 11 | |
| `qml/common/RadioButtonGroup.qml` | 11 | 1 | 11 | |
| `qml/common/Option.qml` | 11 | 1 | 11 | |
| `src/gui/qt_game_art_proxy.hpp` | 10 | 4 | 3 | |
| `src/app/libretro/core.hpp` | 10 | 4 | 3 | |
| `src/app/library/gui/entry_list_model.hpp` | 10 | 3 | 4 | |
| `qml/shop/ShopGridItemDelegate.qml` | 10 | 1 | 10 | |
| `qml/components/v2/library/LibraryNavigationMenu.qml` | 10 | 1 | 10 | |
| `qml/components/settings/SettingsPage.qml` | 10 | 1 | 10 | |
| `qml/components/FirelightDialog.qml` | 10 | 1 | 10 | |
| `qml/common/FirelightButton.qml` | 10 | 1 | 10 | |
| `src/app/netplay/netplay_service.hpp` | 9 | 4 | 3 | |
| `src/app/metadata/metadata_service.hpp` | 9 | 2 | 7 | |
| `src/app/libretro/core_configuration.cpp` | 9 | 1 | 9 | |
| `src/app/emulator_item_renderer.cpp` | 9 | 3 | 4 | |
| `qml/components/v2/titlebar/TitleBar.qml` | 9 | 1 | 9 | |
| `qml/components/v2/LeftNavigationBar2.qml` | 9 | 1 | 9 | |
| `qml/components/FLSuspendPointCard.qml` | 9 | 1 | 9 | |
| `qml/Theme.qml` | 9 | 2 | 5 | |
| `qml/GeneralSettings.qml` | 9 | 1 | 9 | |
| `libs/firelight/settings/include/firelight/settings/settings_index.hpp` | 9 | 2 | 6 | |
| `libs/firelight/settings/include/firelight/settings/setting_definition.hpp` | 9 | 4 | 3 | |
| `libs/firelight/library/src/firelight/library/library_scanner2.hpp` | 9 | 3 | 4 | |
| `src/app/netplay/direct_lobby_backend.hpp` | 8 | 1 | 8 | |
| `qml/components/v2/buttons/FLIconButton.qml` | 8 | 2 | 6 | |
| `qml/components/EmulationSettingsSurface.qml` | 8 | 1 | 8 | |
| `qml/AppearanceSettings.qml` | 8 | 1 | 8 | |
| `libs/firelight/library/include/firelight/library/folder_info.hpp` | 8 | 3 | 3 | |
| `libs/firelight/input/tests/input_service_impl_test.cpp` | 8 | 1 | 8 | |
| `libs/firelight/input/include/firelight/input/input_service.hpp` | 8 | 2 | 4 | |
| `libs/firelight/achievements/src/rcheevos/ra_client.cpp` | 8 | 2 | 6 | |
| `src/gui/models/settings_model.cpp` | 7 | 3 | 3 | |
| `src/gui/models/activity_buckets_list_model.cpp` | 7 | 1 | 7 | |
| `src/cli/launch_config.hpp` | 7 | 1 | 7 | |
| `src/app/libretro/core_registry.cpp` | 7 | 3 | 3 | |
| `src/app/library/gui/entry_list_model.cpp` | 7 | 3 | 3 | |
| `src/app/audio/audio_manager.cpp` | 7 | 2 | 5 | |
| `qml/components/settings/ComboBoxSettingItem.qml` | 7 | 1 | 7 | |
| `qml/common/ToggleOption.qml` | 7 | 1 | 7 | |
| `libs/firelight/settings/src/settings_catalog.cpp` | 7 | 3 | 3 | |
| `libs/firelight/netplay/tests/rtc_transport_test.cpp` | 7 | 1 | 7 | |
| `libs/firelight/netplay/include/firelight/netplay/stream_packets.hpp` | 7 | 2 | 4 | |
| `libs/firelight/media/include/firelight/media/stream_encoder.hpp` | 7 | 2 | 5 | |
| `libs/firelight/media/include/firelight/media/clip_recorder.hpp` | 7 | 1 | 7 | |
| `libs/firelight/media/include/firelight/media/clip_muxer.hpp` | 7 | 1 | 7 | |
| `libs/firelight/input/src/firelight/input/sdl_input_service.hpp` | 7 | 1 | 7 | |
| `libs/firelight/input/include/firelight/input/input_frame.hpp` | 7 | 1 | 7 | |
| `src/gui/qt_network_service_proxy.hpp` | 6 | 2 | 4 | |
| `src/gui/models/core_options_model.hpp` | 6 | 2 | 4 | |
| `src/app/libretro/platform_core_defaults.hpp` | 6 | 2 | 4 | |
| `src/app/emulator_vulkan_renderer.cpp` | 6 | 1 | 6 | |
| `qml/components/v2/buttons/FLButton.qml` | 6 | 1 | 6 | |
| `qml/components/library/LibraryEntryRightClickMenu.qml` | 6 | 1 | 6 | |
| `qml/components/Icon.qml` | 6 | 2 | 4 | |
| `qml/components/FLTwoColumnMenu.qml` | 6 | 1 | 6 | |
| `qml/common/RightClickMenu.qml` | 6 | 1 | 6 | |
| `libs/firelight/settings/include/firelight/settings/settings_service.hpp` | 6 | 1 | 6 | |
| `libs/firelight/saves/include/firelight/saves/isave_manager.hpp` | 6 | 1 | 6 | |
| `libs/firelight/media/include/firelight/media/clip_snapshot.hpp` | 6 | 1 | 6 | |
| `libs/firelight/library/src/firelight/library/patch_associator.hpp` | 6 | 1 | 6 | |
| `libs/firelight/library/src/firelight/library/library_ingest_service.hpp` | 6 | 1 | 6 | |
| `libs/firelight/library/include/firelight/library/content_file.hpp` | 6 | 2 | 3 | |
| `libs/firelight/input/include/firelight/input/shortcut_registry.hpp` | 6 | 1 | 6 | |
| `tests/app/netplay/netplay_service_test.cpp` | 5 | 2 | 3 | |
| `qml/components/v2/surfaces/FLPanel.qml` | 5 | 1 | 5 | |
| `qml/components/v2/surfaces/FLListRow.qml` | 5 | 1 | 5 | |
| `qml/components/v2/library/GameListView.qml` | 5 | 1 | 5 | |
| `qml/components/v2/library/GameContextMenu.qml` | 5 | 1 | 5 | |
| `qml/components/library/UpdateFolderDialog.qml` | 5 | 1 | 5 | |
| `qml/components/library/CreateFolderDialog.qml` | 5 | 1 | 5 | |
| `qml/components/FLInputGuideBar.qml` | 5 | 1 | 5 | |
| `libs/firelight/saves/src/firelight/saves/save_manager_impl.hpp` | 5 | 1 | 5 | |
| `libs/firelight/media/src/clip_recorder.cpp` | 5 | 2 | 3 | |
| `libs/firelight/library/src/sqlite_user_library_repository.cpp` | 5 | 2 | 3 | |
| `libs/firelight/library/src/library_scanner2.cpp` | 5 | 2 | 3 | |
| `libs/firelight/library/src/disc_inspector.cpp` | 5 | 2 | 3 | |
| `libs/firelight/input/tests/shortcut_test.cpp` | 5 | 2 | 3 | |
| `libs/firelight/achievements/include/firelight/achievement_service.hpp` | 5 | 1 | 5 | |
| `tests/app/settings/settings_catalog_test.cpp` | 4 | 2 | 2 | |
| `tests/app/library/library_scanner_test.cpp` | 4 | 2 | 2 | |
| `tests/app/gui/shortcuts_model_test.cpp` | 4 | 2 | 2 | |
| `src/gui/shortcuts_model.hpp` | 4 | 2 | 2 | |
| `src/gui/qt_emulation_service_proxy.hpp` | 4 | 1 | 4 | |
| `src/gui/image_utils.hpp` | 4 | 1 | 4 | |
| `src/app/patching/yay_0_codec.cpp` | 4 | 1 | 4 | |
| `src/app/netplay/guest_stream_receiver.hpp` | 4 | 1 | 4 | |
| `src/app/libretro/libretro_dll.cpp` | 4 | 1 | 4 | |
| `src/app/library/gui/playlist_item_model.hpp` | 4 | 1 | 4 | |
| `src/app/input/gui/analog_settings_model.hpp` | 4 | 1 | 4 | |
| `src/app/emulation/emulation_service.hpp` | 4 | 1 | 4 | |
| `qml/library/GameGridItemDelegate.qml` | 4 | 1 | 4 | |
| `qml/components/v2/library/GameArtPickerDialog.qml` | 4 | 2 | 2 | |
| `qml/components/v2/buttons/FLTabBar.qml` | 4 | 1 | 4 | |
| `qml/components/v2/buttons/FLSegmentedControl.qml` | 4 | 1 | 4 | |
| `qml/components/settings/BaseSettingItem.qml` | 4 | 2 | 2 | |
| `qml/components/HelpArticle.qml` | 4 | 1 | 4 | |
| `qml/components/FLIcon.qml` | 4 | 1 | 4 | |
| `qml/MaterialSymbols.qml` | 4 | 1 | 4 | |
| `libs/firelight/library/include/firelight/library/disc_member.hpp` | 4 | 1 | 4 | |
| `libs/firelight/input/include/firelight/input/shortcut_engine.hpp` | 4 | 1 | 4 | |
| `libs/firelight/cheats/include/firelight/cheats/cheat.hpp` | 4 | 1 | 4 | |
| `tests/app/library/sqlite_user_library_test.cpp` | 3 | 1 | 3 | |
| `tests/app/library/smart_folder_test.cpp` | 3 | 1 | 3 | |
| `src/gui/qt_settings_catalog_proxy.hpp` | 3 | 1 | 3 | |
| `src/gui/qt_save_manager_proxy.hpp` | 3 | 1 | 3 | |
| `src/gui/models/settings_search_model.hpp` | 3 | 1 | 3 | |
| `src/gui/models/search_results_list_model.cpp` | 3 | 1 | 3 | |
| `src/gui/filesystem_utils.hpp` | 3 | 1 | 3 | |
| `src/cli/cli_app.cpp` | 3 | 1 | 3 | |
| `src/app/media/gui/capture_list_model.hpp` | 3 | 1 | 3 | |
| `src/app/input/gui/analog_settings_model.cpp` | 3 | 1 | 3 | |
| `src/app/emulator_item_renderer.hpp` | 3 | 1 | 3 | |
| `src/app/emulation/emulation_service.cpp` | 3 | 1 | 3 | |
| `src/app/emulation/emulation_context.hpp` | 3 | 1 | 3 | |
| `qml/screens/HelpScreen.qml` | 3 | 1 | 3 | |
| `qml/platforms/GameBoySettings.qml` | 3 | 1 | 3 | |
| `qml/platforms/GameBoyColorSettings.qml` | 3 | 1 | 3 | |
| `qml/controllers/ProfileManagementPage.qml` | 3 | 1 | 3 | |
| `qml/components/v2/library/GameTile.qml` | 3 | 1 | 3 | |
| `qml/components/v2/MainNavigationMenuItem.qml` | 3 | 1 | 3 | |
| `qml/components/settings/TextSettingItem.qml` | 3 | 1 | 3 | |
| `qml/components/settings/StepperSettingItem.qml` | 3 | 1 | 3 | |
| `qml/components/settings/SettingsSection.qml` | 3 | 1 | 3 | |
| `qml/components/settings/SegmentedSettingItem.qml` | 3 | 1 | 3 | |
| `qml/components/settings/RadioSettingItem.qml` | 3 | 1 | 3 | |
| `qml/components/settings/MultiSelectSettingItem.qml` | 3 | 1 | 3 | |
| `qml/components/settings/KeyBindingSettingItem.qml` | 3 | 1 | 3 | |
| `qml/components/settings/FilePathSettingItem.qml` | 3 | 1 | 3 | |
| `qml/components/settings/ColorSettingItem.qml` | 3 | 1 | 3 | |
| `qml/components/CoreSelector.qml` | 3 | 1 | 3 | |
| `qml/Router.qml` | 3 | 1 | 3 | |
| `libs/firelight/settings/src/sqlite_core_option_repository.cpp` | 3 | 1 | 3 | |
| `libs/firelight/platforms/include/firelight/platforms/platform_service.hpp` | 3 | 1 | 3 | |
| `libs/firelight/netplay/include/firelight/netplay/sync_strategy.hpp` | 3 | 1 | 3 | |
| `libs/firelight/netplay/include/firelight/netplay/signal_chunker.hpp` | 3 | 1 | 3 | |
| `libs/firelight/netplay/include/firelight/netplay/session_descriptor.hpp` | 3 | 1 | 3 | |
| `libs/firelight/media/include/firelight/media/game_capture.hpp` | 3 | 1 | 3 | |
| `libs/firelight/media/include/firelight/media/clip_sink.hpp` | 3 | 1 | 3 | |
| `libs/firelight/library/src/firelight/library/content_identifier.hpp` | 3 | 1 | 3 | |
| `libs/firelight/library/include/firelight/library/smart_folder.hpp` | 3 | 1 | 3 | |
| `libs/firelight/input/src/sdl_controller.cpp` | 3 | 1 | 3 | |
| `libs/firelight/input/include/firelight/input/keyboard_keycodes.hpp` | 3 | 1 | 3 | |
| `libs/firelight/input/include/firelight/input/device_identifier.hpp` | 3 | 1 | 3 | |
| `libs/firelight/input/include/firelight/input/binding.hpp` | 3 | 1 | 3 | |
| `libs/firelight/cheats/tests/fake_ram_core.hpp` | 3 | 1 | 3 | |
| `libs/firelight/achievements/src/rcheevos/rcheevos_offline_client.cpp` | 3 | 1 | 3 | |
| `tests/qml_main.cpp` | 2 | 1 | 2 | |
| `tests/app/settings/sqlite_core_option_repository_test.cpp` | 2 | 1 | 2 | |
| `tests/app/settings/settings_index_test.cpp` | 2 | 1 | 2 | |
| `tests/app/metadata/metadata_service_test.cpp` | 2 | 1 | 2 | |
| `tests/app/library/entry_resolver_test.cpp` | 2 | 1 | 2 | |
| `tests/app/library/entry_list_model_test.cpp` | 2 | 1 | 2 | |
| `tests/app/library/content_identifier_test.cpp` | 2 | 1 | 2 | |
| `tests/app/library/content_extensions_test.cpp` | 2 | 1 | 2 | |
| `tests/app/gui/settings_model_test.cpp` | 2 | 1 | 2 | |
| `tests/app/emulation/shortcut_actions_test.cpp` | 2 | 1 | 2 | |
| `tests/app/emulation/fake_input_service.hpp` | 2 | 1 | 2 | |
| `tests/app/emulation/fake_core.hpp` | 2 | 1 | 2 | |
| `tests/app/emulation/core_registry_test.cpp` | 2 | 1 | 2 | |
| `src/gui/qt_save_manager_proxy.cpp` | 2 | 1 | 2 | |
| `src/gui/qt_network_service_proxy.cpp` | 2 | 1 | 2 | |
| `src/gui/models/setting_binding.hpp` | 2 | 1 | 2 | |
| `src/cli/scan_command.cpp` | 2 | 1 | 2 | |
| `src/cli/login_command.cpp` | 2 | 1 | 2 | |
| `src/cli/data_dirs.hpp` | 2 | 1 | 2 | |
| `src/cli/console.cpp` | 2 | 1 | 2 | |
| `src/app/netplay/netplay_service.cpp` | 2 | 1 | 2 | |
| `src/app/metadata/metadata_service.cpp` | 2 | 1 | 2 | |
| `src/app/metadata/cpr_http_client.cpp` | 2 | 1 | 2 | |
| `src/app/input/gui/controller_list_model.cpp` | 2 | 1 | 2 | |
| `src/app/emulation/game_loader.cpp` | 2 | 1 | 2 | |
| `qml/screens/EmulatorScreen.qml` | 2 | 1 | 2 | |
| `qml/platforms/PlatformSettingsPage.qml` | 2 | 1 | 2 | |
| `qml/components/v2/surfaces/FLDivider.qml` | 2 | 1 | 2 | |
| `qml/components/v2/library/GameDetailPanel.qml` | 2 | 1 | 2 | |
| `qml/components/v2/library/AddToFolderDialog.qml` | 2 | 1 | 2 | |
| `qml/components/v2/inputs/FLToggle.qml` | 2 | 1 | 2 | |
| `qml/components/v2/inputs/FLTextField.qml` | 2 | 1 | 2 | |
| `qml/components/v2/inputs/FLSlider.qml` | 2 | 1 | 2 | |
| `qml/components/v2/inputs/FLSearchField.qml` | 2 | 1 | 2 | |
| `qml/components/v2/inputs/FLComboBox.qml` | 2 | 1 | 2 | |
| `qml/components/v2/RouteOverlay.qml` | 2 | 1 | 2 | |
| `qml/components/settings/SliderSettingItem.qml` | 2 | 1 | 2 | |
| `qml/components/library/SmartFolderDialog.qml` | 2 | 1 | 2 | |
| `qml/components/FLUserBackground.qml` | 2 | 1 | 2 | |
| `libs/firelight/saves/include/firelight/saves/suspend_point_metadata.hpp` | 2 | 1 | 2 | |
| `libs/firelight/platforms/src/platform_service.cpp` | 2 | 1 | 2 | |
| `libs/firelight/netplay/src/rtc_transport.cpp` | 2 | 1 | 2 | |
| `libs/firelight/netplay/include/firelight/netplay/transport.hpp` | 2 | 1 | 2 | |
| `libs/firelight/netplay/include/firelight/netplay/lobby_backend.hpp` | 2 | 1 | 2 | |
| `libs/firelight/media/src/media_service.cpp` | 2 | 1 | 2 | |
| `libs/firelight/media/src/clip_muxer.cpp` | 2 | 1 | 2 | |
| `libs/firelight/media/include/firelight/media/stream_decoder.hpp` | 2 | 1 | 2 | |
| `libs/firelight/library/src/firelight/library/disc_inspector.hpp` | 2 | 1 | 2 | |
| `libs/firelight/library/src/firelight/library/archive_reader.hpp` | 2 | 1 | 2 | |
| `libs/firelight/library/src/content_hasher.cpp` | 2 | 1 | 2 | |
| `libs/firelight/library/include/firelight/library/content_directory.hpp` | 2 | 1 | 2 | |
| `libs/firelight/input/src/shortcut_engine.cpp` | 2 | 1 | 2 | |
| `libs/firelight/input/src/sdl_input_service.cpp` | 2 | 1 | 2 | |
| `libs/firelight/input/include/firelight/input/input_source.hpp` | 2 | 1 | 2 | |
| `libs/firelight/input/include/firelight/input/gamepad_profile.hpp` | 2 | 1 | 2 | |
| `libs/firelight/cheats/src/cheat_engine.cpp` | 2 | 1 | 2 | |
| `libs/firelight/audio/include/firelight/audio/audio_resampler.hpp` | 2 | 1 | 2 | |
| `libs/firelight/achievements/src/rcheevos/award_achievement_response.hpp` | 2 | 1 | 2 | |

## Biggest block in each of the top files

- `libs/firelight/achievements/src/rcheevos/patch_response.hpp:160` — 637 lines, starts: `// "{\"Success\":true,\"PatchData\":{\"ID\":228,\"Title\":\"Supe`
- `qml/components/FLGameDetailsPanel.qml:197` — 495 lines, starts: `// Pane {`
- `tests/app/platforms/platform_service_test.cpp:183` — 475 lines, starts: `// TEST_F(PlatformServiceTest, PlatformGameboyIsCorrect) {`
- `qml/components/QuickMenu.qml:608` — 391 lines, starts: `// Pane {`
- `qml/Main4.qml:264` — 373 lines, starts: `// SplitView {`
- `src/app/libretro/virtual_filesystem.hpp:30` — 164 lines, starts: `// struct retro_vfs_file_handle;`
- `qml/pages/StoreContent.qml:346` — 153 lines, starts: `// Item {`
- `qml/RewindMenu.qml:191` — 142 lines, starts: `// contentItem: ListView {`
- `qml/pages/NewEmulatorPage.qml:465` — 107 lines, starts: `// FrameAnimation {`
- `tests/app/rcheevos/rcheevos_offline_client_test.cpp:109` — 106 lines, starts: `// TEST_F(RetroAchievementsOfflineClientTest,`
- `qml/components/activity/ActivityPage.qml:97` — 89 lines, starts: `// delegate:`
- `src/gui/models/shop/shop_item_model.cpp:5` — 83 lines, starts: `// m_items.emplace_back(Item{`
- `qml/common/LibraryEntryComboBox.qml:74` — 69 lines, starts: `// indicator: Canvas {`
- `qml/controllers/ControllerTest.qml:88` — 56 lines, starts: `// contentItem: ListView {`
- `qml/components/navigation/FLNavSection.qml:53` — 56 lines, starts: `// delegate: Row {`
- `qml/components/library/EditEntryDialog.qml:74` — 53 lines, starts: `// Pane {`
- `qml/common/FileOption.qml:97` — 49 lines, starts: `// Switch {`
- `qml/common/DirectoryOption.qml:93` — 49 lines, starts: `// Switch {`
- `src/app/input/gui/gamepad_status_item.cpp:14` — 47 lines, starts: `// auto controllerManager = getControllerManager();`
- `qml/components/controllers/ControllerInputMappingView.qml:463` — 46 lines, starts: `// FirelightButton {`
- `qml/home/HomeContentPane.qml:33` — 44 lines, starts: `// Pane {`
- `tests/app/achievements/achievement_service_test.cpp:615` — 41 lines, starts: `// TEST_F(AchievementServiceTest,`
- `tests/app/emulation/emulator_instance_test.cpp:176` — 40 lines, starts: `// TEST_F(EmulatorInstanceTest, RewindSettingChangeUpdatesInstan`
- `qml/settings/DirectorySettings.qml:108` — 36 lines, starts: `// DirectoryOption {`
- `src/app/mods/gui/ModInfoItem.cpp:23` — 34 lines, starts: `// if (!mod) {`
