#pragma once
// Pokemon YELLOW (pret/pokeyellow) RAM/HRAM addresses, for the TAS assist.
// Verified against the pokeyellow.sym symfile + ram/hram.asm. NOTE: Yellow's
// WRAM gameplay data is shifted vs Red/Blue (Pikachu data) -- do NOT reuse Red
// addresses. The HRAM RNG bytes, however, match Red/Blue.
// Multi-byte HP / stats / IDs are BIG-ENDIAN. Read via readGB() (memory-map
// descriptors), which reaches WRAM banks, I/O (rDIV) and HRAM (the RNG bytes).

#include <cstdint>

namespace yellow {

// --- RNG (HRAM) + timing ---
inline constexpr uint16_t rDIV = 0xFF04;          // hardware divider (RNG source)
inline constexpr uint16_t hRandomAdd = 0xFFD3;    // RNG byte A
inline constexpr uint16_t hRandomSub = 0xFFD4;    // RNG byte B (DSum = Add+Sub)
inline constexpr uint16_t hFrameCounter = 0xFFD5; // decremented each V-blank

// --- Party (wPartyMon1 base 0xD16A, struct size 0x2C) ---
inline constexpr uint16_t wPartyCount = 0xD162;
inline constexpr uint16_t wPartySpecies = 0xD163; // 6 + 0xFF terminator
inline constexpr uint16_t wPartyMon1 = 0xD16A;    // species
inline constexpr uint16_t wPartyMon1HP = 0xD16B;    // 2, BE
inline constexpr uint16_t wPartyMon1Level = 0xD18B; // live level
inline constexpr uint16_t wPartyMon1MaxHP = 0xD18C; // 2, BE
inline constexpr uint16_t wPartyMon1DVs = 0xD185;   // 2 (Atk/Def, Spd/Spc)

// --- Battle: player active mon (wBattleMon base 0xD013) ---
inline constexpr uint16_t wBattleMonSpecies = 0xD013;
inline constexpr uint16_t wBattleMonHP = 0xD014;    // 2, BE
inline constexpr uint16_t wBattleMonDVs = 0xD01F;   // 2
inline constexpr uint16_t wBattleMonLevel = 0xD021;
inline constexpr uint16_t wBattleMonMaxHP = 0xD022; // 2, BE

// --- Battle: enemy mon (wEnemyMon base 0xCFE4 -- note 0xCxxx region) ---
inline constexpr uint16_t wEnemyMonSpecies = 0xCFE4;
inline constexpr uint16_t wEnemyMonHP = 0xCFE5;    // 2, BE
inline constexpr uint16_t wEnemyMonDVs = 0xCFF0;   // 2 (wild-encounter DV manip)
inline constexpr uint16_t wEnemyMonLevel = 0xCFF2;
inline constexpr uint16_t wEnemyMonMaxHP = 0xCFF3; // 2, BE

// --- Battle: move / damage / crit ---
inline constexpr uint16_t wPlayerMoveNum = 0xCFD1;
inline constexpr uint16_t wEnemyMoveNum = 0xCFCB;
inline constexpr uint16_t wCriticalHitOrOHKO = 0xD05D; // nonzero = crit/OHKO
inline constexpr uint16_t wDamage = 0xD0D6;            // 2, BE

// --- Overworld ---
inline constexpr uint16_t wCurMap = 0xD35D;
inline constexpr uint16_t wYCoord = 0xD360; // Y before X
inline constexpr uint16_t wXCoord = 0xD361;
inline constexpr uint16_t wCurMapTileset = 0xD366;
inline constexpr uint16_t wPlayerDirection = 0xD529;

// --- Identity ---
inline constexpr uint16_t wPlayerID = 0xD358; // 2, BE (trainer ID)

} // namespace yellow
