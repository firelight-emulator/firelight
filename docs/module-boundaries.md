# Module boundaries: information leakage audit

What each module lets other modules see, where that visibility is unintentional, and what to do
about it.

**Method.** Read from source on `claude/platform-module-leakage-a7osls` at `626f296`, plus CMake
target graphs and include-resolution checks. Nothing here was reproduced at runtime — the build
environment is Windows/MSYS2 and this pass was static. Every claim cites `file:line` so it can be
checked in one jump. Where a finding is an inference rather than a direct reading, it says so.

**How to read it.** Findings are numbered `L1`…`L22` and grouped by boundary. Each carries a tag:

- **[structural]** — the boundary is not enforceable; the current state is luck
- **[leak]** — one module's internals are visible to, or depended on by, another right now
- **[drift]** — one fact has two copies, and they have already diverged
- **[cleanup]** — dead, redundant, or write-only

---

## Summary

| # | Finding | Tag | Where |
|---|---|---|---|
| L1 | 8 domain libs' private `src/` dirs are on the app's include path | structural | `CMakeLists.txt:397,509,765` |
| L2 | 19 lib-private headers are included from `src/` and `tests/` | leak | various |
| L3 | `library` and `discord` declare include paths into `src/app` | structural | two `CMakeLists.txt` |
| L4 | `RAClient` lives in the achievements lib but is compiled into `firelight_lib` | structural | `CMakeLists.txt:286` |
| L5 | `platforms` → `input` in headers, `input` → `platforms` in CMake | structural | see §2 |
| L6 | `Platform` is returned whole; every caller re-derives its own answer | leak | `platform_service.hpp` |
| L7 | "Inputs for this controller type" hand-written 3× in one file | leak | `input_mappings_model.cpp` |
| L8 | `listPlatforms() × controllerTypes` hand-written 3× more | leak | `sqlite_controller_repository.cpp` |
| L9 | `SqliteControllerRepository` takes the concrete `PlatformService` | leak | `sqlite_controller_repository.hpp:23` |
| L10 | `ControllerType::deviceClass` is write-only and duplicates `CoreDeviceVariant` | cleanup | `controller_type.hpp:21` |
| L11 | `ControllerType::imageUrl` is missing from its own serializers | drift | `controller_type.hpp:26-36` |
| L12 | Two unrelated integers are both called "controller type" | leak | see §2 |
| L13 | Platform display derivation copied to 7 sites with 4 fallbacks | drift | see §4 |
| L14 | Whole-`Platform` copies to read one string, one of them inside `data()` | leak | `entry_list_model.cpp:273` |
| L15 | `RAClient` (private header, QObject) is a QML context property | leak | `main.cpp:822` |
| L16 | `LibraryScanner2` (private header, QObject) is exposed to QML, unused | cleanup | `main.cpp:853` |
| L17 | `ServiceAccessor` holds 5 concrete classes among 9 interfaces | leak | `service_accessor.hpp:124-137` |
| L18 | 3 locator users are constructed by us, against the locator's stated contract | leak | see §5 |
| L19 | `ControllerType` shredded into 3 index-aligned QML arrays | leak | `platform_list_model.cpp:32-64` |
| L20 | Domain enums cross into QML as bare ints; QML re-derives the labels | leak | `QuickMenu.qml:667` |
| L21 | `FolderInfo::type` is an `int` although `FolderType` exists | leak | `folder_info.hpp:19` |
| L22 | File-extension knowledge lives in 3 modules and has drifted 14 ways | drift | see §6 |

Two systemic causes account for most of it, and both are worth fixing before the individual items:

1. **The build does not enforce the public/private split.** Modules carefully separate
   `include/firelight/<name>/` from `src/firelight/<name>/`, and then the root `CMakeLists.txt`
   puts the `src/` dirs on the app's include path anyway (L1). Once that is done, nothing stops
   the next include, and 19 have accumulated (L2).

2. **Services return aggregates instead of answering questions.** `IPlatformService` is the clearest
   case, but the pattern repeats: hand the caller the whole struct and let it work out what it
   wanted. Every call site then owns a copy of the derivation, and the copies drift (L6, L13, L22).

---

## 1. The build layer — boundaries that are not enforced

### L1 — 8 libs' private `src/` directories are on the app's include path **[structural]**

Every domain lib declares its sources private:

```cmake
target_include_directories(firelight_library
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src      # private
        PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/include) # public
```

and the root build then re-opens them:

- `CMakeLists.txt:397` — `firelight_lib` gets `achievements/src`, `achievements/src/rcheevos`,
  `input/src`, `saves/src`, `library/src`
- `CMakeLists.txt:509` — `firelight` gets 8: `achievements`, `activity`, `settings`, `mods`,
  `input`, `library`, `saves`, `discord`
- `CMakeLists.txt:765` — `fl_test` gets 7 plus `activity/include`

The `PRIVATE` markers in the lib files are therefore decorative for anything in `src/`. This is the
enabling condition for L2 and L15, and it is the single highest-leverage fix in this document.

### L2 — 19 lib-private headers are included from outside their lib **[leak]**

| private header | non-test consumers |
|---|---|
| `firelight/input/keyboard_input_handler.hpp` | `src/gui/shortcuts_model.cpp`, `src/app/input/gui/input_mappings_model.cpp` |
| `firelight/library/content_identifier.hpp` | `src/cli/rom_launch.cpp` |
| `firelight/library/disc_set_service.hpp` | `src/gui/qt_disc_set_proxy.cpp`, `src/cli/scan_command.cpp` |
| `firelight/library/library_ingest_service.hpp` | `src/cli/scan_command.cpp` |
| `firelight/library/library_scanner2.hpp` | `src/cli/scan_command.cpp` |
| `firelight/library/sqlite_user_library.hpp` | `src/cli/scan_command.cpp` |
| `firelight/activity/sqlite_activity_log.hpp` | *(tests only)* |
| `firelight/discord/discord_manager_impl.hpp` | *(main.cpp only)* |
| `firelight/input/sdl_input_service.hpp` | *(main.cpp only)* |
| `firelight/input/sqlite_controller_repository.hpp` | *(main.cpp only)* |
| `firelight/library/archive_reader.hpp` | *(tests only)* |
| `firelight/library/content_discoverer.hpp` | *(tests only)* |
| `firelight/library/content_extensions.hpp` | *(tests only)* |
| `firelight/library/content_hasher.hpp` | *(tests only)* |
| `firelight/library/disc_inspector.hpp` | *(tests only)* |
| `firelight/mods/sqlite_mod_repository.hpp` | *(main.cpp only)* |
| `firelight/saves/save_manager_impl.hpp` | *(tests only)* |
| `firelight/saves/sqlite_save_database.hpp` | *(tests only)* |
| `firelight/settings/sqlite_settings_repository.hpp` | *(tests only)* |

Three tiers, and they want different fixes:

- **main.cpp only** — defensible. It is the composition root and has to name concrete impls. The
  clean version is a factory in the lib's public header (`makeSqliteControllerRepository(...)
  -> std::unique_ptr<IControllerRepository>`), which lets the `src/` path come off the include list
  entirely.
- **tests only** — a signal that the public API cannot exercise the lib. Note these are the
  *app-level* `fl_test` target reaching in, not each lib's own `firelight_<name>_test`, which is
  already scoped correctly. Either widen the public API or move the test into the lib's own target.
- **production code outside main.cpp** — the real violations, 6 headers across 5 files. `scan_command.cpp`
  alone reaches into four of `firelight_library`'s private headers.

### L3 — `library` and `discord` declare include paths into `src/app` **[structural]**

`libs/firelight/library/CMakeLists.txt:39-40` and `libs/firelight/discord/CMakeLists.txt:14` add
`${CMAKE_SOURCE_DIR}/src/app` (and, for library, `${CMAKE_SOURCE_DIR}/src`) to the lib's own include
path. That inverts the layering: the app depends on the libs, and these libs declare a path back into
the app.

Checked every `#include` in both modules — **nothing currently resolves through those paths.** So
this is a latent hole rather than an active violation: the day someone adds
`#include "emulation/emulation_service.hpp"` inside `firelight_library`, it will compile. Delete the
three lines.

### L4 — `RAClient` is in the achievements lib's tree but compiled into `firelight_lib` **[structural]**

`libs/firelight/achievements/CMakeLists.txt` lists four sources; `ra_client.cpp` is not among them.
It is compiled by the root build instead (`CMakeLists.txt:286`). So the file sits in
`libs/firelight/achievements/src/rcheevos/` while belonging to the app target.

No ODR problem (it is compiled exactly once), but the directory layout lies about ownership, and it
is the reason `RAClient` can be a Qt `QObject` with 13 `Q_PROPERTY`/`Q_INVOKABLE`
(`ra_client.hpp:29-35`) while the rest of `firelight_achievements` stays Qt-light. Either move the
file to `src/app/achievements/` or move it into the lib and strip the Qt.

---

## 2. The platforms module

The module's public surface is four methods, and the two that matter hand back `Platform` by value.
`Platform` is a public-field aggregate with exactly one behaviour (`discordImageOrSlug()`,
`platform.hpp:24`). So the module answers no questions — it ships its data model and lets each caller
re-derive the answer. That is the leak, and it shows up six ways.

### L5 — the dependency runs backwards in the headers **[structural]**

`ControllerType` holds an `input::GamepadInputClass` (`controller_type.hpp:21`) and
`PlatformInputDescriptor` holds an `input::GamepadInput` (`controller_input_descriptor.hpp:12`). So
the platforms module's public data model is written in the input module's vocabulary.

CMake says the opposite: `libs/firelight/input/CMakeLists.txt:37` is `PUBLIC firelight_platforms`,
and `firelight_platforms` links nothing but nlohmann/spdlog. It compiles only because
`gamepad_input.hpp` lives in the root `include/` tree rather than in
`libs/firelight/input/include/`, so `firelight_platforms` picks it up through
`PUBLIC ${CMAKE_SOURCE_DIR}/include` (`platforms/CMakeLists.txt:15`) without ever linking input.

A cycle papered over by header placement. It is invisible to the build graph, so nothing will warn
when it tightens.

### L6 — `getPlatform`/`listPlatforms` return the whole aggregate **[leak]**

`platform_service.hpp:20,26`. Both return by value, and `getPlatform` is an O(n) linear scan
(`platform_service.cpp:675-683`). Every consumer in the codebase wants one or two fields.

For scale: SNES carries 3 controller types and 18 input descriptors, Genesis 3 and 19 — each
descriptor holding a `std::string`. Reading `.slug` deep-copies all of it.

### L7 — "inputs for this controller type" is hand-written three times **[leak]**

`input_mappings_model.cpp:132`, `:185`, `:260` — byte-identical, down to the `spdlog::warn` strings
at `:147`, `:200`, `:275`:

```cpp
auto platform = getPlatformService()->getPlatform(m_platformId);
std::vector<platforms::PlatformInputDescriptor> inputs;
for (const auto &type : platform->controllerTypes) {
  if (type.id == m_controllerTypeId) { inputs = type.inputs; break; }
}
```

This is `inputsFor(platformId, controllerTypeId)` written longhand because the service will not
answer it.

### L8 — the platform × controller-type cross-product is hand-written three more times **[leak]**

`sqlite_controller_repository.cpp:234`, `:412`, `:529` — `loadProfileContents`, `cloneProfile`,
`exportProfile`. All three want the same thing: every `(platformId, controllerTypeId)` pair that can
hold a mapping. None of them wants a `Platform`.

### L9 — the repository depends on the concrete `PlatformService` **[leak]**

`sqlite_controller_repository.hpp:23,52` take `platforms::PlatformService&`. It is the only place in
the codebase that binds to the concrete class rather than `IPlatformService` — `discord`, `library`,
`cli`, `emulation` and the GUI models all take the interface.

Consequence: the repository cannot be tested against a fake platform set, which is why
`controller_repository_test.cpp:34` has to construct a real `PlatformService` and inherit all 25
platforms.

### L10 — `ControllerType::deviceClass` is write-only **[cleanup]**

Set on six entries (`platform_service.cpp:157,212,221,346,381,391`). Read in exactly one place: the
test (`platform_service_test.cpp:57`).

The runtime's device class comes from `CoreRegistry::CoreDeviceVariant::deviceClass`
(`core_registry.hpp:91`), which is what `emulator_instance.cpp:186,322` actually read. Two parallel
models of "what input device classes exist here", one of them dead. The header already concedes the
redundancy at `controller_type.hpp:13` ("`id` mirrors `deviceClass`").

### L11 — `imageUrl` is absent from its own serializers **[drift]**

`controller_type.hpp:26-36` — neither `from_json` nor `to_json` mentions `imageUrl`, though
`platform_service_test.cpp:26` asserts it round-trips.

The assertion never fires: `assertPlatformsEqual` is dead, all 13 call sites commented out
(`platform_service_test.cpp:244`–`:652`), and the round-trip test that does run (`:193`) never
populates `controllerTypes`. Execute the plan stated at `:191` — "platform data could later be
externalized to a file" — and every controller image silently disappears.

### L12 — two unrelated integers are both "controller type" **[leak]**

`platforms::ControllerType` is a console's controller (Zapper, 6-button pad) and is what
`InputMapping::getControllerType()` (`input_mapping.hpp:32`) keys on.

But `IControllerRepository::setPlatformPreferredType(int platformId, int gamepadType)`
(`controller_repository.hpp:55`), `PlatformInputPreferences::controllerTypeOptions()`
(`platform_input_preferences.cpp:32`) and `cli::controllerTypeNames()` (`launch_config.cpp:121`) all
mean *physical hardware* — DualSense, Switch Pro. Adjacent APIs, same word, unrelated integers, no
type to stop you passing one where the other belongs.

---

## 3. Domain libs ↔ app layer

### L15 — `RAClient` is a QML context property **[leak]**

`main.cpp:822`: `setContextProperty("achievement_manager", &raClient)`. `RAClient` is declared in
`libs/firelight/achievements/src/rcheevos/ra_client.hpp` — a private header (see L4) — and is a
`QObject` with 13 `Q_PROPERTY`/`Q_INVOKABLE`.

CLAUDE.md states the rule plainly: *"C++ services are not exposed to QML directly."* Nine QML files
use it, reaching 12 members: `avatarUrl`, `challengeIndicatorsEnabled`, `defaultToHardcore`,
`displayName`, `inHardcoreMode`, `logInUserWithPassword`, `logInUserWithToken`, `loggedIn`,
`logout`, `points`, `progressNotificationsEnabled`, `unlockNotificationsEnabled`.

`QtAchievementServiceProxy` exists alongside it, so there are two doors into achievements from QML —
and both carry `loggedIn`. `QuickMenu.qml:204` reads `AchievementService.loggedIn`;
`FLSuspendPointCard.qml:71`, `RetroAchievementSettings.qml:25` and `RetroAchievementsAccountPane.qml`
(five sites) read `achievement_manager.loggedIn`. Same fact, two sources, no guarantee they agree.

### L16 — `LibraryScanner2` is exposed to QML and never used **[cleanup]**

`main.cpp:853`: `setContextProperty("LibraryScanner", &libScanner2)`. `LibraryScanner2` is another
private-header `QObject` (`library_scanner2.hpp:33-35`, `Q_PROPERTY(bool scanning ...)`). Grepping
all of `qml/` finds **zero** references to `LibraryScanner`. Delete the registration; that also
removes one reason for `library/src` to be on the app include path.

### L17 — `ServiceAccessor` mixes concrete classes and interfaces **[leak]**

`service_accessor.hpp:124-137`. Nine slots hold interfaces (`IControllerRepository`,
`ICoreOptionRepository`, `IActivityLog`, `ISaveManager`, `IModRepository`, `IDiscordManager`, …);
five hold concrete classes:

```cpp
static input::InputService         *s_inputService;
static platforms::PlatformService  *s_platformService;      // not IPlatformService
static achievements::AchievementService *s_achievementService;
static library::UserLibraryService *s_libraryService;
static achievements::RAClient      *s_achievementManager;
```

Every QML-constructed model that reaches `getPlatformService()` is therefore bound to the concrete
class even where `IPlatformService` would do. Four interfaces already exist for these; the slots just
do not use them.

### L18 — three locator users are constructed by us **[leak]**

CLAUDE.md scopes the locator narrowly: *"it exists only for objects the QML engine default-constructs
(qmlRegisterType models/items) which cannot receive dependencies via a constructor. Everything we
construct ourselves should take its dependencies via the constructor instead."*

Against that contract:

| class | how it is created | verdict |
|---|---|---|
| `ActivityBucketsListModel` | `main.cpp:836-837`, context property | **violates** — calls `getLibraryService()`, `getPlatformService()` |
| `LibraryFolderListModel` | `main.cpp:852`, context property | **violates** — uses locator getters |
| `QtPlatformServiceProxy` | `main.cpp:849-850`, context property | **dead inheritance** — inherits `ServiceAccessor` *and* takes `IPlatformService*`; never calls a getter |

`ActivityBucketsListModel` also inherits privately (`activity_buckets_list_model.hpp:9`,
`QAbstractListModel, ServiceAccessor` with no access specifier), which is why it does not show up in
a grep for `public ServiceAccessor` — worth knowing if you audit this again.

The right pattern is already in the tree: `EmulationContext`
(`src/app/emulation/emulation_context.hpp`) threads seven services through as explicit interface
pointers with a comment saying exactly why. That is the model to copy.

---

## 4. The C++ ↔ QML boundary

### L19 — `ControllerType` is shredded into index-aligned arrays **[leak]**

`platform_list_model.cpp:32-64` emits four roles from one vector: `controllerTypeNames`,
`controllerTypeIds`, `controllerImages`, `numControllerTypes`. QML reassembles by index
(`ControllerInputMappingView.qml:27-29`):

```qml
const ids = platformMetadataModel ? platformMetadataModel.controllerTypeIds : undefined;
return (idx >= 0 && platformMetadataModel) ? platformMetadataModel.controllerImages[idx] : "";
```

with a four-line comment in the C++ (`platform_list_model.cpp:48-52`) explaining an alignment
contract the type system no longer enforces. `deviceClass` is not exposed at all, so QML cannot tell
a Zapper row from a Retropad row.

### L20 — domain enums cross as bare ints and QML re-derives the labels **[leak]**

`qt_emulation_service_proxy.cpp:173` writes `entry["deviceClass"] = static_cast<int>(...)`, documented
in the header as `deviceClass:int (1=Joypad,2=Mouse,3=LightGun)`
(`qt_emulation_service_proxy.hpp:51-53`). QML then hardcodes the mapping
(`QuickMenu.qml:667`):

```qml
text: optionButton.modelData.name + (optionButton.modelData.deviceClass === 3 ? "  (Light Gun)"
                                   : optionButton.modelData.deviceClass === 2 ? "  (Mouse)" : "")
```

So `input::GamepadInputClass` (`gamepad_input.hpp:71`) exists in three places: the enum, a comment,
and a QML conditional.

**The fix is already in the codebase.** `src/gui/settings_level_shim.hpp` solves exactly this for
`settings::SettingsLevel` — a `Q_NAMESPACE` shim on the app side, with a `static_assert` that the
values have not drifted, and a comment explaining that the lib is deliberately Qt-free. Apply the
same shim to `GamepadInputClass` and `FolderType`.

### L21 — `FolderInfo::type` is an `int` although `FolderType` exists **[leak]**

`folder_info.hpp:8-11` declares `enum class FolderType { Manual = 0, Smart = 1 }`, and then
`:19` declares the field as `int type = static_cast<int>(FolderType::Manual)`.

Cost: four `static_cast<int>` comparisons in the app
(`library_entry_sort_filter_model.cpp:180`, `entry_list_model.cpp:632`,
`playlist_item_model.cpp:207,271,317`) and a bare `=== 1` in three QML files
(`LibrarySidebar.qml:224,272`, `CollectionTile.qml:21`, `CollectionListItem.qml:16`). One-line fix at
the declaration, plus the L20 shim for the QML side.

### The wider untyped surface

Thirty-nine files under `src/gui` traffic in `QVariantMap`/`QVariantList` whose shape is documented only
in a comment. `controllerVariantsForPort` (L20) is the example, but `storedAssets`, `searchResults`,
`getSaveFiles`, `lobbyMembers`, `presetOptions`, `categories`, `options` and
`ActivityBucketsListModel::get(int)` are the same shape of problem. Not all of these are worth
typing, but the ones carrying an enum are.

---

## 5. Cross-module vocabulary

### L13 — platform display derivation copied to 7 sites with 4 fallbacks **[drift]**

| site | derives | fallback |
|---|---|---|
| `qt_platform_service_proxy.cpp:13` | `"qrc:/icons/" + slug` | `""` |
| `platform_list_model.cpp:58` | raw slug as `iconName` | — |
| `entry_list_model.cpp:273` | raw slug | `{}` |
| `library_entry_item.cpp:26` | raw slug as `platformIconName` | keeps stale value |
| `game_activity_list_model.cpp:34` | name + slug | `"unknown"` / `"Unknown Platform"` |
| `activity_buckets_list_model.cpp:197,215` | name | `"Unknown"` |
| `ModInfoItem.cpp:36` | name | `"Unknown"` |

Plus `FLPlatformIcon.qml:16` doing the missing-icon fallback a third way (`"qrc:/icons/unknown"`), and
the `qrc:/icons/` template appearing again in `LibrarySidebar.qml:175` and `CollectionDialog.qml:352`.

Incidentally `LibraryEntryItem::platformIconName` (`library_entry_item.hpp:15`) is entirely dead:
computed from the slug, emitted, copied into a QML object literal at `GameListView.qml:62` and
`qml/v3/library/GameListView.qml:59`, and read by nothing.

### L14 — whole-aggregate copies on hot paths **[leak]**

`entry_list_model.cpp:273-279` calls `getPlatform()` **inside `data()`** — a full `Platform`
deep-copy per row, per role query, per repaint, to produce a slug string.

`EmulationService` stores `platforms::Platform m_currentPlatform` by value
(`emulation_service.hpp:150`), returns `std::optional<Platform>` (`:129`), and `GameLoadResult`
carries one too (`game_loader.hpp:34`) — all so `qt_emulation_service_proxy.cpp:92-99` can read
`->name`.

### L22 — file-extension knowledge lives in three modules and has drifted **[drift]**

Three owners:

1. `platforms` — `Platform::fileAssociations`, 39 extensions
2. `library` — `DISC_EXTENSIONS` and friends in the private `content_extensions.hpp`, 10 more
3. `app` — per-core `valid_extensions` in `core_registry.cpp:64-95`, 56 extensions

Diffing (1) against (3):

**12 extensions a bundled core will open that no platform claims**, so the scanner never offers them:
`68k`, `ccd`, `dsi`, `elf`, `ids`, `neo`, `prx`, `sgb`, `sgd`, `toc`, `u1`, `vboy`. Concretely: mGBA
accepts `.sgb`, melonDS accepts `.dsi`/`.ids`, PPSSPP accepts `.elf`/`.prx`, Beetle VB accepts
`.vboy`, Mupen64Plus accepts `.u1`, Geolith accepts `.neo` — drop any of those in a watched folder
and nothing happens.

**2 the platform claims that no bundled core opens**: `32x` and `gcn`. Sega 32X and GameCube have
`fileAssociations` but no entry in `core_registry.cpp`, so the scanner catalogues the file and the
launch then fails.

Some of these are intentional (`ccd`/`toc` are disc sheets handled by (2); 32X/GameCube are
presumably planned). The point is that nothing connects the three lists, so intentional and accidental
look identical.

A related note: `firelight_settings` keys its entire API on `int platformId`
(`settings_service.hpp:31,37,68-88`, `settings_repository.hpp:19-21`) while linking nothing from
`firelight_platforms` — it cannot validate or name the identity it stores against. Same for
`contentHash` as a bare `std::string` from `library`. That is defensible as deliberate decoupling,
but it should be a decision rather than an accident.

---

## 6. What is already right

Worth stating, because the fixes below should not disturb it:

- **`EmulationContext`** (`src/app/emulation/emulation_context.hpp`) is the model: seven services as
  explicit interface pointers, forward-declared, injected from `main.cpp`, with a comment explaining
  why it is not the locator.
- **The event bus is disciplined.** Mapping all 39 event types to their defining module and their
  subscribers, every type but one is declared in its module's *public* `include/`, and events flow
  lib → app, never app → lib. The one exception is `RichPresenceMessageChangedEvent`, declared in the
  private `ra_client.hpp:25` and subscribed in `main.cpp`. (The bus itself is a process-wide
  `std::any`-keyed singleton, so these edges are invisible to CMake — but the edges themselves are
  sound.)
- **Per-lib test targets are correctly scoped.** `firelight_input_test`, `firelight_util_test` and
  friends link only their own lib. The boundary problems in L2 are all in the app-level `fl_test`.
- **`settings_level_shim.hpp`** already solves the enum-crossing problem correctly, with a drift
  `static_assert`. It just has not been applied to the other two enums that need it.
- **`CoreRegistry`** deliberately decouples core from platform and says so
  (`core_registry.hpp:112-114`).

Also worth flagging: **CLAUDE.md's module table is stale.** `libs/firelight/` holds 17 modules; the
table names 14 of them. `metadata`, `monitoring`, `netplay` and `util` are missing, so a reader
working from CLAUDE.md will not know they exist.

---

## 7. Recommended order

Sequenced so each step makes the next one smaller. Steps 1–3 are mechanical.

**1. Close the build hole (fixes L1, L3; shrinks L2).**
Delete `src/app` and `src` from `library`/`discord`'s include dirs. Then remove the lib `src/` paths
from `CMakeLists.txt:397,509,765` one lib at a time and fix what breaks — each break is an item from
L2 that needs either a public header or a factory. Start with `activity` and `mods` (main.cpp only),
finish with `library` (six consumers).

**2. Delete what is dead (L10, L11, L16, and the `platformIconName` chain in L13).**
Drop the `LibraryScanner` context property, the `platformIconName` property and its two QML copies,
and either revive `assertPlatformsEqual` or remove it along with its 13 commented call sites. Decide
whether `ControllerType::deviceClass` or `CoreDeviceVariant::deviceClass` is the authority and delete
the loser; if `ControllerType`'s stays, fix `imageUrl` in the serializers.

**3. Give `IPlatformService` the queries its callers already write (L6, L7, L8, L13, L14, part of L22).**

```cpp
[[nodiscard]] virtual std::vector<PlatformInputDescriptor>
    inputsFor(int platformId, int controllerTypeId) const = 0;
[[nodiscard]] virtual std::vector<int> controllerTypeIds(int platformId) const = 0;
[[nodiscard]] virtual std::vector<std::pair<int,int>> mappingKeys() const = 0;
[[nodiscard]] virtual std::string displayName(int platformId) const = 0;   // one fallback
[[nodiscard]] virtual std::string iconSource(int platformId) const = 0;    // one fallback
[[nodiscard]] virtual std::set<std::string> allFileAssociations() const = 0;
```

That removes six hand-rolled loops, seven display derivations, the `data()` hot-path copy, and lets
`EmulationService` drop its `Platform` value member for a `getCurrentPlatformName()`.

**4. Fix the injections (L9, L17, L18).**
`SqliteControllerRepository` takes `IPlatformService&`. `ServiceAccessor`'s five concrete slots become
their existing interfaces. The three self-constructed locator users take constructor arguments, and
`QtPlatformServiceProxy` drops the inheritance it never uses.

**5. Type the QML boundary (L19, L20, L21).**
`FolderInfo::type` becomes `FolderType`. Add `settings_level_shim`-style `Q_NAMESPACE` shims for
`GamepadInputClass` and `FolderType`. Replace the three parallel platform arrays with one list of
objects carrying `deviceClass`.

**6. Decide the remaining boundaries (L4, L5, L12, L15, L22).**
These need a judgement call, not a refactor:
- Does `RAClient` belong to the app (move the file) or the lib (strip the Qt, route QML through the
  existing proxy)? Either way, one door into achievements, not two.
- Does `ControllerType` belong in `platforms` at all, given it is written entirely in input's
  vocabulary? Moving it to `firelight_input` and keying it by platform id would make the CMake graph
  honest.
- Rename one of the two "controller type" concepts. `ConsoleControllerType` vs `GamepadHardwareType`,
  or whatever fits — the current collision is a live foot-gun.
- Reconcile the three extension lists, or write down why they are separate.
