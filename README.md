# Firelight

One Paragraph of project description goes here

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing
purposes.

### Prerequisites

What things you need to install the software and how to install them

```
Give examples
```

### Installing

A step by step series of examples that tell you how to get a development env running

Say what the step will be

```
Give the example
```

And repeat

```
until finished
```

End with an example of getting some data out of the system or using it for a little demo

## Running the tests

Explain how to run the automated tests for this system

### Break down into end to end tests

Explain what these tests test and why

```
Give an example
```

### And coding style tests

Explain what these tests test and why

```
Give an example
```

## Deployment

Add additional notes about how to deploy this on a live system

## Built With

* [Dropwizard](http://www.dropwizard.io/1.0.2/docs/) - The web framework used
* [Maven](https://maven.apache.org/) - Dependency Management
* [ROME](https://rometools.github.io/rome/) - Used to generate RSS Feeds

## Contributing

Please read [CONTRIBUTING.md](https://gist.github.com/PurpleBooth/b24679402957c63ec426) for details on our code of
conduct, and the process for submitting pull requests to us.

## Versioning

We use [SemVer](http://semver.org/) for versioning. For the versions available, see
the [tags on this repository](https://github.com/your/project/tags).

## Authors

* **Billie Thompson** - *Initial work* - [PurpleBooth](https://github.com/PurpleBooth)

See also the list of [contributors](https://github.com/your/project/contributors) who participated in this project.

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

## Acknowledgments

* Hat tip to anyone whose code was used
* Inspiration
* etc

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

### Installation on Windows

1. Download and install [MSYS2](Download and install MSYS2: https://github.com/msys2/msys2-installer/releases)
2. Open a MSYS2 terminal and run the following commands:
    * pacman -S mingw-w64-x86_64-qt6 (hit enter for all)
    * pacman -S mingw-w64-x86_64-SDL2
    * pacman -S mingw-w64-x86_64-openssl
    * pacman -S mingw-w64-x86_64-glew
    * pacman -S mingw-w64-x86_64-freetype
    * pacman -S mingw-w64-x86_64-gtest
    * pacman -S mingw-w64-x86_64-cmake
    * pacman -S mingw-w64-x86_64-gcc
    * pacman -S make gettext base-devel
    * pacman -S mingw-w64-x86_64-make
    * pacman -S mingw-w64-x86_64-spdlog
3. Go to your MSYS2 bin folder and copy mingw32-make.exe as make.exe
4. Add the MSYS2 directory to the PATH environment variable in Windows (mine was C:\msys64\mingw64\bin)
5. Make a folder called build in the root directory of the project
6. Open a Linux Shell and run the following commands (start in the build directory)
    * apt install cmake
    * apt install make
    * cmake -G "MinGW Makefiles" ..
    * make