#!/usr/bin/env python3
# Emit a WRAM poke file that overwrites the party with the crafted L100 max team,
# plus wRivalStarter=1 and wOptions=SET. One byte per line (the poke tool takes 1 byte/line).
import sys

LEVEL = 100
DV = 15
STATEXP_TERM = 63                 # max StatExp (65535) bonus
OTID = 0x2536
def stat(base, hp=False):
    core = ((base + DV) * 2 + STATEXP_TERM) * LEVEL // 100
    return core + (LEVEL + 10 if hp else 5)

# name encoding (A=0x80..Z=0x99, terminator 0x50)
def name_bytes(s, length=11):
    out = bytearray()
    for ch in s:
        if 'A' <= ch <= 'Z': out.append(0x80 + ord(ch) - ord('A'))
        elif 'a' <= ch <= 'z': out.append(0xA0 + ord(ch) - ord('a'))
        elif '0' <= ch <= '9': out.append(0xF6 + ord(ch) - ord('0'))
        else: out.append(0x50)
    out.append(0x50)
    while len(out) < length: out.append(0x50)
    return bytes(out[:length])

# (nick, idx, baseHP,atk,def,spd,spc, catch, t1,t2, moves[4], pp[4])  -- STARMIE lead, PSYCHIC slot1
TEAM = [
    ("STARMIE",   0x98, 60, 75, 85,115,100, 0x3C, 0x15,0x18, [0x5E,0x3A,0x55,0x69], [63,40,30,20]),  # Psychic PP=63 (game uses current-PP byte; needs to last all 26 gauntlet mons)
    ("TAUROS",    0x3C, 75,100, 95,110, 70, 0x2D, 0x00,0x00, [0x22,0x3F,0x59,0x3B], [15, 5,10, 5]),
    ("LAPRAS",    0x13,130, 85, 80, 60, 95, 0x2D, 0x15,0x19, [0x3B,0x39,0x55,0x22], [ 5,15,15,15]),
    ("EXEGGUTOR", 0x0A, 95, 95, 85, 55,125, 0x2D, 0x16,0x18, [0x4F,0x5E,0x17,0x99], [15,10,20, 5]),
    ("RHYDON",    0x01,105,130,120, 40, 45, 0x3C, 0x04,0x05, [0x59,0x9D,0x22,0xA4], [10,10,15,10]),
    ("PIKACHU",   0x54, 35, 55, 30, 90, 50, 0xBE, 0x17,0x17, [0x55,0x57,0x56,0x62], [15,10,20,30]),
]

# WRAM addresses (Gen-1 Yellow)
wPartyCount   = 0xD162
wPartySpecies = 0xD163      # 6 species + 0xFF
wPartyMon1    = 0xD16A      # struct 0x2C
wPartyMonOT   = 0xD273      # 6 x 11
wPartyMonNick = 0xD2B5      # 6 x 11
STRUCT = 0x2C

lines = []
def put(addr, data):
    for i, b in enumerate(data):
        lines.append("0x%04X: %02X" % (addr + i, b & 0xFF))

def be16(v): return bytes([(v >> 8) & 0xFF, v & 0xFF])
def be24(v): return bytes([(v >> 16) & 0xFF, (v >> 8) & 0xFF, v & 0xFF])

put(wPartyCount, bytes([len(TEAM)]))
put(wPartySpecies, bytes([m[1] for m in TEAM]) + b"\xFF")
for i, (nick, idx, hp, atk, df, spd, spc, catch, t1, t2, moves, pp) in enumerate(TEAM):
    mhp = stat(hp, hp=True); ma = stat(atk); md = stat(df); ms = stat(spd); mc = stat(spc)
    s = bytearray(STRUCT)
    s[0x00] = idx
    s[0x01:0x03] = be16(mhp)          # current HP = full
    s[0x03] = LEVEL
    s[0x04] = 0                        # status
    s[0x05] = t1; s[0x06] = t2
    s[0x07] = catch
    s[0x08:0x0C] = bytes(moves)
    s[0x0C:0x0E] = be16(OTID)
    s[0x0E:0x11] = be24(1250000)       # exp >= Slow-group L100
    s[0x11:0x1B] = b"\xFF" * 10        # StatExp max
    s[0x1B] = 0xFF; s[0x1C] = 0xFF      # DVs max
    s[0x1D:0x21] = bytes(pp)
    s[0x21] = LEVEL
    s[0x22:0x24] = be16(mhp); s[0x24:0x26] = be16(ma); s[0x26:0x28] = be16(md)
    s[0x28:0x2A] = be16(ms); s[0x2A:0x2C] = be16(mc)
    put(wPartyMon1 + i * STRUCT, s)
    put(wPartyMonOT + i * 11, name_bytes("YELLOW"))
    put(wPartyMonNick + i * 11, name_bytes(nick))

# seeds: wRivalStarter=1 (Jolteon), wOptions=SET+fast
put(0xD714, bytes([0x01]))
put(0xD354, bytes([0x41]))

open(sys.argv[1], "w").write("\n".join(lines) + "\n")
print("wrote %s (%d byte-pokes) -- STARMIE Spc=%d" % (sys.argv[1], len(lines), stat(100)))
