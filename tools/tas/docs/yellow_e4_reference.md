# Pokémon Yellow — Elite Four gauntlet reference (for TAS authoring)

Disassembly-verified (pret/pokeyellow) + Bulbapedia/Serebii cross-check. Compiled
2026-07-11 for the FireLight TAS POC. Enemy rosters/levels are byte-exact from
`data/trainers/parties.asm`; movesets are Yellow-specific (differ from Red/Blue).

## Gauntlet structure (maps, warps, locks)

Five sequential single-room battles, each its own map, **no healing/PC/save between**.

| Room | Map ID | Forward warp (top) |
|---|---|---|
| Indigo Plateau Lobby (last Center + Mart + PC) | `$AE` (174) | up → Lorelei |
| Lorelei's Room | `$F5` (245) | (4,0)/(5,0) → Bruno |
| Bruno's Room | `$F6` (246) | (4,0)/(5,0) → Agatha |
| Agatha's Room | `$F7` (247) | (4,0)/(5,0) → Lance |
| Lance's Room | `$71` (113) | (5,0)/(6,0) → Champion |
| Champion's Room | `$78` (120) | on win → Hall of Fame |
| Hall of Fame | `$76` (118) | — |

Lock mechanic: on Lorelei-room load, `BIT_STARTED_ELITE_4` is set in `wElite4Flags`.
Entering a room auto-walks you in and starts the battle; trying to leave before the
win prints "Don't run away!" and force-feeds Up. The forward exit is a solid tile
(`$24`) swapped to walkable (`$05`) only after `EVENT_BEAT_<ROOM>_TRAINER_0`; then you
walk north into the warp. One continuous session — carry healing items.

## Champion Eeveelution — `wRivalStarter` = `$D716`

1 = Jolteon, 2 = Flareon, 3 = Vaporeon. Set by scripts (NOT a win counter):
- Win the Oak's-Lab rival fight → 2 (Flareon); lose → 3 (Vaporeon).
- Then win the **Route 22** rival fight → upgrades 2→1 (Jolteon).
- Net: **win Lab + win Route 22 → Jolteon**; win Lab only → Flareon; lose Lab → Vaporeon.

**POC choice: Jolteon** (set `$D716 = 1`). Its ace (Jolteon L65, pure Electric, no
Ground move) is hard-countered by Earthquake; Rhydon is also immune to its Electric.

## Enemy teams (send order = party order)

**Lorelei** (kill fast; watch Jynx Lovely Kiss = sleep, Lapras Confuse Ray). All Waters 2× weak to Electric.
- Dewgong L54 · Cloyster L53 · Slowbro L54 (kill before Amnesia) · Jynx L56 (pure Ice) · Lapras L56 (ace)

**Bruno** (Psychic walls his Fighters; Onix have base-20 Special → any special OHKOs). No status moves.
- Onix L53 · Hitmonchan L55 · Hitmonlee L55 · Onix L56 · Machamp L58 (Karate Chop = high-crit)

**Agatha** (status spam: Confuse Ray×3, Hypnosis, Glare, Toxic). All part-Poison → Psychic 2×; Earthquake OHKOs all but Golbat (Flying). AI routine 4 = never switches.
- Gengar L56 · Golbat L56 · Haunter L55 · Arbok L58 · Gengar L60 (ace; Psychic + Hypnosis/Dream Eater)

**Lance** (every mon has Hyper Beam slot 4). Gyarados 4× weak Electric; Dragons/Aerodactyl take Ice; Blizzard freeze = pseudo-KO.
- Gyarados L58 · Dragonair L56 · Dragonair L56 · Aerodactyl L60 (fast, base 130 Spe) · Dragonite L62 (ace)

**Champion (Jolteon variant)** — lead always Sandslash; Alakazam is priority kill (Psychic + Recover).
- Sandslash L61 · Alakazam L59 · Exeggutor L61 · Cloyster L61 · Ninetales L63 · **Jolteon L65** (EQ OHKOs)

## Recommended player team (balanced, ~Lv 60+)

| Mon | Moves | Role |
|---|---|---|
| Starmie | Thunderbolt / Ice Beam / Psychic / Recover | lead special sweeper; fast, high crit |
| Tauros | Body Slam / Hyper Beam / Earthquake / Blizzard | physical crit machine (110 Spe) |
| Lapras | Blizzard / Surf / Thunderbolt / Body Slam | Ice cleaner for Lance; bulky |
| Exeggutor | Sleep Powder / Psychic / Stomp / Explosion | status opener + special wall |
| Rhydon | Earthquake / Rock Slide / Body Slam / Substitute | Ground for Agatha/Champion; EQ OHKOs Jolteon |
| Pikachu | Thunderbolt / Surf / Thunder Wave / Quick Attack | mandatory Yellow mon; Electric + para |

Items into the gauntlet: ~8–10 Full Restore, 2–3 Max Revive, 3–4 X Speed, 2–3 X Special.

## Gen-1 mechanics the TAS exploits (luck-manip levers via idle-frame timing)

- **Crit** = random byte < floor(baseSpeed/2); high-crit moves (Slash/Razor Leaf/Crabhammer/Karate Chop) ×4 (~99.6% for fast users). **Crits ignore all stat stages** (blow past Amnesia/Withdraw/Reflect).
- **1/256 miss** on every move except Swift/Bide — manip to always hit.
- **Sleep**: target can't act the turn it wakes; a faster sleeper can permanently lock.
- **Freeze**: no self-thaw in Gen 1 = pseudo-KO. Blizzard's 10% freeze on Lance's Dragons is a win condition.
- **Special = one stat** (offense + defense); Amnesia / X Special boost both.
- **Partial-trap** (Wrap/Clamp/Fire Spin) can lock turns — OHKO the user or manip AI away.
- **Hyper Beam** skips its recharge on a KO (Lance/Champion abuse it; so can you).

Highest manip leverage: crit bulky walls (Slowbro/Machamp/Alakazam), force enemy misses
(Lance Hyper Beams, Jolteon Thunder), sleep Champion Exeggutor/Alakazam, Blizzard-freeze Lance's Dragons.

## Battle RAM (Yellow; anchor on tools/tas/yellow_ram.hpp — some derived, re-verify live)

- `wIsInBattle=0xD056` (0 none / 1 wild / 2 trainer / 0xFF lost) · `wCurOpponent=0xD058`
- Enemy active (base 0xCFE4): species `0xCFE4`, HP `0xCFE5` (2,BE), status `0xCFE8`, level `0xCFF2`, maxHP `0xCFF3`
- Player active (base 0xD013): species `0xD013`, HP `0xD014` (2,BE), status `0xD017`, level `0xD021`, maxHP `0xD022`
- `wPlayerMoveNum=0xCFD1` · `wEnemyMoveNum=0xCFCB` (executing move) · `wCriticalHitOrOHKO=0xD05D` · `wMoveMissed=0xD05E` · `wDamage=0xD0D6` (2,BE)
- BattleStatus flags: player `0xD061/62/63`, enemy `0xD066/67/68` (turn-phase / multi-turn lock)
- Overworld: `wCurMap=0xD35D`, `wYCoord=0xD360`, `wXCoord=0xD361`, `wPlayerDirection=0xD529`
- `wElite4Flags` and the move-MENU-selection addresses are APPROXIMATE — reconfirm vs a fresh pokeyellow.sym before hardcoding.
