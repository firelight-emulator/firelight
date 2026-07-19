# QML triage — files unreachable from the app

81 QML files (9,812 lines, **33% of all QML**) are not reachable from `qml/Main4.qml`, computed
transitively through type references plus the nine singletons declared in `CMakeLists.txt`.

Verified complete: no QML file is referenced from C++ (`src/main.cpp:763` is the only entry point,
`engine.loadFromModule("QMLFirelight", "Main4")`), and `routing.js` holds only route patterns, no
component names. Every one of these files is still compiled into the binary — `CMakeLists.txt:135`
is a `file(GLOB …)` ending in a catch-all `qml/**/*.qml`.

**Mark each row keep or delete.** Nothing is deleted until you have.

Deleting requires `cmake --preset debug-win` afterwards: the glob has no `CONFIGURE_DEPENDS`, so a
stale file list breaks the build.

---

## Superseded — 60 files, 7,611 lines

A newer equivalent is live. Named for each.

### The Main3 shell and its parts (14 files, 2,227 lines)

`Main3.qml` was the previous `ApplicationWindow`; everything it uniquely mounted died with it.

| File | Lines | Replaced by | Decision |
|---|---|---|---|
| `qml/Main3.qml` | 672 | `qml/Main4.qml` | |
| `qml/screens/HomeScreen.qml` | 254 | `qml/Main4.qml` | |
| `qml/components/navigation/LeftNavigationBar.qml` | 244 | Main4's inline `navRail` Repeater of `FLIconButton` | |
| `qml/screens/EmulatorScreen.qml` | 379 | `v2/GameplayLayer.qml` | |
| `qml/components/MainContent.qml` | 143 | Main4's inline `navRail` + `RouteView` | |
| `qml/components/v2/LeftNavigationBar2.qml` | 79 | same — a v2 nav bar attempt Main4's rail bypassed | |
| `qml/components/navigation/LeftNavigationItem.qml` | 78 | same | |
| `qml/components/v2/MainNavigationMenuItem.qml` | 73 | `v2/library/LibraryNavigationMenuItem.qml` | |
| `qml/NavMenuButton.qml` | 69 | `v2/buttons/FLIconButton.qml` | |
| `qml/Notification.qml` | 69 | `v2/Toast.qml` | |
| `qml/components/navigation/FLNavItem.qml` | 62 | Main4's rail — was itself `LeftNavigationItem`'s successor; both lost | |
| `qml/components/navigation/FLNavSection.qml` | 55 | same | |
| `qml/home/HomeContentPane.qml` | 40 | `v2/RouteView.qml` | |
| `qml/ActiveFocusHighlight.qml` | 10 | `components/FLFocusHighlight.qml` | |

### Per-platform settings pages (10 files, ~1,380 lines)

**Delete as a block.** Platform settings are now catalog-driven through
`qml/platforms/PlatformEmulationSettingsPage.qml`, routed from `SettingsScreen.qml:485`. All ten call
`emulator_config_manager.getOptionValueForPlatform(...)` — **that object no longer exists anywhere in
`src/` or `libs/`, so these would throw on load.** Seven of the ten render only "There's nothing here
yet." as their entire body.

| File | Lines | Note | Decision |
|---|---|---|---|
| `qml/platforms/GameBoySettings.qml` | 497 | real content, now served generically | |
| `qml/platforms/GameBoyColorSettings.qml` | 180 | real content | |
| `qml/platforms/GbaSettings.qml` | 156 | real content | |
| `qml/platforms/GameGearSettings.qml` | 78 | empty body | |
| `qml/platforms/GenesisSettings.qml` | 78 | empty body | |
| `qml/platforms/MasterSystemSettings.qml` | 78 | empty body | |
| `qml/platforms/NesSettings.qml` | 78 | empty body | |
| `qml/platforms/SnesSettings.qml` | 78 | empty body | |
| `qml/platforms/Nintendo64Settings.qml` | 78 | empty body | |
| `qml/platforms/NintendoDsSettings.qml` | 78 | empty body | |

### Old library UI (12 files, 1,287 lines)

| File | Lines | Replaced by | Decision |
|---|---|---|---|
| `qml/components/library/LibraryPage.qml` | 338 | `v2/library/LibraryPageV2.qml` | |
| `qml/library/GameGridItemDelegate.qml` | 169 | `v2/library/GameTile.qml` + `GameGridView.qml` | |
| `qml/common/SelectLibraryEntryDialog.qml` | 144 | `v2/netplay/NetplayGamePickerDialog.qml` | |
| `qml/components/library/LibraryEntryListDelegate.qml` | 114 | `v2/library/GameListView.qml` | |
| `qml/components/v2/library/LibraryNavigationMenu.qml` | 111 | `LibraryNavigationMenuSection.qml` — its own `objectName` already says so | |
| `qml/common/LibraryEntryComboBox.qml` | 104 | `FLSearchField.qml` / `GameContextMenu` flows | |
| `qml/components/library/LibraryEntryRightClickMenu.qml` | 82 | `v2/library/GameContextMenu.qml` | |
| `qml/components/library/UpdateFolderDialog.qml` | 65 | `CreateFolderDialog.qml` / `SmartFolderDialog.qml` | |
| `qml/components/library/EditEntryDialog.qml` | 64 | `GameArtPickerDialog.qml` + inline rename in `GameContextMenu.qml` | |
| `qml/library/EntrySummaryPage.qml` | 54 | `v2/library/GameDetailPanel.qml` | |
| `qml/components/library/LibraryFolderListDelegate.qml` | 30 | `v2/library/LibraryNavigationMenuItem.qml` | |
| `qml/components/v2/library/GameListHeader.qml` | 12 | `ListViewColumnHeader.qml` — this file is a stub (10 imports, `Item {}`) | |

### Old settings/options primitives (10 files, 756 lines)

Replaced by the catalog-driven `qml/components/settings/*SettingItem.qml` family. Note their
siblings `ToggleOption.qml`, `DirectoryOption.qml`, `MyComboBox.qml` stay **alive** via `qml/settings/`.

| File | Lines | Replaced by | Decision |
|---|---|---|---|
| `qml/components/FLTwoColumnMenu.qml` | 191 | `v2/settings/SettingsScreen.qml` — says so in its own comment | |
| `qml/common/RadioButtonGroup.qml` | 97 | `settings/RadioSettingItem.qml` | |
| `qml/common/FileOption.qml` | 89 | `settings/FilePathSettingItem.qml` | |
| `qml/common/MySlider.qml` | 87 | `settings/SliderSettingItem.qml` | |
| `qml/common/Option.qml` | 74 | `settings/BaseSettingItem.qml` | |
| `qml/common/ComboBoxOption2.qml` | 64 | `settings/ComboBoxSettingItem.qml` | |
| `qml/components/pagelayouts/TwoPaneLayout.qml` | 56 | `v2/settings/SettingsScreen.qml` — **already broken**, names a type whose dir `pagelayouts/TwoPaneMenuItem/` is empty | |
| `qml/common/OptionGroup.qml` | 51 | `settings/SettingsGroup.qml` | |
| `qml/common/ComboBoxOption.qml` | 33 | `settings/ComboBoxSettingItem.qml` | |
| `qml/common/SliderOption.qml` | 14 | `settings/SliderSettingItem.qml` | |

### Everything else superseded (14 files, 1,962 lines)

| File | Lines | Replaced by | Decision |
|---|---|---|---|
| `qml/pages/StoreContent.qml` | 325 | `pages/ShopItemPage.qml` / `FLModShopItemPanel.qml` | |
| `qml/controllers/KeyboardProfilePage.qml` | 492 | `controllers/ControllerProfilePage.qml` — now handles keyboard too | |
| `qml/pages/DiscoverPage.qml` | 219 | `pages/ShopLandingPage.qml` (`/shop`) | |
| `qml/pages/GameDetailsPage.qml` | 162 | `components/FLGameDetailsPanel.qml` (`/library/entries/:entryId`) | |
| `qml/achievements/AchievementListItem.qml` | 148 | `components/achievements/AchievementListButton.qml` | |
| `qml/components/FLInputGuideBar.qml` | 144 | `v2/ButtonBar.qml` | |
| `qml/achievements/AchievementList.qml` | 126 | inline `ListView` + `AchievementListButton` in `QuickMenu.qml` | |
| `qml/components/activity/ActivityPage.qml` | 95 | `v2/activity/ActivityPageV2.qml` (`/activity`) | |
| `qml/common/ImageViewer.qml` | 83 | `FLImageCarousel.qml` / `v2/gallery/CaptureViewer.qml` | |
| `qml/discover/AddedPopup.qml` | 82 | `v2/Toast.qml` | |
| `qml/components/AchievementSetView.qml` | 38 | the achievement-set `ListView` inside `QuickMenu.qml` | |
| `qml/common/DetailsButton.qml` | 21 | `v2/buttons/FLIconButton.qml` | |
| `qml/common/ListViewSectionDelegate.qml` | 16 | `v2/FLSectionHeader.qml` | |
| `qml/common/RightClickMenuSeparator.qml` | 11 | `v2/surfaces/FLDivider.qml` | |

---

## Scratch / experiment — 3 files, 764 lines

| File | Lines | Why | Decision |
|---|---|---|---|
| `qml/components/FLThing.qml` | 417 | Alternate shell prototype; root comment `// required property Component libraryPage` is commented out. Superseded by `RouteView` | |
| `qml/pages/DashboardTesting.qml` | 288 | `GridView` over a hardcoded `ListModel` of badge URLs — a visual sketch with no data source | |
| `qml/controllers/ControllerTest.qml` | 59 | Declares its own `ApplicationWindow` — a standalone harness, cannot be embedded | |

---

## Keep — component library, 9 files

Not dead. This is the `v2` design system, built ahead of adoption. Being migrated onto the live UI
in Phase D of the plan.

| File | Lines | State |
|---|---|---|
| `qml/components/v2/surfaces/FLListRow.qml` | 62 | needs disabled + focus states (Phase C) |
| `qml/components/v2/inputs/FLToggle.qml` | 47 | needs focus/hover/disabled; `color: "white"` → `Theme.onAccent` |
| `qml/components/v2/FLEmptyState.qml` | 43 | finished — 7 adoption sites |
| `qml/components/v2/inputs/FLSlider.qml` | 38 | needs hover/pressed/disabled; `color: "white"` → `Theme.onAccent` |
| `qml/components/v2/FLBadge.qml` | 27 | finished — 5 adoption sites |
| `qml/components/v2/surfaces/FLPanel.qml` | 24 | finished — **~25 adoption sites, the biggest win** |
| `qml/components/v2/surfaces/FLDivider.qml` | 16 | finished — 9 sites; only works inside a Layout |
| `qml/components/v2/surfaces/FLScrollView.qml` | 13 | drop unconditional `ScrollBar.horizontal`, then adopt — 13 sites |
| `qml/components/v2/FLSectionHeader.qml` | 12 | finished — ~14 sites |

### Component deletions you already approved

| File | Lines | Why | Decision |
|---|---|---|---|
| `qml/components/v2/buttons/FLTabBar.qml` | 62 | Overlaps `FLSegmentedControl` (2 refs, better states) | delete |
| `qml/components/v2/FrostedPane.qml` | 64 | Superseded by `FLPanel { variant: "glass" }`; depends on an undeclared global `window.background`, so it only works where a `window` id happens to be in scope | delete |

---

## Keep — features built but never wired, 6 files, 1,022 lines

**These are capability regressions from the Main3 → Main4 port, not cleanup.** Verified: `LobbyChip`
and `ReadyCheckToast` are referenced *only* from the dead `Main3.qml`; `ScanPopup` and
`NewUserScreen` have no references at all.

| File | Lines | What is missing from the running app | Decision |
|---|---|---|---|
| `qml/screens/NewUserScreen.qml` | 442 | First-run onboarding wizard (welcome → directories → …), fully built, never mounted. Sole live consumer of `DirectoryOption.qml`/`ToggleOption.qml` outside `qml/settings/` | |
| `qml/library/ManageSavefilesDialog.qml` | 231 | Save-file management. Pick this **or** `ManageSaveDataDialog` before deleting the other | |
| `qml/components/library/ManageSaveDataDialog.qml` | 130 | Newer take on the same feature (uses `SaveDataInformation`); also unrouted | |
| `qml/components/v2/netplay/ReadyCheckToast.qml` | 102 | App-wide ready prompt when the host starts a game. Guests currently get nothing | |
| `qml/library/ScanPopup.qml` | 71 | Library-scan progress. **There is currently no scan feedback in the live UI at all** | |
| `qml/components/v2/netplay/LobbyChip.qml` | 46 | The "you are in a lobby" chip while browsing elsewhere. On this branch the lobby is invisible outside `/netplay` | |

The two netplay ones are the cheapest to re-wire and the most relevant to the branch you are on.

---

## Unclear — 1 file

| File | Lines | Why | Decision |
|---|---|---|---|
| `qml/components/FLBreadcrumbs.qml` | 7 | Empty stub — `FocusScope { id: root }` and nothing else. Intent is obvious from the name, but nothing was built. Delete or finish | |
