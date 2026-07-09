# Firelight Development Skills

Common commands and workflows for developing Firelight on Windows with MSYS2/MinGW64.

## Build (mingw64-debug preset)

```bash
C:\msys64\msys2_shell.cmd -defterm -here -no-start -mingw64 -use-full-path -c "cd /c/Users/nicho/Documents/GitHub/firelight && cmake --build cmake-build-debug"
```

The `mingw64-debug` CMake preset outputs to `cmake-build-debug/` using GCC from `C:/msys64/mingw64/bin`.

## Run

```bash
C:\msys64\msys2_shell.cmd -defterm -here -no-start -mingw64 -use-full-path -c "cd /c/Users/nicho/Documents/GitHub/firelight && ./cmake-build-debug/firelight.exe"
```

## Configure (first time or after CMakeLists changes)

```bash
C:\msys64\msys2_shell.cmd -defterm -here -no-start -mingw64 -use-full-path -c "cd /c/Users/nicho/Documents/GitHub/firelight && cmake --preset=mingw64-debug"
```

## Run Tests

```bash
C:\msys64\msys2_shell.cmd -defterm -here -no-start -mingw64 -use-full-path -c "cd /c/Users/nicho/Documents/GitHub/firelight/cmake-build-debug && ctest --output-on-failure"
```

Run a specific test by name:
```bash
C:\msys64\msys2_shell.cmd -defterm -here -no-start -mingw64 -use-full-path -c "cd /c/Users/nicho/Documents/GitHub/firelight/cmake-build-debug && ctest -R <test_name> --output-on-failure"
```

## Other Presets

| Preset | Compiler | Output Dir | Use Case |
|---|---|---|---|
| `mingw64-debug` | GCC (MSYS2) | `cmake-build-debug/` | Primary local dev |
| `debug-win` | Clang | `build/debug-win/` | Debug with clang |
| `release-win` | Clang | `build/release-win/` | Release + packaging |
