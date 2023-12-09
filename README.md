# Firelight

libs:
Dear ImGui
SDL2
GLEW
OpenSSL
spdlog
freetype2
libretro

1. load the game and get system av info before you refresh the hw context

### On game_geometry max_width vs base_width

BiscuitCakes — Today at 12:17 PM
Anyone know of any situations where a core's game_geometry.base_width would differ from game_geometry.max_width (same
for height)?
JesseTG — Today at 12:19 PM
When an emulator supports upscaling the game's resolution.
BiscuitCakes — Today at 12:20 PM
Ahhh. That makes sense. It'd be nice if it gave me the current values instead of just the range... unless it does and
I'm missing it (or forgot that I saw it)
JesseTG — Today at 12:20 PM
Or if the core draws additional information (say, a custom HUD) outside the emulated screen's area but within the
libretro screen, and this position is customizable
BiscuitCakes — Today at 12:21 PM
That's interesting. I think I might cross that one when I get to it. For now I guess I need to make my software
framebuffer max_width*max_height 😵‍💫
JesseTG — Today at 12:21 PM
Yes, that's the intent. And then you only blit the area given by base_width * base_height
Ooh, here's another scenario; when a core optionally can render two instances of the emulator side-by-side. Like, say,
with one of the Game Boy emulators; it emulates the link cable by emulating two Game Boys and synchronizing their
state (it's the only way to accommodate the link cable's latency requirements)
Here's another scenario; when you can change the layout of the emulated screen(s). Like for melonDS DS, you can lay out
both screens vertically, horizontally, scaled in various ways... And for the Wonderswan, the original hardware was
intended to be held in two orientations, so that's another layout change.
Psyraven — Today at 12:24 PM
In DOSBox (and other computer cores I assume) the emulated screen resolution varies widely. max is just the biggest
supported emulated monitor resolution.
BiscuitCakes — Today at 12:25 PM
Oof glad I asked. Thank you guys for the info, really appreciate it 👍
JesseTG — Today at 12:25 PM
Basically any scenario that would involve adjusting the screen resolution. Luckily, it's often not too difficult for a
core to compute the max_width and max_height.

### freetype

had to compile myself

```
git clone https://git.savannah.nongnu.org/git/freetype/freetype2.git
cd freetype2
mkdir build && cd build
cmake ..      # generates Makefile + deactivates HarfBuzz if not found
make          # compile libs
make install  # install libs & headers
```