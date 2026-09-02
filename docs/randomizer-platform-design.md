<!-- TODO: NEEDS REVIEW -->
# Randomizers as a platform

Generalizing from "ship the Paper Mario randomizer" to "Firelight supports randomizers as a category,
sold through the store, with multiworld later."

Companion to `paper-mario-randomizer-analysis.md`, which costs one concrete instance in detail. This
doc is the shape around it. Backend only — no screens costed.

Everything measured against clones of `icebound777/PMR-SeedGenerator` @ `0.31.2 (beta)` and
`ArchipelagoMW/Archipelago` @ `main` (691,617 lines of Python, MIT, ~81 core-verified games).

## The reframe

The instinct is to build "a randomizer feature". That's the wrong unit, because it welds together three
things that have nothing to do with each other:

| Layer | Question it answers | Needed by |
|---|---|---|
| **1. Production** | How does a patch come to exist? | Every randomizer |
| **2. Session** | How does a *running* game talk to something outside itself? | Multiworld, trackers, DeathLink |
| **3. Catalogue** | How does a person find and configure it? | Everything in the store |

Keeping them apart is what makes the platform worth building:

- Paper Mario needs **1 + 3**. No session.
- Static ROM hacks (already shipped in `content.db`) need **3** alone.
- Archipelago needs **1 + 2 + 3** — but layer 2 is *also* what gives Firelight native auto-tracking and
  DeathLink for games with no randomizer at all.

Layer 2 is where the crazy lives, and it turns out to be the cheapest thing on the list. More below.

## Layer 1: production

### The abstraction

One interface, three transports:

```
IContentGenerator
  getOptionSchema()                          -> options + presets + constraints
  generate(baseContentHash, options, seed)   -> GeneratedContent
GeneratedContent { patchBytes, patchFormat, manifest, displayMetadata, spoilerData }
```

- **Remote** — POST options, get a patch back (Paper Mario; anything with a service).
- **Local process** — a bundled generator invoked out-of-process (a frozen Python binary, say).
- **Imported** — the user drops in a file someone else generated (`.apbp`, `.apz5`, a plain `.bps`).
  This one is free and should ship first: it's the fallback that makes every randomizer on earth work
  in Firelight on day one, without Firelight integrating any of them.

The important property, and it holds for both PMR and Archipelago: **generation never needs the user's
ROM.** PMR emits ops against a known base-mod image; Archipelago emits a bsdiff against a known
vanilla image. The user's dump only joins at *apply* time, locally. That keeps generation remotable and
keeps Firelight out of distributing anything it shouldn't.

### Patch formats

| Format | Used by | State |
|---|---|---|
| BPS / IPS / UPS | ROM hacks, base mods | Have — `libs/firelight/patching/` |
| Star Rod (`PMSR`) | Paper Mario mods | Have |
| **bsdiff4** | **Archipelago's default `APDeltaPatch`** | **Missing** |
| **AP token ops** (`WRITE`/`COPY`/`RLE`) | Archipelago `apply_tokens` | **Missing — trivial** |
| **PMR ops** (seek/write-word) | Paper Mario seeds | **Missing — trivial** |

Only bsdiff4 is real work, and it's a well-specified format (bzip2-compressed control/diff/extra
blocks) with reference implementations everywhere. Everything else is a day each.

`.ap*` patch files are just zips: `archipelago.json` manifest plus a `procedure` list, defaulting to
`[("apply_bsdiff4", ["delta.bsdiff4"])]`. Reading one is a zip read, a JSON parse, and a dispatch over
about four procedure names. This is a genuinely small surface, and it means **Firelight can consume
Archipelago patch output without running any Archipelago code.**

## Layer 2: the session — where this gets interesting

### The problem Archipelago players have today

To play an emulator game in a multiworld right now, you run: the game in BizHawk, *plus* a Lua
connector script inside it, *plus* a separate Python client process, *plus* (for SNES) SNI as a third
process. The official setup guides tell users to switch BizHawk's Lua core from "NLua+KopiLua" to
"Lua+LuaInterface" and restart, to toggle it back and forth if it looks already selected because fresh
installs lie about it, to tick "Run in background" so the client doesn't drop, to drag `Connector.lua`
onto the window, and to expect the emulator to freeze every few seconds until the client shows up.

That is the actual, current, documented onboarding.

### The lever

The BizHawk connector is not a deep integration. It is **a newline-delimited JSON socket on TCP ports
43055–43059** with exactly twelve commands:

```
PING · HASH · MEMORY_SIZE · SYSTEM · PREFERRED_CORES
LOCK · UNLOCK · DISPLAY_MESSAGE · SET_MESSAGE_INTERVAL
GUARD · READ · WRITE
```

`READ` takes `(address, size, domain)`. `WRITE` takes `(address, value, domain)` with the value
base64'd. `GUARD` is a compare-and-proceed: a batch only applies if the guarded bytes still match, which
is how clients avoid writing into a loading screen. `LOCK`/`UNLOCK` hold the frame so a batch is atomic.
Domains in use across the shipped worlds are just `RAM`, `EWRAM`, `IWRAM`, `WRAM`, `ROM`,
`System Bus`, `CartRAM` and `PRG ROM` — eight names to map onto libretro's memory ids.

**Firelight can speak this protocol natively.** And if it does, every existing Archipelago BizHawk
client works against Firelight *unmodified* — no changes to Archipelago, no Lua, no BizHawk, no core
switching, no third process. The user's setup becomes: open Firelight, join room.

That is a differentiator no other frontend has, and it is a few hundred lines of C++.

### Why it's cheap here specifically

Every primitive it needs already exists in this codebase:

| Connector needs | Firelight has |
|---|---|
| Read RAM | `Core::getMemoryData(unsigned)` — live writable pointer |
| Write RAM | `CheatEngine::apply()` already pokes it every frame |
| Region sizes | `Core::getMemorySize(id)` |
| **`System Bus` address resolution** | `rc_libretro_memory_init` / `rc_libretro_memory_read` — rcheevos already maps console-relative addresses through `retro_memory_map`, and `ra_client.cpp` already calls it |
| Frame-accurate hook | `EmulatorInstance::runFrame()`, where achievements already tick |
| A socket | Qt6 `Network` already a dependency |
| JSON | `nlohmann-json` already a dependency |

The `System Bus` row is the one that would otherwise be miserable, and it's already solved as a
side-effect of achievements.

### Coverage

23 of Archipelago's shipped worlds are ROM-patch games; 13 of those drive the BizHawk connector. The
consoles involved:

> A Link to the Past · Super Metroid · SMZ3 · EarthBound · Super Mario World · Yoshi's Island ·
> Kirby's Dream Land 3 · Lufia II · Secret of Evermore *(SNES)* · Link's Awakening DX · Super Mario
> Land 2 · Pokémon Red/Blue *(GB)* · Circle of the Moon · Mario & Luigi Superstar Saga · MegaMan
> Battle Network 3 · Pokémon Emerald · Yu-Gi-Oh! 2006 *(GBA)* · Mega Man 2 · Mega Man 3 · Zelda 1
> *(NES)* · Castlevania 64 · Gauntlet Legends *(N64)* · Zillion *(SMS)*

Firelight already emulates **every one of these** — snes9x, gambatte, mgba, fceumm, mupen64plus_next,
genesis_plus_gx are all in `_cores/`. There is no platform gap to close.

The SNES worlds go through SNI instead, which is a **usb2snes/QUsb2Snes websocket protocol** (`Opcode`
requests, `DeviceList` and friends) rather than the BizHawk socket. That's a second, independent
adapter — worth doing, but it can wait, and it's the same shape of work.

### Where the per-game logic actually lives

This is the part that decides everything else, so it's worth being precise. Three files, three jobs:

| File | Lines | Job |
|---|---|---|
| `worlds/_bizhawk/__init__.py` | 336 | **The connector transport** — the twelve commands. The only piece Firelight would replace |
| `worlds/_bizhawk/context.py` | 377 | Generic loop. Holds *both* connections — the AP server socket and the emulator socket — identifies the ROM by hash and system, dispatches to a game handler |
| `worlds/<game>/client.py` | 139–783 | **The game logic.** One per game |

A per-game client is a small class with `validate_rom`, `set_auth`, `game_watcher`, and `on_package`.
The critical property is that **it talks to both sides**. From `worlds/marioland2/client.py`:

```python
game_name = await read(ctx.bizhawk_ctx, [(0x134, 10, "ROM")])       # <- emulator
...
await ctx.send_msgs([{"cmd": "LocationChecks", "locations": ...}])  # <- AP server
```

It is the translation layer *between* the two connections, not a component hanging off either one.

### The ladder, and why rung 3 is a different bet

**Rung 1 — connector endpoint.** Firelight replaces the first row only. `context.py` and every
`client.py` keep running untouched, and the per-game client keeps both of its connections. **No
per-game work in Firelight, now or ever** — the game logic stays upstream's, including for games added
after this ships.

Better still, it needs no changes to Archipelago whatsoever. AP's client already launches the emulator
itself, and its `bizhawkclient_options.rom_start` setting accepts an arbitrary command line
(`shlex.split(auto_start)`). Point that at Firelight and the existing plumbing does the rest.

**Rung 2 — own the launch.** Firelight ships or drives the AP Python client so the user only ever sees
one app. Per-game logic is still Archipelago's; the cost is packaging, and it isn't trivial — the AP
client pulls in kivy, kivymd (from a git ref), websockets, orjson, cython and bsdiff4.

**Rung 3 — native AP client.** Firelight speaks the Archipelago websocket protocol itself. This is where
per-game logic becomes mandatory, and the reason is structural rather than incidental: **taking over the
server connection removes one of the two connections every per-game client is written against.**
`context.py` goes, and all thirteen `client.py` files go with it — 139 to 783 lines each, re-derived in
C++ and then chased forever as upstream fixes them.

So rungs 2 and 3 are not successive refinements of one idea. Rung 3 is a decision to fork Archipelago's
game logic, and nothing about rungs 1 or 2 requires it.

**Recommendation: rung 1, and stop there until there's a concrete reason not to.** It delivers nearly
all of the felt improvement, asks Archipelago for nothing, and forks nothing.

### What layer 2 buys you beyond multiworld

Once a supervised memory channel exists, several things become nearly free:

- **Native auto-tracking.** Randomizer communities run PopTracker or EmoTracker as a separate window
  fed by the same connector. Firelight could render a tracker itself. PMR even ships a
  `docs/RAMLocations.md` written explicitly for auto-tracker authors.
- **DeathLink.** It's an AP tag and a `Bounce` packet. Once connected, it's a rules detail.
- **Richer presence / activity.** The activity log currently knows "played for 40 minutes"; it could
  know "got to chapter 4".

## Layer 3: the catalogue

`content.db` already models `mods → patches → roms`, with `mod_shop_pages` for the storefront copy, and
it's populated (5 mods, 5 patches, including a Paper Mario mod pointing at the exact US dump the base
mod expects). `ShopItemModel` exists with roles defined and is never populated.

What's missing is one concept: **a mod whose patch is produced rather than downloaded.** Concretely a
`generators` table alongside `patches` — generator kind (remote/local/import), endpoint or command,
an options schema reference, and which base content and patch it targets. A randomizer then is a
mod that happens to carry a generator instead of a fixed patch, and everything else about the store
already works.

The options schema is the awkward part and deserves a decision rather than a default. Neither PMR nor
Archipelago publishes a machine-readable one: PMR's real schema is 2,363 lines of `OptionSet.py`, and
AP's lives in each world's `Options.py`. Mirroring either faithfully means chasing a moving target
forever. **Presets-as-opaque-blobs plus a small hand-modeled subset** keeps Firelight out of that,
at the cost of not exposing everything.

## What multiworld breaks

These are consequences to design for now, not to discover in a bug report.

- **Rewind and save states must be gated.** Loading a state after sending a check un-sends it locally
  while the server still has it: the multiworld desyncs and the item is gone. This is not hypothetical,
  it's the normal failure mode. Firelight already has exactly the right mechanism — `ShortcutActions`
  gates rewind and slow-motion behind a single `blockedByHardcore()` predicate driven by one
  `std::function<bool()>`. Generalizing that to "session integrity" (hardcore **or** an active
  multiworld) is close to a one-line change, and it's the single most valuable piece of prior art in
  the codebase for this feature.
- **Achievements are off.** `AchievementService` is keyed on content hash; a randomized ROM matches
  nothing. Correct, but the entry should carry the fact explicitly so it reads as "not applicable"
  rather than broken.
- **Saves are per-hash**, so every seed gets its own. Right, but it means seeds accumulate.
- **Entries multiply.** One per seed. `VariantGroup` already exists and probably fits; decide before
  someone's library has forty `Paper Mario` rows.
- **Spoiler logs need a home and a policy** (PMR's are ~110 KB each; AP hides them from players by
  default).
- **Generated content is not scannable.** A seed doesn't come from a watched directory, so
  `LibraryScanner2` isn't the ingest path — `LibraryIngestService` gets a second, deliberate entry
  point. Worth being explicit about, since everything in the library today arrives by scan.

## Phasing

Each phase is independently useful and independently shippable.

| Phase | Contents | Rough |
|---|---|---|
| **0** | Patch formats: bsdiff4, AP token ops, PMR ops. `.ap*` container reader | 1–1.5 wks |
| **1** | Import a pre-generated patch → library entry. Generator abstraction + ingest path | 1.5–2 wks |
| **2** | Remote generation (Paper Mario as the first instance) + store `generators` concept | 2–3 wks |
| **3** | **BizHawk connector endpoint** + memory-domain mapping + session-integrity gating | 2–3 wks |
| **4** | usb2snes/SNI adapter, bringing the SNES worlds in | 2–3 wks |
| **5** | *Optional:* own the launch, so the AP client isn't a separate app (packaging, not protocol) | 1–2 wks |

A native C++ AP client is deliberately not a phase. Per the ladder above, it forces reimplementing every
per-game client — a fork of Archipelago's game logic rather than an integration with it.

Phase 0–1 alone is worth shipping: it makes every randomizer that emits a patch file usable in
Firelight, with zero per-randomizer integration. Phase 3 is the one nobody else has.

Deliberately *not* in scope: **running Archipelago generation locally.** It's 691k lines of Python and
rooms are hosted (archipelago.gg, or self-hosted by the community) — Firelight should be a client of
that, never a host of it.

## Open questions

1. **Is the store server-backed or shipped in `content.db`?** A generator catalogue that updates
   without an app release is a different product than a baked-in table. This gates layer 3 entirely
   and is worth answering before phase 2.
2. **How far up the multiworld ladder do you actually want to go?** Rung 1 is a few weeks and huge for
   users. Rung 3 is a permanent per-game commitment. Rungs 1 and 2 don't obligate you to 3, but the
   abstractions differ depending on whether 3 is ever coming.
3. **Has anyone talked to Archipelago or the PMR team?** Both are MIT and neither needs permission for
   any of this, but a connector endpoint is much better with their blessing — and AP might want it
   upstreamed as a supported connector target, which would be the best possible outcome.
4. **Is a Firelight-operated generation service acceptable?** Same question as the PM doc, but it
   compounds here: a general "Randomizers" section implies hosting several generators, or bundling
   several Python programs, or accepting that most entries are import-only.
5. **Where does per-game connector logic live if rung 3 happens?** Compiled into Firelight, or
   data-driven from a manifest the store ships? The second is much better and much harder, and choosing
   late means rewriting.
6. **Do randomized entries belong in the library at all, or in their own space?** Forty seeds of one
   game is a different browsing problem than a game collection, and `VariantGroup` may not be the right
   answer.

## Sources

- `ArchipelagoMW/Archipelago` @ `main` — cloned; `worlds/_bizhawk/__init__.py`, `worlds/Files.py`,
  `SNIClient.py`, `worlds/`
- [Archipelago network protocol](https://github.com/ArchipelagoMW/Archipelago/blob/main/docs/network%20protocol.md)
- [A Link to the Past setup guide](https://archipelago.gg/tutorial/A%20Link%20to%20the%20Past/multiworld_en),
  [Pokémon Emerald / BizHawk setup](https://archipelago.gg/tutorial/Archipelago/other_en) — the current onboarding
- [Core-verified games](https://archipelago.miraheze.org/wiki/Core-verified_games)
- `icebound777/PMR-SeedGenerator` @ `0.31.2 (beta)` — cloned and executed
- This repo: `libs/firelight/patching/`, `libs/firelight/cheats/`, `libs/firelight/achievements/`,
  `src/app/libretro/core.{hpp,cpp}`, `src/app/emulation/shortcut_actions.hpp`, `content.db`
