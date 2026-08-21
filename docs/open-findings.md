# Open findings

Bugs and gaps turned up while building `verify-ui` and `doctor`. Everything here is **still open** —
fixed items are not listed. Each entry says how it was confirmed, because a few came from automated
audits and were only partly right.

Confidence markers: **[verified]** = I reproduced it. **[reported]** = found by an audit, not
independently reproduced.

---

## Ship-blocking

### The PS1 core is missing from the source tree — **[verified]**

`mednafen_psx_hw_libretro` is the registered default for `PLATFORM_ID_PS1`
(`src/app/libretro/core_registry.cpp:84,104`), but the DLL is **not** in `_cores/windows/` — that
directory holds 13 DLLs, all tracked in git, and none of them is the PSX core.

It *is* present in `build/debug-win/system/_cores/windows/` (14 files), untracked and unignored, so it
works on this machine as a stale build artifact. **A fresh clone builds without it and every PS1
launch fails** with only a log line from `game_loader.cpp:105-108`.

`CoreRegistryTest.EveryPlatformDefaultCoreIsInstalled` covers this and was confirmed to go red when
the DLL is genuinely absent — so CI will catch it. Fix is to add the DLL to `_cores/windows/`.

### Schema versioning — **[verified via `doctor`]** — *resolved*

Every database now reports a `user_version` on a fresh install (`doctor` shows all `v1`, controllers
`v2`), all managed by the forward-only runner at `include/firelight/migrations/migration_runner.hpp`.
The `saves`, `achievements`, `library`, `metadata` (media-asset), `settings`, and `cheats` repos each
adopted a v1 migration wrapping their existing schema. `SqliteSettingsRepository` now also owns the
`core_options` table (it implements `ICoreOptionRepository` too), so `settings.db` has a single owner
of its four tables and its schema version — `doctor` shows it as `v1, 4 tables`.

The runner's own tests (`tests/app/migrations/migration_runner_test.cpp`, 3 cases) only exercise the
loop with lambdas — **no test opens a v1 file, reopens it with a v2 schema, and asserts the data
survived.**

---

## Correctness

### `writeSuspendPoint` ignores its own `saveSlotNumber` parameter — **[verified by reading]**

`libs/firelight/saves/src/save_manager.cpp:189-193`:

```cpp
void SaveManager::writeSuspendPoint(const std::string &contentHash, int saveSlotNumber, int index,
                                    const SuspendPoint &suspendPoint) {
  writeSuspendPointToDisk(contentHash, index, suspendPoint);   // saveSlotNumber not passed
  EventDispatcher::instance().publish(SuspendPointUpdatedEvent{contentHash, saveSlotNumber, index});
}
```

The directory comes from `suspendPoint.saveSlotNumber` (`:236`) while the published event uses the
`saveSlotNumber` **parameter**. If a caller ever passes one that disagrees with the struct field, the
file lands in one slot and the UI is told about another — and `readSuspendPoint` would look in the
slot the event named and find nothing.

The new tests pass because they set both consistently. Either drop the parameter or assert they match.

### `Core::loadGame` dereferences a null audio receiver — **[reported]**

`src/app/libretro/core.cpp:160` calls `audioReceiver->initialize(...)` unguarded, while the sibling
`videoReceiver` call two lines above **is** guarded. `audioReceiver` defaults to null
(`core.hpp:159`) and `EmulatorInstance::initialize` only sets it when an audio factory is present
(`emulator_instance.cpp:116-125`, whose comment says "Null in headless/tests").

Any headless path that loads a real core segfaults here. One-line guard; blocks `run-frames`.

### PPSSPP asset seeding runs after the headless dispatch — **[reported]**

`main.cpp:391-408` seeds `core-system/PPSSPP` from the install, but the headless subcommand dispatch
returns at `main.cpp:189-200` — long before it. So `scan`, `login`, `doctor` and `verify-ui` all run
without PSP assets, and any future headless command inherits the same silently-broken PSP.

This is a latent bug **today**, not just a `run-frames` blocker. Hoist the seeding into something both
entry points call.

### Two QML warnings block a strict gate — **[verified]**

`firelight verify-ui --fatal-warnings` fails 3 routes on 2 distinct warnings:

- `DelegateModel::cancel: index out range 0 0` — on `/library` and `/gallery/games`. A Qt-internal
  complaint from view recycling; needs real digging.
- `FLModShopItemPanel.qml:156: QML QQuickImage: Cannot open: qrc:images/mods/mk64ampedup/clearlogo` —
  a genuinely missing asset in the shop demo data. Either add it or make the placeholder deliberate.

Fixing both would make `--fatal-warnings` usable as the default gate. Until then it is opt-in.

---

## Disabled or missing coverage

### `tst_FLFocusHighlight` — 9 failing tests, now excluded — **[verified]**

Renamed to `qml_tests/components/disabled_tst_FLFocusHighlight.qml` so Quick Test's filesystem
discovery skips it (removing it from the CMake list was **not** enough — discovery is by filename, not
by module membership).

The failures look like the same stale-test pattern as the old E2E one: `test_initial_state` expects
the highlight to fill its target and gets `null`; `test_radius_with_background` expects 9 and gets
`undefined`. Deciding whether the component or the test is wrong means deciding how `FLFocusHighlight`
is meant to behave.

### `RtcTransportTest.LoopbackConnectExchangeAndClose` is `DISABLED_` — **[verified]**

`libs/firelight/netplay/tests/rtc_transport_test.cpp:41`. The only test in the repo that would prove
libdatachannel actually connects. Everything else in netplay runs against `fake_transport.hpp`.

### Modules with no tests at all — **[reported]**

- **`RAClient`** (`libs/firelight/achievements/src/rcheevos/rcheevos/ra_client.cpp`) — the entire
  rcheevos runtime: hardcore mode, `doFrame(ICore*)`, `loadGame`. Zero tests, no fake, despite
  `IAchievementClient` existing as a seam.
- **`libs/firelight/mods/`** — `sqlite_mod_repository.cpp` has no tests and does not even use
  `CREATE TABLE IF NOT EXISTS`.
- **`libs/firelight/discord/`** — `IDiscordManager` and `ITokenStore` exist and are trivially
  fakeable; nobody has.
- **`src/gui/`** — the nine `qt_*_proxy.cpp` bridge classes, `game_image_provider`, `image_utils`,
  and the save-file/suspend-point list models.

### `EntryResolver` has 2 test cases — **[reported]**

`tests/app/library/entry_resolver_test.cpp`. This is the class the launch path uses to pick content
and patch, and **the patch-resolution branch is untested**.

### The N64 content hash is pinned to stability, not a golden value — **[verified]**

`tests/app/library/content_hasher_test.cpp` asserts the GBA ROM's exact hash
(`e26ee0d44e809351c8ce2d73c7400cdd`, corroborated by the E2E test) and the RFC 1321 MD5 vectors, but
for `testrom.z64` it only asserts determinism. Recording whatever the byte-swap path currently emits
would enshrine it as correct without evidence. **Tighten once real hashes are available.**

---

## Cleanup

### `content.db` is shipped and read by nothing — **[verified]**

Built and installed (`CMakeLists.txt:469,727`), zero source references anywhere. `doctor` reports it
as unused shipped data. Deletion candidate.

### `metadata.db` never ships populated — **[verified via `doctor`]**

Opened at `<appdata>/metadata.db` (`main.cpp:317`) but nothing ever ships or copies a populated one,
so offline metadata is inert on a fresh install. `doctor` reports it as a warning since the code
tolerates absence.

### `geolith_libretro` is unreachable — **[verified via `doctor`]**

Ships in `_cores/windows/` but has an empty `supportedPlatformIds`, so `resolveCoreName` can never
select it. Either give it platforms or stop shipping it.

### `CliOptions` is straining — **[verified]**

20+ fields across 8 commands, and `--route` means two different things depending on whether it
precedes `verify-ui`. Worth restructuring before command #9.

### No BIOS handling exists anywhere — **[reported]**

No provisioning, validation, or even an expectation of which files each platform needs. Cores that
require BIOS just fail internally. `doctor` has a BIOS section that is `info`-only because there is
nothing to check against yet.
