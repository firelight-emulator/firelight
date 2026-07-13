#!/usr/bin/env python3
# Construct a Pokemon Yellow .sav (32 KB) that boots to CONTINUE at the Indigo
# Plateau lobby with a balanced Lv80 max-DV team + all badges + Full Restores.
# Offsets/encoding from disassembly research (byte-identical to Red/Blue for .sav).
import sys, math, os

SAV = bytearray(32 * 1024)  # zero-filled
_base = os.environ.get("BASE_SAV")
if _base:  # patch a real in-game save instead of building from zeros
    with open(_base, "rb") as f:
        SAV = bytearray(f.read())
    assert len(SAV) == 32 * 1024, "base save must be 32 KB"
    # sanity: our checksum algo must reproduce the base's stored checksum
    _chk = (~(sum(SAV[0x2598:0x3523]) & 0xFF)) & 0xFF
    print(f"  base '{_base}' stored checksum=0x{SAV[0x3523]:02X} recompute=0x{_chk:02X} match={_chk==SAV[0x3523]}")

# ---- Gen-1 char encoding ----
def name_bytes(s, length=11):
    out = bytearray()
    for ch in s:
        if 'A' <= ch <= 'Z': out.append(0x80 + (ord(ch) - ord('A')))
        elif 'a' <= ch <= 'z': out.append(0xA0 + (ord(ch) - ord('a')))
        elif '0' <= ch <= '9': out.append(0xF6 + (ord(ch) - ord('0')))
        else: out.append(0x50)
    out.append(0x50)  # terminator
    while len(out) < length: out.append(0x50)
    return bytes(out[:length])

def w8(off, v): SAV[off] = v & 0xFF
def w16be(off, v): SAV[off] = (v >> 8) & 0xFF; SAV[off+1] = v & 0xFF
def w24be(off, v): SAV[off] = (v>>16)&0xFF; SAV[off+1]=(v>>8)&0xFF; SAV[off+2]=v&0xFF
def wbytes(off, b): SAV[off:off+len(b)] = b

LEVEL = 100
DV = 15                 # max DVs -> DV word 0xFFFF, HP_DV = 15
STATEXP_TERM = 63       # floor(sqrt(65535)/4): the max-StatExp bonus to the pre-level stat term
OTID = 0x2536

def stat(base, lvl=LEVEL, hp=False):
    # Gen-1 stat with max StatExp (65535) baked in
    core = ((base + DV) * 2 + STATEXP_TERM) * lvl // 100
    return core + (lvl + 10 if hp else 5)

# species: (internal_idx, baseHP, atk, def, spd, spc, catchRate, type1, type2, nick, moves[4], pp[4])
# STARMIE lead: PSYCHIC in slot 1 (STAB, neutral+no-immunity on the whole gauntlet) for the auto-A OHKO sweep
TEAM = [
    ("STARMIE",   0x98, 60, 75, 85,115,100, 0x3C, 0x15,0x18, [0x5E,0x3A,0x55,0x69], [10,10,15,20]),
    ("TAUROS",    0x3C, 75,100, 95,110, 70, 0x2D, 0x00,0x00, [0x22,0x3F,0x59,0x3B], [15, 5,10, 5]),
    ("LAPRAS",    0x13,130, 85, 80, 60, 95, 0x2D, 0x15,0x19, [0x3B,0x39,0x55,0x22], [ 5,15,15,15]),
    ("EXEGGUTOR", 0x0A, 95, 95, 85, 55,125, 0x2D, 0x16,0x18, [0x4F,0x5E,0x17,0x99], [15,10,20, 5]),
    ("RHYDON",    0x01,105,130,120, 40, 45, 0x3C, 0x04,0x05, [0x59,0x9D,0x22,0xA4], [10,10,15,10]),
    ("PIKACHU",   0x54, 35, 55, 30, 90, 50, 0xBE, 0x17,0x17, [0x55,0x57,0x56,0x62], [15,10,20,30]),
]

# ---- main data fields ----
wbytes(0x2598, name_bytes("YELLOW"))     # player name
w8(0x25C9, 1)                            # wNumBagItems
wbytes(0x25CA, bytes([0x10, 30, 0xFF]))  # bag: Full Restore x30, terminator
w24be(0x25F3, 0x999999)                  # money (BCD)
w8(0x2601, 0x41)                         # wOptions: BIT_BATTLE_SHIFT(0x40)=SET style + fast text(0x01)
w8(0x2602, 0xFF)                         # all 8 badges
w16be(0x2605, OTID)                      # player/trainer ID
w8(0x29C1, 0x01)                         # wRivalStarter = 1 (Jolteon Champion variant); skipped by crafted save
CURMAP = int(sys.argv[2], 0) if len(sys.argv) > 2 else 0xAE
CY = int(sys.argv[3], 0) if len(sys.argv) > 3 else 9
CX = int(sys.argv[4], 0) if len(sys.argv) > 4 else 7
w8(0x260A, CURMAP)                       # wCurMap
w8(0x260D, CY)                           # wYCoord
w8(0x260E, CX)                           # wXCoord
w8(0x260F, CY // 2)                      # wYBlockCoord
w8(0x2610, CX // 2)                      # wXBlockCoord
# CONTINUE reuses the saved map-header state, so patch tileset/height/width to the
# TARGET map's values (else the map is rebuilt with the base save's dims -> corrupt).
_ts = os.environ.get("TILESET"); _h = os.environ.get("HGT"); _w = os.environ.get("WID")
if _ts is not None: w8(0x2613, int(_ts, 0))   # wCurMapTileset (WRAM 0xD366)
if _h  is not None: w8(0x2614, int(_h, 0))     # wCurMapHeight in blocks (0xD367)
if _w  is not None: w8(0x2615, int(_w, 0))     # wCurMapWidth in blocks  (0xD368)

# ---- party ----
PARTY_BASE = 0x2F34
STRUCT = 0x2C
w8(0x2F2C, len(TEAM))                                  # wPartyCount
wbytes(0x2F2D, bytes([m[1] for m in TEAM]) + b"\xFF")  # species list + terminator
for i, (nick, idx, hp, atk, df, spd, spc, catch, t1, t2, moves, pp) in enumerate(TEAM):
    b = PARTY_BASE + i * STRUCT
    mhp = stat(hp, hp=True); ma = stat(atk); md = stat(df); ms = stat(spd); mc = stat(spc)
    w8(b + 0x00, idx)
    w16be(b + 0x01, mhp)          # current HP = full
    w8(b + 0x03, LEVEL)           # box level
    w8(b + 0x04, 0)               # status
    w8(b + 0x05, t1); w8(b + 0x06, t2)
    w8(b + 0x07, catch)
    wbytes(b + 0x08, bytes(moves))
    w16be(b + 0x0C, OTID)
    w24be(b + 0x0E, 1250000)      # exp >= Slow-group L100 (1.25*L^3) so a KO can't recompute level down
    wbytes(b + 0x11, b"\xFF" * 10)  # StatExp (0x11..0x1A) = max (65535 each) -> +63 stat term
    w8(b + 0x1B, 0xFF); w8(b + 0x1C, 0xFF)  # DVs = max
    wbytes(b + 0x1D, bytes(pp))
    w8(b + 0x21, LEVEL)           # live level
    w16be(b + 0x22, mhp); w16be(b + 0x24, ma); w16be(b + 0x26, md); w16be(b + 0x28, ms); w16be(b + 0x2A, mc)
    wbytes(0x303C + i * 11, name_bytes("YELLOW"))  # OT name
    wbytes(0x307E + i * 11, name_bytes(nick))      # nickname
    print(f"  {nick:10s} idx=0x{idx:02X} Lv{LEVEL}  HP={mhp} Atk={ma} Def={md} Spd={ms} Spc={mc}")

# ---- checksum: ones-complement of byte sum over 0x2598..0x3522 inclusive ----
s = sum(SAV[0x2598:0x3523]) & 0xFF
SAV[0x3523] = (~s) & 0xFF
print(f"checksum = 0x{SAV[0x3523]:02X} (sum&0xFF=0x{s:02X})")

out = sys.argv[1] if len(sys.argv) > 1 else "pre_e4.sav"
with open(out, "wb") as f: f.write(SAV)
print(f"wrote {out} ({len(SAV)} bytes)")
