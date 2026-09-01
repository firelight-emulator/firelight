<!-- TODO: NEEDS REVIEW -->
# Paper Mario Randomizer: integration analysis

What it would take to put **Shop → Randomizers → Paper Mario** in Firelight: fill out a form, press
generate, and get a playable entry in the library.

Target: [icebound777/PMR-SeedGenerator](https://github.com/icebound777/PMR-SeedGenerator) (MIT), the
backend behind the official web generator for the Open World Paper Mario Randomizer.

Every number below was measured against a clone of the generator at `0.31.2 (beta)` and against this
repo, not estimated.

## Verdict up front

The library-side work is small, because Firelight already has almost all of it. The blocker is one
architectural decision: **the randomization logic is 26,250 lines of Python and there is no realistic
way to run it in-process from C++.** Everything else follows from how that gets answered.

The good news is that the seed generator already has a "don't touch the ROM, just hand back a patch"
code path built for the website, and that path emits a **43 KB blob in a 5-byte-per-op format that a
C++ applier can consume in well under 100 lines**. So the boundary between "Python that must run
somewhere" and "C++ that Firelight owns" is clean and narrow, wherever you decide to draw it.

## What the generator actually is

| Property | Measured |
|---|---|
| Language | Python 3.11+ |
| Runtime deps | `peewee==3.17.6`, `PyYAML==6.0.1` — that's the whole list |
| Source | 26,250 lines across 234 `.py` files |
| Repo size (no `.git`) | 6.0 MB |
| Prebuilt data | `default_db.sqlite`, 581 KB (no rebuild needed at runtime) |
| Submodule | `PMR-Plando-Validator` — a **hard import**, not optional |
| License | MIT (validator submodule is separately MIT) |

Where the lines live: `rando_modules/` 6,000 · `metadata/` 5,902 · `models/options/` 2,744 ·
top level 3,385 · `db/` 873. The item-placement graph logic and the game metadata tables are the bulk.

### It runs, and it's fast

A full seed on this machine, cold:

```
world graph build : 1.37 s   (1,720 nodes — cacheable across seeds)
web_randomizer    : 1.56 s
patch output      : 43,068 bytes
spoiler log       : 109,557 bytes
```

Three CLI dry runs end to end: **2.6 s, 3.1 s, 3.4 s**. Generation speed is a non-issue — this is well
inside "press the button and watch a spinner".

Two snags found while getting it running, both worth knowing before anyone budgets this:

- `.gitmodules` uses an SSH URL, so `--recurse-submodules` fails without a GitHub key. The validator
  has to be vendored or cloned over HTTPS.
- The CLI writes its spoiler log to a hardcoded `../out/` and crashes with `FileNotFoundError` if the
  directory doesn't exist. Minor, but it means the CLI path is not quite a clean library API.

## The pipeline

```
Paper Mario (USA).z64          41,943,040 bytes · crc32 a7f5cd7e   [user must own this]
        │
        ├── apply  res/base_rando_0.31.1_beta.bps   (1.36 MB, BPS1)
        ▼
base mod ROM                   44,960,816 bytes · crc32 a887f010   [static, same for everyone]
        │
        ├── apply  seed patch  (~43 KB, generated per seed)
        ├── recalculate N64 CIC-6103 CRCs at 0x10 / 0x14
        ▼
randomized ROM                 → new content hash → library entry
```

The first arrow is already solved: `content.db` row `roms.id = 7855` is
`Paper Mario (USA).z64`, size **41,943,040**, crc32 **a7f5cd7e** — byte-for-byte the input the base-mod
BPS declares in its header. Firelight can already detect that the user owns the right dump and already
has a BPS applier. Nothing new is needed for that step.

### The seed patch format (decoded)

`write_data_to_array()` emits a flat opcode stream, big-endian throughout:

| Opcode | Payload | Meaning |
|---|---|---|
| `0x00` | `u32` address | Seek to ROM offset |
| `0x01` | `u32` value | Write 4 bytes at cursor, advance 4 |
| `0x02` | `u32` address | **Final** seek — after this the rest of the blob is raw `u32` words written sequentially, no more opcodes |

Then the two N64 boot CRCs at `0x10`/`0x14` are recomputed (CIC-6103, ~40 lines of ugly-but-known
integer math in `calculate_crc.py`).

That's the entire ROM-side contract. **A C++ applier plus CRC is a day of work, and it's the only piece
that has to be exactly right.**

One gotcha for anyone tempted to normalize this into a standard patch format: the randomizer's data
table lives at `0x01D00000` (30.4 MB), which **overflows IPS's 3-byte offset field**. IPS is out. Use
BPS, or add a native patch type — there's already precedent for the latter in
`libs/firelight/patching/src/pm_star_rod_mod_patch.cpp`.

## What Firelight already has

Considerably more than I expected. This feature is mostly assembly, not construction.

| Piece | Where | State |
|---|---|---|
| BPS applier | `libs/firelight/patching/src/bps_patch.cpp` | Done, tested |
| Star Rod / PM patch applier | `libs/firelight/patching/src/pm_star_rod_mod_patch.cpp` | Done — precedent for a custom record format |
| Patch-at-launch | `ContentLoader::applyPatch()` | Done — patches in memory, recomputes content hash |
| `PatchFile` model | `libs/firelight/library/include/firelight/library/patch_file.hpp` | Done (`IPS/BPS/UPS/XDELTA` enum would gain a member) |
| Run configurations w/ `patchId` | `run_configuration.hpp`, `IUserLibraryRepository` | Done — "a way to launch an entry" already models patched bytes |
| Entry creation | `IUserLibraryRepository::createEntry` / `createRunConfiguration` | Done |
| N64 emulation | `mupen64plus_next_libretro`, defaults in `platform_core_defaults.hpp` | Done |
| Shop scaffolding | `src/gui/models/shop/shop_item_model.{hpp,cpp}` | Stub — roles defined, `m_items` never populated |
| Shop routes | `/shop`, `/shop/mods/:modId` in `verify_ui_command.cpp` | Declared; commented out in `qml/v3/routing/RouteView.qml` |
| Shop delegate | `qml/shop/ShopGridItemDelegate.qml` | Exists, pre-design-system (raw `"#25282C"` literals) |
| Shop content model | `content.db`: `mods`, `mod_shop_pages`, `patches` | **Schema exists and is populated** |

That last row is the important one. `content.db` already ships 5 mods and 5 patches, and one of them is
a Paper Mario mod:

```
mods.id=2  'Ultimate Goomboss Challenge'  game_id=24942 (Paper Mario)  creator='Enneagon'
patches.id=2  'UltimateGoombossChallenge.bps'  mod_id=2  rom_id=7855
```

So the shop's data model is **already exactly the right shape**: a mod points at a game, a patch points
at a mod and a specific ROM dump. A randomizer differs from `Ultimate Goomboss Challenge` in precisely
one way — its patch is *generated per seed* instead of *downloaded*. That is a much smaller delta than
"build a randomizer feature".

## The one hard problem: where does the Python run?

26,250 lines of active, frequently-updated Python. Four options, honestly costed.

### A. Bundle a Python runtime

Ship CPython (embedded, or PyInstaller-frozen) plus the generator inside Firelight.

- **Works offline.** No service to run, no privacy story, no uptime.
- Adds roughly **15–40 MB** to the installer per platform, and a second toolchain to the build.
- Cross-platform packaging pain is real and recurring: Windows/MinGW is the primary dev environment
  today, and Android is already on the roadmap (`docs/android-port-gap-analysis.md`) — CPython on
  Android is a genuinely miserable path.
- Upstream updates become a vendoring chore, but a *tractable* one (drop in a new tree).

### B. Firelight-hosted generation service

Firelight runs the Python; the client POSTs settings and gets back 43 KB of patch bytes.

- Client side is **~200 lines of C++**: build JSON, `cpr::Post`, apply ops, recompute CRC. `cpr` is
  already a dependency (`achievement_service.cpp`, `cpr_http_client.cpp`).
- Keeps the C++ codebase clean and makes upstream updates a server-side deploy nobody has to ship.
- But: **you now operate a service.** Uptime, abuse, cost, and a feature that dies without a network.
- No ROM ever leaves the machine — only settings go up, only a patch comes down. Worth saying out loud
  because it's the obvious first objection and the answer is good.

### C. Call the official pm64randomizer.com generator

- Cheapest by far if it's on the table.
- **There is no documented public API.** `get_seed_json.py` talks to a private Firestore
  (`seeds-prod`) with a `service_account.json` credential, so the web frontend is not a REST surface
  anyone can just call.
- This is a *conversation with icebound777 and the PMR dev team*, not an engineering task. It may also
  be the best outcome for everyone — worth asking before building anything.

### D. Port the logic to C++

- Months of work, and then you own a permanent fork that must chase upstream's logic fixes forever.
  The `CHANGELOG` for a single patch release is dozens of logic corrections.
- **Not recommended.** The only argument for it is Android + offline, and that's not worth this price.

**Recommendation: ask about C first, build B, and keep A in your pocket** if offline generation turns
out to matter to users. B and A share everything except transport — the ops applier, the CRC, the
settings model, and all the library plumbing are identical either way, so starting with B does not
strand the work.

## Settings: the surprise scope

`default_settings.yaml` has **347 keys**; `get_web_settings()` returns **329**. This is not a form, it's
a settings *application* — difficulty, item placement, entrance rando, glitch logic, partners, palettes,
audio, plus a full plandomizer.

For calibration, the repo ships **9 presets** (`Beginner`, `Intermediate`, `OpenWorld`,
`ExtremeShuffle`, and five race presets).

Three plausible scopes:

1. **Presets only** — a dropdown of the 9 shipped presets, plus a seed field. Perhaps a week of UI.
   Ships the feature, and honestly covers most people who aren't already deep in the community.
2. **Presets + a curated subset** — the ~30 settings people actually touch, on top of a preset base.
   The sane target. A few weeks.
3. **Full parity** — 329 settings. Only worth it generated from a schema rather than hand-authored, and
   even then it's a big surface to design, theme, and keep in sync with upstream.

Worth noting `OptionSet.py` is 2,363 lines and the YAML keys don't map 1:1 to the web settings names, so
"just generate the form from the YAML" is less free than it looks.

## Library integration

The natural shape, given what already exists:

1. User picks settings → generate → 43 KB of ops come back.
2. Apply ops to base-mod bytes in memory, recompute CRCs.
3. Store the result as a `PatchFile` **relative to the base mod** — a BPS of base-mod → randomized is a
   few tens of KB, versus 43 MB for a full ROM per seed. This matters: people generate a lot of seeds.
4. `createEntry` + `createRunConfiguration(contentFileId, path, platformId, contentHash)` with the
   patch id. `ContentLoader::applyPatch` already does the rest at launch.

Consequences to decide on, not to discover later:

- **Achievements are gone.** `AchievementService` is keyed on content hash
  (`getAchievementSetByContentHash`), and a randomized ROM's hash matches nothing on RetroAchievements.
  This is correct behaviour — you can't cheev a randomizer — but the UI should say so rather than look
  broken.
- **Saves are per-hash too**, so every seed gets its own save file. Almost certainly what you want.
- **The spoiler log is 110 KB per seed** and needs a home, plus a deliberate choice about whether it's
  even shown (the web generator hides it from players by default).
- **Entries will multiply.** One per seed means a library full of `Paper Mario Randomizer`. Some
  grouping story is needed — `VariantGroup` already exists and may fit.
- **Base mod version drift.** `randomizer.py` declares `BASE_MOD_VERSION = "0.10.0 (beta)"` and
  `BASE_MOD_MD5 = 10785ABD…` while the shipped patch is `base_rando_0.31.1_beta.bps` and the changelog
  reads `0.31.2`. The CLI's own MD5 check is **commented out** with a note admitting it was disabled for
  usability. Whatever gets built needs its own version pinning; upstream's is not currently trustworthy.

## Rough effort

Assuming option B and settings scope 2, and assuming the shop scaffolding gets finished as part of it:

| Work | Estimate |
|---|---|
| Ops applier + CIC-6103 CRC + tests | 1–2 days |
| Base-mod acquisition & verification flow (detect dump, apply BPS, cache) | 2–3 days |
| Generation client (`cpr`, JSON settings, errors, cancel) | 2–3 days |
| Settings model + QML form (~30 settings, presets, `FL*` components) | 1–2 weeks |
| Library integration (patch storage, entry creation, grouping) | 3–5 days |
| Shop route/page revival, delegate rebuilt on the design system | 3–5 days |
| **Client total** | **~4–6 weeks** |
| Server (containerize generator, thin API, deploy, monitoring) | 1–2 weeks, plus ongoing |

Option A instead of B trades the server line for roughly 1–2 weeks of packaging work per platform, and
a permanent tax on the build.

## Open questions

1. **Has anyone talked to the PMR team?** Option C changes the whole shape of this. They may have an
   API, or want one, or object to a third-party client on principle. This is the highest-leverage
   question and it costs one message.
2. **Is a Firelight-operated backend acceptable at all?** If Firelight is meant to stay a purely local
   app, option B is off the table and this becomes "bundle Python", with everything that implies.
3. **Is the shop meant to be server-backed generally?** `ShopItemModel` carries `capsule_image_url`,
   `creator_name`, `user_has_required_game` — that reads like a remote catalogue, but the data is
   currently in the local `content.db`. Which is it going to be? A randomizer is a much easier sell if
   the shop already has a service behind it.
4. **Settings scope** — presets only, curated subset, or full parity?
5. **Can Firelight ship the base-mod BPS?** It's MIT in the repo, but the base mod's *source* is
   private, and redistributing the patch is a courtesy question as much as a licence one.
6. **Is Paper Mario the pilot for a general "Randomizers" section**, or a one-off? Nearly every
   randomizer (OoT, ALttP, Metroid) is a Python program with the same shape. If the answer is
   "general", the ops applier should be a plugin point rather than PM-specific code, and that changes
   the design now rather than later.

## Sources

- `icebound777/PMR-SeedGenerator` @ `0.31.2 (beta)` — cloned and executed
- `icebound777/PMR-Plando-Validator` — cloned
- This repo: `libs/firelight/patching/`, `libs/firelight/library/`, `src/gui/models/shop/`,
  `content.db`
