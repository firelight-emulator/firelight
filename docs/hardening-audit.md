# Architecture hardening audit

A living inventory of structural smells and the plan to fix them. Feature work is paused for a
hardening pass focused on three themes: **de-Qt the domain modules**, **reduce global singletons**, and
**tighten interfaces / coupling**. God-object/large-file splitting is intentionally out of scope for now.

**How to use this doc:** it's the single source of truth for the effort — nothing gets refactored that
isn't listed here first. Each finding has a `file:line`, a severity, and the workstream that resolves
it. Check items off (`✅`) as they land; this doubles as the changelog.

## Target architecture (the rules we're aligning to)

- **Domain modules (`libs/firelight/*`) speak `std`, not Qt.** `std::string` / `std::function` /
  `std::future` in interfaces and value types. `QObject` / `QString` / `signals` / `Q_PROPERTY` /
  `Q_INVOKABLE` live only in **thin Qt adapters** at the QML boundary (the existing `src/gui/qt_*_proxy`
  pattern). A module links `Qt6::*` only when it genuinely needs it (e.g. `Qt6::Sql`, `QImage`).
- **One content-hash type everywhere:** `std::string` (achievements/cheats already do; library +
  settings + saves currently use `QString`).
- **Dependencies are injected**, not fetched from global singletons. The composition root
  ([main.cpp](../src/main.cpp)) owns the instances; `EmulationContext`-style constructor injection
  passes them down. `ServiceAccessor` is the **only** locator, and only for QML-default-constructed
  types (`qmlRegisterType` models/items) that can't take constructor args.
- **`EventDispatcher`** is an allowed decoupling bus, but domain modules must not subscribe to
  *app-level* events (that's a layering inversion).

## Findings

### Theme 1 — Qt in the domain (→ WS-1)

| # | Finding | Location | Sev |
|---|---|---|---|
| Q1 | ✅ **Interface-first done.** `ISaveManager` is now a **plain domain interface** — no `QObject`/`Q_OBJECT`/`Q_PROPERTY`/signals; the meaningless `QFuture<bool>` return on `writeSuspendPoint` is gone (→ `void`). Suspend-point changes are announced via `EventDispatcher` (`SuspendPointUpdated/DeletedEvent`, [save_events.hpp](../libs/firelight/saves/include/firelight/saves/save_events.hpp)); the QML save-directory binding moved to a thin `QtSaveManagerProxy`. `SaveManager` is no longer a `QObject`; dropped the **dead** `QThreadPool` + empty `handleUpdatedSuspendPoint` slot. `suspend_points_item` now subscribes to the events. **Impl `QString→std::string` also done:** `ISaveManager` + `SaveManager` now speak `std::string` for content hashes and the save directory; the file-I/O helpers convert to `QString`/`QDir` internally, and `QImage`(SuspendPoint)/`QSettings` remain as accepted boundary/utility types. | [isave_manager.hpp](../libs/firelight/saves/include/firelight/saves/isave_manager.hpp) | High |
| Q2 | ✅ **Fixed.** `library::Entry` is now 13 `std::string` fields (Qt-free value type). Conversion to `QString` happens only at the Qt-model edge (`entry_list_model`, `library_entry_item`, the activity/search models, `emulator_item`). The 3 duplicated row-readers in the sqlite impl collapsed into one `deserializeEntry` helper. | [entry.hpp](../libs/firelight/library/include/firelight/library/entry.hpp) | High |
| Q3 | ✅ **Fixed.** Repo + service `contentHash`/`filePath` params are now `std::string` — `contentHash` is one type (`std::string`) across achievements/cheats/activity/saves **and** library. **`ContentDirectory` now done too:** `path` → `std::string`, and the write-only `QDateTime lastModified` → `uint64_t lastModifiedEpochMs` — so **every** library value type (Entry, ContentDirectory, ContentFile, RunConfiguration, FolderInfo) is now `std`-only. | [user_library_repository.hpp](../libs/firelight/library/include/firelight/library/user_library_repository.hpp) | High |
| Q4 | ✅ **Fixed.** `SqliteSettingsRepository` is now a plain class implementing the std-typed `ISettingsRepository` — dropped `QObject`/`Q_OBJECT`/the 3 `Q_INVOKABLE QString` methods/6 `signals` (all **dead**: emitted but never connected, and the QML `EmulationSettingsManager` context property was **never referenced in any `.qml`**). `getEffectiveValue` kept as a pure-`std` helper. Removed the dead context-property registration + the `ServiceAccessor::{set,get}EmulationSettingsManager` pair. **Bonus:** the whole `firelight_settings` module is now Qt-free (dropped `Qt6::Core` + `find_package(Qt6)`, `AUTOMOC OFF`). | [sqlite_settings_repository.hpp](../libs/firelight/settings/src/firelight/settings/sqlite_settings_repository.hpp) | Med-High |
| Q5 | `media` keeps `QImage`/`QString` — **accepted boundary** (legit image/path types), documented so it isn't re-flagged. | [media/*.hpp](../libs/firelight/media/include/firelight/media) | — |
| Q6 | ✅ **Fixed.** Dropped dead `Qt6::Gui` links: `achievements` only uses `QDateTime`/`QCryptographicHash` → now links `Qt6::Core`; `activity` only uses `QSql*` → now links `Qt6::Sql` alone (Core comes transitively). `db` = `Qt6::Sql` (justified). `saves`/`media` keep `Qt6::Gui` legitimately (`QImage` in `SuspendPoint`/thumbnails). | `achievements`/`activity` `CMakeLists.txt` | Low |
| Q6b | ✅ **Fixed.** `SqliteUserLibraryRepository` ported off `QSqlDatabase`/`QSqlQuery` to bundled **SQLiteCpp**, matching the cheats/settings repos; `firelight_library` no longer links `Qt6::Sql`. Its old per-thread `QSqlDatabase` connections became one `SQLite::Database` guarded by a **recursive** mutex (recursive because `create()`/`update()` publish `EventDispatcher` events whose `LibraryIngestService` handlers synchronously re-enter the repo on the same thread). A shared connection also lets in-memory (`:memory:`) DBs work across the scanner worker thread — previously impossible with per-thread connections. | [sqlite_user_library_repository.cpp](../libs/firelight/library/src/sqlite_user_library_repository.cpp) | Med |

### Theme 2 — Global singletons (→ WS-2)

Domain modules reach these globals (call counts inside `libs/firelight/`): `EventDispatcher::instance()`
×60, `ShortcutRegistry::instance()` ×11, `SettingsCatalog::instance()` ×1. `PlatformService::getInstance()`
is **gone** — the singleton accessor was removed and every consumer now receives `IPlatformService` by
constructor injection (`ServiceAccessor::getPlatformService()` remains only for `qmlRegisterType` models).

| # | Finding | Location | Sev |
|---|---|---|---|
| S1 | ✅ **Fixed.** `SqliteControllerRepository` now takes `platforms::PlatformService&` by constructor injection (composition root passes the instance); the 3 in-module `getInstance().listPlatforms()` calls are gone. | [sqlite_controller_repository.hpp](../libs/firelight/input/src/firelight/input/sqlite_controller_repository.hpp) | Med |
| S2 | `SettingsCatalog` is a Meyers singleton (`instance()`), reached from domain code. | [settings_catalog.hpp:25](../libs/firelight/settings/include/firelight/settings/settings_catalog.hpp) | Med |
| S3 | ⚠️ **Partly addressed.** `CoreRegistry::resolveCoreName` no longer reaches into `SettingsService::instance()` — it takes the service explicitly (the one clear hidden-dependency removal). The `CoreRegistry::instance()` singleton itself stays: it's reached by `qmlRegisterType` models + **static free functions** in `platform_metadata.hpp` (`getCoreName`/`getCoreDllPath`), where full removal is a lateral `instance()`→locator move, not an elimination. | [core_registry.hpp](../src/app/libretro/core_registry.hpp) | Med |
| S4 | ⚠️ **Partly addressed.** `SettingsService` is now **injected into the emulation runtime** via `EmulationContext` — `EmulatorInstance` and `CoreRegistry::resolveCoreName` no longer call `instance()`; the emulation tests dropped the global `setInstance` hack accordingly. The `instance()`/`setInstance()` singleton remains **only** for the two `qmlRegisterType` settings models (`EmulationSettingsModel`/`CoreOptionsModel`), which are default-constructed by QML and grab it as a member initializer — the legitimate locator case. | [settings_service.hpp](../libs/firelight/settings/include/firelight/settings/settings_service.hpp) | Low-Med |
| S5 | `EventDispatcher::instance()` global bus, ×60 in domain. Likely **keep** (decoupling tool) but document its contract and confirm no domain→app-event subscriptions. | usage across `libs/firelight` | Med (review) |
| S6 | `ShortcutRegistry::instance()` ×11 — **module-internal** to input; lowest priority. | input module | Low |
| S7 | `EmulationService::getInstance()` settable-pointer singleton (app-level, not in a lib). | [emulation_service.hpp:75](../src/app/emulation/emulation_service.hpp) | Low-Med |

**Priority:** fix the ones crossing module boundaries first (S1, S2, S3), then app-level (S3/S7),
review-only for S5, leave S6.

### Theme 3 — Interfaces & coupling (→ WS-3)

| # | Finding | Location | Sev |
|---|---|---|---|
| C1 | ✅ **Fixed.** controller QML used snake_case roles (`display_name`, `controller_images`) against `PlatformListModel`'s **camelCase** roles → `undefined`. `ControllerProfilePage.qml` was already camelCase; fixed the remaining refs in `ControllerInputMappingView.qml`. | [ControllerInputMappingView.qml](../qml/components/controllers/ControllerInputMappingView.qml), [platform_list_model.cpp:60-76](../src/gui/platform_list_model.cpp) | High |
| C6 | ✅ **Fixed.** `ControllerImages` no longer skips empty urls, so it's aligned 1:1 with `controllerTypeIds`; the QML gained a `controllerImageUrl` helper that looks up via `controllerTypeIds.indexOf(controllerType)` instead of `controllerType - 1` (which broke for non-contiguous ids like NES = 1,3). Both usage sites use the helper. | [ControllerInputMappingView.qml:24](../qml/components/controllers/ControllerInputMappingView.qml), [platform_list_model.cpp:50](../src/gui/platform_list_model.cpp) | Med |
| C2 | ✅ **Fixed with Q4.** Turned out the QML exposure was entirely dead (no `.qml` referenced `EmulationSettingsManager`), so no proxy was needed — the repo is simply pure now and the dead registration is gone. | see Q4 | Med |
| C3 | ✅ **Fixed.** `EmulatorItemRenderer` no longer inherits `ServiceAccessor`; it takes its 5 services (`IActivityLog`, `RAClient`, `GameImageProvider`, `ISaveManager`, `MediaService`) by constructor from `EmulatorItem` (the legitimate `ServiceAccessor`). Every remaining `ServiceAccessor` consumer is now a QML-registered `*/gui/*` model. `main.cpp` only *sets* (composition root — fine). | [emulator_item_renderer.hpp](../src/app/emulator_item_renderer.hpp), [emulator_item.cpp:582](../src/app/emulator_item.cpp) | Low |
| C4 | Interface sweep: check `libs/firelight/*/include` for fat/leaky interfaces, `virtual` surface no caller uses, and concretes injected where an interface belongs. | `libs/firelight/*/include` | Low-Med |
| C5 | Header hygiene: drop unused / unnecessary Qt includes surfaced while doing the above. | repo-wide | Low |

## Workstream backlog (execution order)

Re-sequenced after discovering the saves *impl* is deeply Qt-entangled (Q1): lead with the smallest,
lowest-risk, individually-verifiable items; do the widest/riskiest de-Qt (saves impl) last.

**Landed this pass** (each built + `ninja check` green against the 14-achievement baseline):

1. ✅ **C1** (QML role bug): snake_case→camelCase roles in the controller mapping view.
2. ✅ **S1** (WS-2): `PlatformService` injected into `SqliteControllerRepository` — removed the only
   cross-module `getInstance()` call from inside a domain lib.
3. ✅ **C3** (WS-3): `EmulatorItemRenderer` no longer inherits `ServiceAccessor`; takes its 5 services
   from `EmulatorItem`. `ServiceAccessor` is now used *only* by QML-registered models.
4. ✅ **Q6** (WS-1): dropped dead `Qt6::Gui` links — `achievements`→Core-only, `activity`→Sql-only.
5. ✅ **C6** (WS-3): fixed `controllerImages` OOB indexing (align model list + `indexOf` lookup).
6. ✅ **WS-1b — library de-Qt** (Q2, Q3): `Entry` + repo/service `contentHash`/`filePath` params
   `QString → std::string`; converted at the Qt-model edge (~12 consumer files). `contentHash` is now
   one type across every module. `ninja check` green (baseline). This proved the de-Qt pattern for the
   riskier saves work.

**Remaining** (all wide or risky — each deserves its own focused session with the tight build/test loop):

7. **S2/S3** (WS-2, *deprioritized*): `SettingsCatalog`/`CoreRegistry` are **app-level** singletons
   (loaded-once config), not called from inside any domain lib — so they don't break module isolation
   like S1 did. Injecting them is wide (emulation service/instance, gui models, cli, `platform_metadata.hpp`
   static fns) for modest value. Do only if/when their global mutable state actually bites testing.
8. ✅ **WS-1c — settings repo de-Qt** (Q4, C2): `SqliteSettingsRepository` is a plain `ISettingsRepository`
   impl; dead QML/signals/accessor wiring removed; `firelight_settings` is now a fully Qt-free module.
   `ninja check` green (baseline).

9. ✅ **WS-1a — saves interface-first** (Q1): `ISaveManager` de-QObject'd (pure interface + `QtSaveManagerProxy`
   + `EventDispatcher` notifications); removed dead `QThreadPool`/slot/`QFuture`. `ninja check` green
   (baseline). The impl's `QString`/`QImage`/`QSettings` internals remain as an optional later sweep.

**Still remaining:**

10. ✅ **Saves impl + `ContentDirectory` de-Qt** (finish Q1/Q2/Q3): saves interface+impl `QString→std::string`
    and `ContentDirectory` `path`/`lastModified` → std types. `ninja check` green (baseline). All library
    value types + the save contract are now `std`-only.

**Still remaining:**

11. **WS-3 remainder** (C4, C5): interface sweep (fat/leaky virtuals, concretes where an interface
    belongs) + header/Qt-include hygiene surfaced along the way.
12. ⚠️ **App-level singletons (S3/S4) — partly done.** `SettingsService` injected into the emulation
    runtime; `CoreRegistry::resolveCoreName`'s hidden `SettingsService` dep removed (explicit param).
    The `instance()` singletons themselves remain where they're reached by `qmlRegisterType` models +
    `platform_metadata` static shims — full removal there is a lateral `instance()`→locator swap, so it's
    left as an accepted app-level locator. `SettingsCatalog` (S2) + `PlatformService` are unchanged for
    the same reason.
13. **Out of scope:** the `fl_qml_test` startup failure.

## Cleanup candidates spotted while executing (unscheduled)

Noticed in passing; not yet turned into workstream items. Each needs a quick confirm before acting.

- **`ContentDirectory` is the last Qt-typed library value type** (`QString path` + `QDateTime lastModified`,
  [content_directory.hpp](../libs/firelight/library/include/firelight/library/content_directory.hpp)).
  Natural WS-1 follow-up to finish library de-Qt (path → `std::string`; decide on a portable time type).
- ✅ **`PlatformListModel` stub methods** ([platform_list_model.cpp](../src/gui/platform_list_model.cpp)) —
  turned out to be one dead + one **broken-live**: `getPlatformIconName` had no caller (deleted);
  `getPlatformDisplayName` **was** called from [LibraryEntryListDelegate.qml:115](../qml/components/library/LibraryEntryListDelegate.qml)
  but returned `""` (stubbed when the `Platform` field was renamed `displayName`→`name`) — **fixed** to
  return the real name. (The `LibraryEntryItem::getPlatformIconName` is a different, live method.)
- ✅ **`ServiceAccessor` dead-accessor sweep.** After `EmulationSettingsManager` (WS-1c), audited all 16
  set/get pairs: **3 more fully dead** — `{set,get}SettingsService` (SettingsService is reached via its
  own `instance()`), `{set,get}UserdataManager`, and `{set,get}CoreSystemDirectory` (the core-system dir
  flows through `EmulationContext`). Removed the pairs + members + the two dead `set*` calls in
  `main.cpp`. The remaining 13 accessors all have live callers.
- **Dead `Q_INVOKABLE` convenience overloads.** The settings repo had three unused QML-only invokables;
  a sweep of the `src/gui/qt_*_proxy` classes may surface more invokables/`Q_PROPERTY`s nothing binds.
- **S2/S3 app-level singletons** (`SettingsCatalog`/`CoreRegistry`) and **large-file splits** — already
  tracked below / deprioritized, listed here for completeness.

## Verification (every step)

- PATH-prefixed `ninja` build of `firelight`, then `ninja check` (aggregate CTest) — all module tests +
  `fl_test` green against the known baseline (14 pre-existing achievement failures; `fl_qml_test`
  startup failure is out of scope).
- Add pure (Qt-free) unit tests where a de-Qt'd interface becomes newly testable (e.g. a fake
  `ISaveManager`).
- Manual smoke after UI-touching steps: app launches, game loads, saves/suspend points work (WS-1a),
  library lists + controller mapping images/labels render (C1/WS-1b), settings resolve (WS-1c/WS-2).
  **No behavior change is the goal.**

## Out of scope (deferred; revisit later)

- Splitting large files by responsibility: `core.cpp` (1629 lines), `sqlite_user_library_repository.cpp`
  (1361), `platform_service.cpp` (861), `main.cpp` (744 composition root).
- The `fl_qml_test` startup failure (exit 27, no output — pre-existing, unrelated to this effort).
