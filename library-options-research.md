<!-- TODO: NEEDS REVIEW -->
# Library options: competitive research & recommendations

Research into the game-library experience of thirteen emulator frontends, what their users praise and
complain about, and a prioritized recommendation for Firelight. Compiled from official docs, wikis, forum
threads, and GitHub issues (sources at the end).

Frontends surveyed: **ES-DE** (and the EmulationStation forks — Batocera / RetroBat / Knulli),
**LaunchBox / Big Box**, **RetroArch**, **ares**, **OpenEmu**, **Pegasus**, **iiSU**, **Playnite**,
**muOS**, **Daijishō**.

## Where Firelight is today

The library screen currently exposes only: **sort**, **view as** (grid/list), **tile size**, and
**show/hide unplayable games**.

The headline finding: the biggest wins are not more toggles. They are a handful of *features*, plus
surfacing capabilities Firelight has already built (achievements, suspend points, netplay, disc sets,
variant groups, per-game core, content-hash identity) that competitors structurally cannot copy.

---

## The five highest-impact additions

1. **Favorites as a first-class dimension** — one-button toggle → auto "Favorites" section →
   favorites-only filter → favorites-on-top ordering. The single most-used organizing action in the whole
   space (ES-DE's "Y" toggle is iconic). Firelight likely has the favorite primitive already; what's
   missing is the section/filter/sort around it.

2. **"Continue Playing" powered by suspend-point thumbnails.** Every frontend has a recently-played row
   showing box art. Firelight can show the *actual last frame* per game and per slot, and resume in place.
   This is the beloved auto-resume / history feature (muOS History, OnionOS auto-resume) done in a way no
   scraper-based frontend can match.

3. **Remember scroll / cursor position across launches and system switches.** The single most-requested
   navigation fix (RetroArch #8874, #17427); trivial to build and disproportionately loved, because the
   cost of not having it multiplies over every launch.

4. **Search everywhere** — in-view text search (table stakes), jump-to-letter / alphabet scroll
   (indispensable on controller & handheld; long-requested), and a **global cross-system search**. That
   last one is RetroArch's biggest search gap — people hand-edit `.lpl` files to fake a unified view.

5. **A faceted filter bar led by Firelight-native dimensions.** Genre / year / players is the baseline
   (RetroArch Explore, Playnite), but lead with **achievement state** (unplayed / in-progress / beaten /
   mastered / hardcore) and **completion status** (Playing / Beaten / Completed / Abandoned / Backlog).
   There is proven unmet demand for completion tracking — people run a *second app* (Playnite, Backloggd)
   purely for it, and LaunchBox added a "Game Progress" field to catch up.

---

## Firelight's moat — engines already built, waiting to be surfaced

Most of these have working backends already. The competition can't easily replicate them:

- **Achievement-aware library** — filter / sort / badge / section by RetroAchievements state, plus an
  "Almost mastered" smart collection. ES-DE, LaunchBox, RetroArch have nothing comparable.
- **Netplay in the browse itself** — a "Friends playing now" / joinable-lobby section and host-from-tile.
  No competitor puts live social play in the library.
- **1G1R via variant groups** with a quick "show other versions" — the Recalbox behavior users celebrate
  for "ending scrolling through duplicates," but hash-based instead of a filename hack. Name it
  **"Versions," never "Duplicates."**
- **Disc-set picker + in-game swap** — Firelight already folds discs; exposing the picker removes the
  universally-hated manual `.m3u` + `.hidden` folder chore.
- **Per-game core as a badge / quick-action** — RetroArch buries core association several menus deep.
- **Content-hash identity as a structural advantage** — rename or relocate a ROM and its art,
  achievements, saves, playtime, and collection membership all follow. This is exactly the pain (broken
  thumbnails, path-based collections) that sinks the others. Never regress to path keys.

---

## The complete option space, by category

Priority key: **[TS]** table-stakes · **[D]** strong differentiator · **[N]** nice-to-have ·
**[L]** later / avoid.

### View & layout
- **[TS]** Box-art grid (have) · sortable list/table view · tile size (have)
- **[TS]** "Lite" media mode — screenshots over video, fewer image types, lower res. The video wall is the
  #1 love *and* a top stutter complaint; ship the escape hatch from day one.
- **[TS]** Selected-game detail panel (media + metadata + actions) — Ozone dual-pane, LaunchBox Game
  Details are heavily used.
- **[N]** Artwork-source switch (box front / 3D box / cart / screenshot / logo) · video snap on hover
  (opt-in, gated behind lite mode) · fullscreen media viewer
- **[N]** Carousel / coverflow variant
- **[L]** Full swappable theme engine — huge scope, at odds with a curated design system.

### Sort
- **[TS]** Name (with article stripping, e.g. move "The") · ascending/descending toggle
- **[D]** Last played · Play time (total) · Favorites-on-top secondary ordering
- **[N]** Times played · Date added · Release year · Rating · Developer / Publisher / Genre / Players

### Filter
- **[TS]** Text search · Favorites-only · **Reset-all-filters** (without it, users think games are
  missing)
- **[D]** Faceted panel (values built from real metadata) · **Achievement-state** · **Completion status**
  · **Region/Versions** filter · Saveable filter presets (reusable curated lenses)
- **[N]** Missing-cover-art smart list · Availability facet (broaden the unplayable boolean into
  BIOS-missing / file-missing)

### Group / section
- **[TS]** Group by platform/system · Auto Favorites section
- **[D]** Auto "Continue Playing" / recently-played section
- **[N]** Recently-added · Bucketed recents (Today / Yesterday / This week) · Group by genre/year/developer

### Collections
- **[TS]** Favorites (one-button) · reference games by **stable hash id, never file path** (ES-DE's
  path-based collections break on relocate) · rename a collection without losing art (RetroArch ties
  thumbnails to the playlist filename)
- **[D]** Manual cross-system collections · **Smart / rule-based collections** (the celebrated
  Batocera/Playnite feature ES-DE notably lacks — a clear way to leapfrog it) · Auto History collection

### Metadata on tiles
- **[TS]** Box art + title label (so a missing-art grid isn't blank)
- **[D]** Achievement-progress badge · Status badges (favorite / completion / multi-disc / unavailable)
- **[N]** Playtime / last-played sublabel · Region/variant indicator on collapsed rows · Per-system count
  badges — **cached, not recomputed** (recomputing on section entry is a documented LaunchBox perf killer)

### Search
- **[TS]** In-view text search · Jump-to-letter / alphabet scroll
- **[D]** Global cross-system search
- **[N]** Persist the query across navigation / launch

### Per-game actions
- **[TS]** Toggle favorite · Hide (truly hidden, not dimmed)
- **[D]** Add/remove collection · Set completion status · **Resume from suspend slot (with thumbnail)** ·
  **Disc picker / in-game swap** · **Show other versions** · **Override core** · View achievements
- **[N]** Rate (stars) · Edit metadata / set custom box art

### Behavior toggles
- **[TS]** Remember scroll position across launches · Lite media mode · Cache counts · Incremental startup
  (no full rescan each launch) · "Hidden means gone, not dimmed" · Show/hide unplayable (have — broaden to
  a facet)
- **[D]** Auto-resume last game on startup **with a hold-to-force-menu escape hatch** · 1G1R region-
  preference collapsing with "show versions" · Multi-disc auto-collapse toggle
- **[N]** Favorites-on-top / folders-on-top

---

## Anti-patterns — what users demonstrably hate

Design against these from the start:

- **Rescanning / recomputing the whole library on startup.** The perennial 10k+ complaint (LaunchBox 8–10
  min at 50k). Do incremental sync off the content-hash index.
- **Recomputing per-system/collection counts on section entry.** A silent perf killer; LaunchBox's own
  fix is to stop doing it. Cache counts.
- **A rich video/box-art wall with no lite fallback.** Richness fights smoothness; stutters on big
  libraries.
- **Forgetting scroll position after a launch or system switch.** RetroArch's most-cited daily
  frustration.
- **"Hidden" that only dims instead of removing.** ES-DE's low-opacity default confuses everyone.
- **Tying artwork / identity to the filename or path.** A rename silently breaks art and collections.
- **Naming a version filter "Duplicates."** Users conflate byte-dupes, regional siblings, revisions and
  multi-disc. RomM renamed theirs "Versions" (PR #3688).
- **Auto-resume that boots to a black screen** when the last game crashed — always ship an escape hatch.
- **Leaving multi-disc games as N separate entries** and expecting hand-built `.m3u` + `.hidden` folders.
- **Paywalling the couch/big-screen mode or badges** the way LaunchBox gates Big Box behind Premium.
- **Overloading destructive shortcuts** (OnionOS's easy-to-trigger load-state combo) so a stray press
  wipes progress.

---

## Non-obvious insights

- The box-art/video wall is simultaneously the #1 love and a top performance complaint — the lite mode is
  not optional.
- "Show counts" badges are a silent performance killer; cache, never recompute on entry.
- Filename-as-database-key is a recurring failure across systems (RetroArch thumbnails, scrapers).
  Firelight's content-hash identity is exactly the right instinct.
- Not remembering scroll position is cited *more* than many "bigger" features because the cost compounds.
- On NAS libraries, the I/O path dominates perceived speed — ES-DE reports NFS is 10–30× faster than SMB
  for scans. If NAS is ever supported, that matters more than list-render speed.
- "Duplicates" is a dangerously overloaded word — separate and name byte-dupes, regional variants,
  revisions, and multi-disc carefully.
- There is proven demand to bring backlog/completion tracking in-house — users bolt on a second app for it.
- On handhelds, speed/simplicity often beats feature richness (muOS wins fans for booting instantly).

---

## Sources

RetroArch: [#8874](https://github.com/libretro/RetroArch/issues/8874),
[#17427](https://github.com/libretro/RetroArch/issues/17427),
[#18012](https://github.com/libretro/RetroArch/issues/18012),
[playlists/thumbnails guide](https://docs.libretro.com/guides/roms-playlists-thumbnails/) ·
LaunchBox: [last-game request](https://forums.launchbox-app.com/topic/29101-option-to-select-or-launch-last-game-played-when-starting-launchbox/),
[recent-played improvement](https://forums.launchbox-app.com/topic/75531-feature-request-last-played-game-improvement/),
[slow startup](https://forums.launchbox-app.com/topic/54083-slow-startup/),
[perf troubleshooting](https://feedback.launchbox-app.com/en/help/articles/9454889-troubleshooting-launchbox-and-big-box-performance),
[BigBox stutter fix](https://forums.launchbox-app.com/topic/82437-fixed-big-box-slowdownstutter-when-scrolling/) ·
ES-DE: [user guide](https://gitlab.com/es-de/emulationstation-de/-/raw/master/USERGUIDE.md),
[FAQ](https://gitlab.com/es-de/emulationstation-de/-/raw/master/FAQ.md) ·
EmulationStation: [#253](https://github.com/Aloshi/EmulationStation/issues/253),
[#611](https://github.com/Aloshi/EmulationStation/issues/611),
[#408](https://github.com/Aloshi/EmulationStation/issues/408) ·
1G1R: [Recalbox](https://www.recalbox.com/blog/2026-02-22-one-game-one-rom-1g1r-liste-jeux-retrogaming/),
[RomM #1736](https://github.com/rommapp/romm/issues/1736),
[RomM "Versions" PR #3688](https://github.com/rommapp/romm/pull/3688) ·
OpenEmu: [#2270](https://github.com/OpenEmu/OpenEmu/issues/2270) ·
Playnite: [library statistics](https://api.playnite.link/docs/manual/features/gameLibraryStatistics.html),
[emulation support](https://api.playnite.link/docs/manual/features/emulationSupport/emulationSupportOverview.html) ·
muOS: [history module](https://muos.dev/tour/modules/muxhistory) ·
Pegasus: [meta files](https://pegasus-frontend.org/docs/user-guide/meta-files/) ·
OnionOS: [FAQ](https://onionui.github.io/docs/4.1/faq) ·
Multi-disc: [Skyscraper #38](https://github.com/muldjord/skyscraper/issues/38),
[m3u guide](https://www.joeysretrohandhelds.com/guides/setup-m3u-for-multi-disc-games/) ·
Scraping: [box-art troubleshooting](https://heldgames.com/guides/fix-box-art-scraper-not-working) ·
Roundups: [handheld front-ends](https://droix.net/blogs/retro-handheld-front-ends/),
[muOS vs Knulli vs Onion](https://heldgames.com/guides/muos-vs-knulli-vs-onion-os)
