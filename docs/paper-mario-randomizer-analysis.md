<!-- TODO: NEEDS REVIEW -->
# Paper Mario Randomizer: integration analysis

What it would take to put **Shop → Randomizers → Paper Mario** in Firelight: fill out a form, press
generate, and get a playable entry in the library.

Target: [icebound777/PMR-SeedGenerator](https://github.com/icebound777/PMR-SeedGenerator) (MIT), the
backend behind the official web generator for the Open World Paper Mario Randomizer.

Every number below was measured against a clone of the generator at `0.31.2 (beta)` and against this
repo, not estimated.

**Scope: backend only.** Nothing here costs or designs the screens — this is about where the seed gets
generated, what crosses the boundary, and how the result becomes a library entry.

See `randomizer-platform-design.md` for the general case: multiple randomizers, multiworld, and the
store. This doc is one instance of that shape, costed in detail.

## Verdict up front

The library-side work is small, because Firelight already has almost all of it. What's left is
deciding where the randomization logic runs — and on a second, measured pass that decision is much
less fraught than it first looked.

The generator already has a "don't touch the ROM, just hand back a patch" entry point built for the
website. Wrapping it so a host process can drive it takes **one 47-line file and no fork of upstream**;
frozen standalone it is **25 MB on disk, 10 MB compressed**, and returns a seed in about 5 seconds. That
same binary is what you would host if you ever wanted remote generation, so **local subprocess versus web
service is a transport switch, not an architecture** — build the wrapper and both stay open.

A native C++ port is also more reasonable than I first said: the real target is **~12,700 lines, not
26,000** (nearly half the tree is data that exports rather than translates), and seeds are provably
deterministic, so bit-exact parity is reachable and differentially testable. It is still the expensive
option and nothing forces it today — but it becomes the answer if randomizers need to work on Android.

The patch it produces is a **43 KB blob in a 5-byte-per-op format a C++ applier consumes in well under
100 lines**, so the boundary between "generator" and "Firelight" is narrow wherever it gets drawn.

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

## Where does the generation logic run?

Re-reviewed with measurements rather than instinct. The three candidates — native port, web endpoint,
Python wrapper — turn out to be less far apart than they look, and two of them share an artifact.

### First: it is not 26,250 lines of logic

Classifying every `.py` file by whether it contains any `def`/`class`/control flow at all:

| | Lines |
|---|---|
| Pure data literals (lists and dicts, no logic) | **14,093** |
| Actual code | **15,376** |
| Total | 29,469 |

Nearly half is data — item tables, location names, formation metadata, graph edges — that a port would
**export to JSON or SQLite rather than translate**. `maps/` is a further 2.2 MB already sitting in JSON,
and `default_db.sqlite` (581 KB) is already SQLite, which Firelight already speaks.

The code that would actually have to be ported:

| Area | Code lines | Notes |
|---|---|---|
| `rando_modules/` | 6,021 | The algorithms. `logic.py` (1,714) and `modify_entrances.py` (1,518) are the bulk |
| root | 3,397 | `random_seed.py`, `worldgraph.py`, `table.py`, `parse.py`, `spoilerlog.py` |
| `models/` | 3,259 | The option set — much of it declarative boilerplate |
| `db/` | 876 | peewee over the SQLite file; replaced outright by SQLiteCpp, not ported |
| `plandomizer/` | 1,151 | Skippable in a first cut |

So the honest port target is roughly **12,700 lines of C++**, not 26,000. Still large, still
correctness-critical, but a different order of problem than I first described.

### Second: bit-exactness is required, and it is achievable

A port that produces different items for the same seed is a *different randomizer*. Communities share
seed numbers for races and weekly events; a permalink that doesn't reproduce is a broken feature, not a
minor incompatibility. So a port has to match upstream byte-for-byte.

I assumed that was the fatal problem. It isn't. Generating seed `424242` three times in separate
processes:

```
PLACEMENT_MD5 360061ef0e33eb5b285409cc2dc35e0c  n=718
PLACEMENT_MD5 360061ef0e33eb5b285409cc2dc35e0c  n=718
PLACEMENT_MD5 360061ef0e33eb5b285409cc2dc35e0c  n=718
```

Placement is **fully deterministic**. The whole-patch hash *does* vary between runs, which looks alarming
until you find why: `set_seed_hash()` calls bare `random.seed()` on purpose, reseeding from OS entropy to
pick the four item icons shown on the save-select screen. That is a cosmetic fingerprint of the file, not
of the seed, and it is the only non-determinism in the output.

Notably this held without pinning `PYTHONHASHSEED`, despite `logic.py` iterating a `set` of strings inside
the placement loop — so set ordering does not appear to reach placement. That's worth re-testing across
settings before betting on it, but three-for-three over 718 placements is decent evidence.

What a port would have to reproduce exactly:

- **CPython's `random`** — 112 call sites: `randint` (55), `choice` (33), `shuffle` (21), `randrange`,
  `sample`, `choices`, `random`. All of it is MT19937 plus `_randbelow`'s rejection sampling, and all of it
  is documented and readable. A faithful clone is on the order of 300 lines and is a *bounded* problem —
  you either match the reference vectors or you don't.
- **Insertion-ordered dicts**, since Python 3.7 guarantees them and the code relies on it.

And critically, the port is **differentially testable**: run Python and C++ over the same (seed × settings)
matrix and compare placement hashes. That turns "months of hoping" into a measurable convergence, which is
the single biggest thing in the port's favour and something I under-weighted the first time.

### Third: the wrapper is 47 lines, and I tested it

The generator already has the entry point — `web_randomizer()`, the one the official site uses, which
returns patch bytes instead of writing a ROM. Wrapping it for a host process needs **one new file, no fork,
no patches to upstream**: read JSON settings on stdin, write one JSON envelope on stdout, redirect the
generator's progress chatter to stderr so stdout stays clean.

That file is 47 lines. Built and run against the real generator, it returns:

```
ok: true   seed: 424242   patch: 42,540 bytes   spoiler: 109,692 chars   settings: 329 keys
```

Frozen with PyInstaller into a standalone bundle that needs no installed Python:

| | Measured |
|---|---|
| Bundle on disk | **25 MB** |
| Compressed (installer impact) | **10 MB** |
| Cold run, subprocess round trip | **4.4–5.0 s** |
| Same, in-process | 2.5 s |

The ~2 s difference is interpreter startup plus the 1.4 s world-graph build, both paid per invocation. A
persistent worker process that builds the graph once would recover nearly all of it, and 5 s is inside
"press generate and watch a spinner" regardless.

Two things this settles:

- **Subprocess, not embedding.** There is no reason to link CPython into Firelight. A binary that reads
  stdin and writes stdout needs no Python C API, no GIL handling, no ABI coupling, and a crash in the
  generator cannot take the app down.
- **10 MB compressed is not the objection I made it out to be**, in an app already shipping ffmpeg, Qt and
  a dozen cores.

### The thing that collapses the decision

**The wrapper and the web service are the same artifact.** If you host generation, what you host is this
binary behind an HTTP handler. If you ship it locally, you ship the same binary and spawn it.

So "local subprocess" versus "remote service" stops being an architecture choice and becomes a transport
switch behind `IContentGenerator` — decidable later, changeable later, and testable both ways from day one.
That is not true of the port, which forecloses nothing but costs 12,700 lines up front.

### Recommendation

**Build the wrapper.** It is 47 lines against 12,700, it works offline, there is no service to operate, and
upstream updates are a rebuild rather than a re-port. Ship it as a frozen sidecar; add the remote transport
later if hosting turns out to be worth it, reusing the identical binary.

**Keep the port as a real option, not a strawman.** It is smaller than I said, bit-exactness is reachable,
and there is one scenario where it stops being optional: **Android.** A 25 MB CPython bundle per ABI is
genuinely bad there, and if randomizers matter on mobile, the port is the answer. The good news is that
sequencing costs nothing — the wrapper is exactly the reference implementation a differential test harness
needs, so building it first makes the port *cheaper and safer* if it ever happens.

### One robustness finding

Upstream validates almost nothing. Passing `{"SeedValue": "not-a-number"}` produced a cheerful success
envelope with a string where the seed should be, rather than an error. Whatever calls the generator has to
validate input itself; it will not be told when something is wrong.

## Settings: the surprise scope

`default_settings.yaml` has **347 keys**; `get_web_settings()` returns **329** — difficulty, item
placement, entrance rando, glitch logic, partners, palettes, audio, plus a full plandomizer.

Setting presentation aside entirely, this is still a real modelling problem: the C++ side has to hold a
settings object, serialize it to what the generator expects, validate it, and version it. Two things
make that harder than the key count suggests.

- **The YAML keys and the web settings names do not match 1:1.** `OptionSet.py` (2,363 lines) is the
  translation layer, and that is where the real schema lives — not in the YAML.
- **There is no machine-readable schema.** Types, ranges and the many interdependencies ("irrelevant
  if `IncludeShops:false`", "always false if `IncludeFavorsMode:0`") live in YAML comments and Python
  branches. Anything derived from the YAML alone will be wrong at the edges.

The cheap way through is to **treat settings as an opaque preset plus a small diff**: carry the 9
upstream presets (`Beginner`, `Intermediate`, `OpenWorld`, `ExtremeShuffle`, and five race presets) as
blobs, and hand-model only the subset actually exposed. That keeps Firelight out of the business of
mirroring a 329-field schema it does not own and cannot validate.

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
  This is correct behaviour — you can't cheev a randomizer — but the entry needs to carry the fact
  explicitly, so it reads as "not applicable" rather than as something that failed to load.
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

Backend only — no UI work counted. Assuming the frozen-wrapper transport and the preset-plus-subset
settings approach:

| Work | Estimate |
|---|---|
| Ops applier + CIC-6103 CRC + tests | 1–2 days |
| Base-mod acquisition & verification (detect dump, apply BPS, cache the result) | 2–3 days |
| Wrapper contract + frozen build in CI, per platform | 3–5 days |
| Subprocess driver (spawn, JSON in/out, timeout, cancellation, input validation) | 2–3 days |
| Settings model, preset handling, serialization | 3–5 days |
| Library integration (patch storage, entry creation, grouping) | 3–5 days |
| Service + model surface for whatever drives it | 2–3 days |
| **Backend total** | **~3–4 weeks** |

Adding the remote transport later is roughly 1 week of client work plus hosting the *same* binary. The
native port instead is on the order of **12,700 lines plus a differential test harness** — call it
2–4 months to reach provable parity, and only worth costing if Android forces it.

## Open questions

1. **Has anyone talked to the PMR team?** Shipping a frozen build of their generator inside Firelight
   is squarely within MIT, but it is still their project and their support burden when a seed misbehaves.
   Worth one message before it ships, not after.
2. **Do randomizers need to work on Android?** This is now the question that decides port-versus-wrapper,
   and nothing else does. A 25 MB CPython bundle per ABI is bad; if mobile matters, budget the port.
3. **Is the shop meant to be server-backed generally?** `ShopItemModel` carries `capsule_image_url`,
   `creator_name`, `user_has_required_game` — that reads like a remote catalogue, but the data is
   currently in the local `content.db`. Which is it going to be? A randomizer is a much easier sell if
   the shop already has a service behind it.
4. **Settings scope** — presets only, curated subset, or full parity? A data-contract question before
   it is anything else: full parity means owning a mirror of a 329-field schema that upstream changes
   without notice.
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
