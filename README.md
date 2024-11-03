# Firelight Emulator

Firelight is a libretro-based emulation frontend that aims to be the easiest way to play your retro games, discover
awesome mods for those games, and just have a dang good time.

Plus, it's all in one, so you don't need to do anything but download the app, tell it where to find your games, and
start playing! Firelight takes care of the rest.

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing
purposes.

### Prerequisites

Here are the things you need to build the application:

- A C++17 compiler. I use Clang, but other stuff should work too... I think.
- CMake 3.22.1 or later.
- The following libraries:
    - Qt6 (6.7)
        - Components:
            - Quick
            - Gui
            - OpenGL
            - QuickControls2
            - Quick3D
            - Sql
    - SDL2
    - spdlog
    - GTest (currently required even without running tests, but this will be fixed later)

#### Recommended Windows setup using MinGW

1) Follow this guide to download MSYS2 https://github.com/HO-COOH/CPPDevOnWindows
2) Open MSYS2 and install the following using pacman:
```
pacman -S mingw-w64-x86_64-clang
pacman -S mingw-w64-x86_64-ninja
pacman -S mingw-w64-x86_64-gdb
pacman -S mingw-w64-x86_64-cmake
pacman -S mingw-w64-x86_64-qt6-base
pacman -S mingw-w64-x86_64-qt6-tools
pacman -S mingw-w64-x86_64-qt6-multimedia
pacman -S mingw-w64-x86_64-qt6-declarative
pacman -S mingw-w64-x86_64-qt6-quick3d
pacman -S mingw-w64-x86_64-qt6-charts
pacman -S mingw-w64-x86_64-qt6-svg
pacman -S mingw-w64-x86_64-spdlog
pacman -S mingw-w64-x86_64-cpr
pacman -S mingw-w64-x86_64-gtest
pacman -S mingw-w64-x86_64-nlohmann-json
```
3) In Windows, edit your PATH environment variable to add your MinGW MSYS2 Path (should be similar to C:\msys64\mingw64\bin)
4) In your preferred IDE (CLion is what we used), use the debug-full-mingw CMakePreset to run cmake.
5) Run the build and start firelight.exe
6) ***IMPORTANT*** - If you get a white screen:
   - Go to Graphics Settings in Windows
   - Add Firelight.exe
   - Instruct it to use your graphics card

[//]: # (### Installing)

[//]: # ()

[//]: # (A step by step series of examples that tell you how to get a development env running)

[//]: # ()

[//]: # (Say what the step will be)

[//]: # ()

[//]: # (```)

[//]: # (Give the example)

[//]: # (```)

[//]: # ()

[//]: # (And repeat)

[//]: # ()

[//]: # (```)

[//]: # (until finished)

[//]: # (```)

[//]: # ()

[//]: # (End with an example of getting some data out of the system or using it for a little demo)

[//]: # ()

[//]: # (## Running the tests)

[//]: # ()

[//]: # (Explain how to run the automated tests for this system)

[//]: # ()

[//]: # (### Break down into end to end tests)

[//]: # ()

[//]: # (Explain what these tests test and why)

[//]: # ()

[//]: # (```)

[//]: # (Give an example)

[//]: # (```)

[//]: # ()

[//]: # (### And coding style tests)

[//]: # ()

[//]: # (Explain what these tests test and why)

[//]: # ()

[//]: # (```)

[//]: # (Give an example)

[//]: # (```)

[//]: # ()

[//]: # (## Deployment)

[//]: # ()

[//]: # (Add additional notes about how to deploy this on a live system)

## Built With

* [Qt6](https://contribute.qt-project.org/) - The GUI framework used
* [libretro](https://github.com/libretro) - The emulation library used (will list specific cores later)

## Contributing

I'm not currently accepting contributions, but in the future I will!

[//]: # (Please read [CONTRIBUTING.md]&#40;https://gist.github.com/PurpleBooth/b24679402957c63ec426&#41; to see the process for)

[//]: # (submitting pull requests.)

[//]: # (## Versioning)

[//]: # ()

[//]: # (We use [SemVer]&#40;http://semver.org/&#41; for versioning. For the versions available, see)

[//]: # (the [tags on this repository]&#40;https://github.com/your/project/tags&#41;.)

## Authors

* [BiscuitCakes](https://github.com/biscuitcakes)

[//]: # (See also the list of [contributors]&#40;https://github.com/your/project/contributors&#41; who participated in this project.)

## License

This project is licensed under the GPL-3.0 License - see the [LICENSE.md](LICENSE.md) file for details

## Acknowledgments

* The libretro team for making such a great library and for putting up with my hundreds, nay, thousands of questions
* The romhack authors who are letting me include their mods in the app in these early days

[//]: # (### Installation on Windows)

[//]: # ()

[//]: # (1. Download and install [MSYS2]&#40;Download and install MSYS2: https://github.com/msys2/msys2-installer/releases&#41;)

[//]: # (2. Open a MSYS2 terminal and run the following commands:)

[//]: # (    * pacman -S mingw-w64-x86_64-qt6 &#40;hit enter for all&#41;)

[//]: # (    * pacman -S mingw-w64-x86_64-SDL2)

[//]: # (    * pacman -S mingw-w64-x86_64-glew)

[//]: # (    * pacman -S mingw-w64-x86_64-freetype)

[//]: # (    * pacman -S mingw-w64-x86_64-gtest)

[//]: # (    * pacman -S mingw-w64-x86_64-cmake)

[//]: # (    * pacman -S mingw-w64-x86_64-gcc)

[//]: # (    * pacman -S make gettext base-devel)

[//]: # (    * pacman -S mingw-w64-x86_64-make)

[//]: # (    * pacman -S mingw-w64-x86_64-spdlog)

[//]: # (3. Go to your MSYS2 bin folder and copy mingw32-make.exe as make.exe)

[//]: # (4. Add the MSYS2 directory to the PATH environment variable in Windows &#40;mine was C:\msys64\mingw64\bin&#41;)

[//]: # (5. Make a folder called build in the root directory of the project)

[//]: # (6. Open a Linux Shell and run the following commands &#40;start in the build directory&#41;)

[//]: # (    * apt install cmake)

[//]: # (    * apt install make)

[//]: # (    * cmake -G "MinGW Makefiles" ..)

[//]: # (    * make)