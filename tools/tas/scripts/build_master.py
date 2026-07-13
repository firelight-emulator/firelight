#!/usr/bin/env python3
# Generate the continuous E4 master input script (one compile from the Lorelei L100 anchor, no pokes).
import sys
L = []
def hold(n, btn): L.append(f"{n} {btn}")
def idle(n): L.append(f"{n} -")
def sweep(cycles=340):          # dense auto-A Psychic OHKO sweep
    for _ in range(cycles): hold(2, "A"); idle(13)
def defeat_and_route(around):   # B-mash closes the whole defeat dialogue (advances boxes + closes final, won't re-talk leader), then route to forward warp
    for _ in range(70): hold(2, "B"); idle(16)
    hold(8, around); idle(12)   # step around the center-column leader (LEFT or RIGHT)
    for _ in range(14): hold(8, "UP"); idle(10)
    idle(150)

phases = sys.argv[1:] if len(sys.argv) > 1 else ["lorelei","bruno","agatha","lance","champion","hof"]

if "lorelei" in phases:
    # anchor is at (5,5); Lorelei at (5,2). walk up, trigger, sweep, route LEFT around her
    hold(8,"UP"); idle(12); hold(8,"UP"); idle(12); hold(2,"A"); idle(50)
    sweep(); defeat_and_route("LEFT")
if "bruno" in phases:
    # entered at (4,3); Bruno at (5,2). RIGHT then UP to face him, trigger, sweep, route LEFT
    idle(50); hold(8,"RIGHT"); idle(15); hold(8,"UP"); idle(15); hold(2,"A"); idle(50)
    sweep(); defeat_and_route("LEFT")
if "agatha" in phases:
    idle(50); hold(8,"RIGHT"); idle(15); hold(8,"UP"); idle(15); hold(2,"A"); idle(50)
    sweep(); defeat_and_route("LEFT")
if "lance" in phases:
    # Lance's room is a tall maze: idle for the entry auto-walk, walk up ~40 tiles into his sight, trigger, sweep
    idle(80)
    for _ in range(40): hold(8,"UP"); idle(8)
    hold(2,"A"); idle(50)
    sweep()
    # transition to Champion: B-mash defeat, route LEFT around Lance, up to the Champion warp
    for _ in range(70): hold(2,"B"); idle(16)
    hold(8,"LEFT"); idle(12)
    for _ in range(10): hold(8,"UP"); idle(10)
    idle(200)
if "champion" in phases:
    # PLAYER_ENTERS auto-fired on entry (Agatha armed it); advance BLUE intro + sweep 6 mons (tanky ones take 2 hits)
    idle(120)
    sweep(520)
    for _ in range(90): hold(2,"A"); idle(40)   # OAK victory sequence -> player-follows-Oak walks into the Hall of Fame
if "hof" in phases:
    for _ in range(140): hold(2,"A"); idle(45)  # Hall of Fame registration (all 6) + credits

open("master.txt","w").write("\n".join(L)+"\n")
print(f"wrote master.txt ({len(L)} lines) phases={phases}")
