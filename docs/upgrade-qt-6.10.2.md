# Plan: Upgrade Qt6 to 6.10.2

## Background

The project currently requires Qt 6.8 as its minimum version (`find_package(Qt6 6.8 REQUIRED ...)` and `qt6_standard_project_setup(REQUIRES 6.8)` in `CMakeLists.txt`). The codebase already contains a partial guard for Qt 6.10 behaviour around `GuiPrivate`. This document covers all changes needed to build cleanly against Qt 6.10.2.

---

## Step 1 – Install Qt 6.10.2 via MSYS2

Qt packages in MSYS2 are managed through `pacman`. Upgrade Qt and its tooling:

```bash
# Inside an MSYS2 MinGW64 shell
pacman -Syu
pacman -S mingw-w64-x86_64-qt6 mingw-w64-x86_64-qt6-tools
```

Verify the installed version:

```bash
qmake6 --version   # or: qtpaths-qt6 --qt-version
```

If you use a standalone Qt Online Installer instead of MSYS2 packages, install to e.g. `C:/Qt/Qt-6.10.2` and update the `CMAKE_PREFIX_PATH` accordingly (see Step 3).

---

## Step 2 – Fix the `Qt6::GuiPrivate` Link Issue

**File:** `CMakeLists.txt`

Currently `Qt6::GuiPrivate` is found conditionally (only when `Qt6_VERSION >= 6.10.0`) but linked **unconditionally**. This will produce a CMake error on Qt < 6.10 and a silent undefined-target error otherwise. Align the link with the conditional find:

**Current code (lines 74–78 and 233):**
```cmake
if (Qt6_VERSION VERSION_GREATER_EQUAL 6.10.0)
    find_package(Qt6 OPTIONAL_COMPONENTS GuiPrivate)
endif()
...
target_link_libraries(firelight_lib PUBLIC
    ...
    Qt6::GuiPrivate
    ...
)
```

**Change to:**
```cmake
if (Qt6_VERSION VERSION_GREATER_EQUAL 6.10.0)
    find_package(Qt6 OPTIONAL_COMPONENTS GuiPrivate)
endif()
...
target_link_libraries(firelight_lib PUBLIC
    ...
    # GuiPrivate only available as a named component from 6.10 onward
    $<$<TARGET_EXISTS:Qt6::GuiPrivate>:Qt6::GuiPrivate>
    ...
)
```

This generator expression links `Qt6::GuiPrivate` only when the target actually exists, making the build safe across all versions.

---

## Step 3 – Update CMake Version Requirements

**File:** `CMakeLists.txt`

Bump the required Qt version and the `qt6_standard_project_setup` call to match 6.10:

```cmake
# Change:
find_package(Qt6 6.8 REQUIRED COMPONENTS ...)
qt6_standard_project_setup(REQUIRES 6.8)

# To:
find_package(Qt6 6.10 REQUIRED COMPONENTS ...)
qt6_standard_project_setup(REQUIRES 6.10)
```

---

## Step 4 – Update the Hard-Coded Qt Install Path

**File:** `CMakeLists.txt`, line 307

The `install()` command references the old Qt 6.9.0 bin directory for `RUNTIME_DEPENDENCIES`:

```cmake
# Change:
DIRECTORIES ${SHARED_LIB_DIR} ${QT_PLUGIN_DIR} "C:/Qt/Qt-6.9.0/bin"

# To (if using standalone installer):
DIRECTORIES ${SHARED_LIB_DIR} ${QT_PLUGIN_DIR} "C:/Qt/Qt-6.10.2/bin"
```

If you are using MSYS2-supplied Qt exclusively (where Qt lives in `C:/msys64/mingw64/bin`), remove this hard-coded path entirely — `SHARED_LIB_DIR` already points to the MSYS2 bin directory:

```cmake
DIRECTORIES ${SHARED_LIB_DIR} ${QT_PLUGIN_DIR}
```

---

## Step 5 – Check for Removed/Deprecated API

Qt 6.10 removes APIs that were deprecated before Qt 6.0. Run a build with deprecation warnings treated as errors to surface anything that needs updating:

```bash
# Add to CMakeLists.txt temporarily while auditing:
target_compile_definitions(firelight_lib PRIVATE QT_DISABLE_DEPRECATED_BEFORE=0x060A00)
```

Known areas to audit in this codebase:

| Area | What to check |
|---|---|
| `emulator_item_renderer.cpp` | `QRhiVulkanNativeHandles` — field names (`gfxQueueIdx`, `physDev`) were stable through 6.9; verify they haven't changed in 6.10 release notes |
| `emulator_item_renderer.cpp` | `QRhiCommandBuffer::ExternalContent` flag — confirm still present |
| Any `QAbstractSocket` usage | `error()` signal was renamed to `errorOccurred()` (deprecated since 5.15) |
| Any `QDesktopWidget` usage | Removed in Qt 6.0 (should already be gone) |
| `resources.qrc` | Qt 6.10 prefers `qt_add_resources` CMake API over `.qrc` files; not a hard break but worth noting |

Remove the temporary compile definition after auditing.

---

## Step 6 – Update CMakePresets.json (Optional)

**File:** `CMakePresets.json`

The `mingw64-debug` preset uses the MSYS2 GCC toolchain, which picks up whatever Qt version is installed system-wide — no changes needed there. If you add a preset that points to a standalone Qt install, add the path:

```json
{
    "name": "debug-win-qt610",
    "displayName": "Debug Qt 6.10 (standalone)",
    "inherits": "debug-win",
    "cacheVariables": {
        "CMAKE_PREFIX_PATH": "C:/Qt/Qt-6.10.2/mingw_64"
    }
}
```

---

## Step 7 – Update CI Workflow (GitHub Actions)

**File:** `.github/workflows/build-win64-x86.yml`

The workflow installs `qt6:p` via MSYS2's `pacboy`, which resolves to the latest available Qt6 in the MSYS2 repo. No version pin is set, so once MSYS2 ships 6.10.2 the CI build will automatically use it. No changes are strictly required.

If you want to pin the version explicitly, use pacman's versioned syntax:

```yaml
pacboy: >-
  qt6:p>=6.10.2
```

---

## Step 8 – Verify the Build

After making the above changes, reconfigure and build:

```bash
# Remove the old build directory to avoid stale CMake cache
rm -rf cmake-build-debug

# Reconfigure
C:\msys64\msys2_shell.cmd -defterm -here -no-start -mingw64 -use-full-path -c \
  "cd /c/Users/nicho/Documents/GitHub/firelight && cmake --preset=mingw64-debug"

# Build
C:\msys64\msys2_shell.cmd -defterm -here -no-start -mingw64 -use-full-path -c \
  "cd /c/Users/nicho/Documents/GitHub/firelight && cmake --build cmake-build-debug"

# Run tests
C:\msys64\msys2_shell.cmd -defterm -here -no-start -mingw64 -use-full-path -c \
  "cd /c/Users/nicho/Documents/GitHub/firelight/cmake-build-debug && ctest --output-on-failure"
```

---

## Summary of File Changes

| File | Change |
|---|---|
| `CMakeLists.txt` | Bump required Qt version 6.8 → 6.10 in `find_package` and `qt6_standard_project_setup` |
| `CMakeLists.txt` | Fix `Qt6::GuiPrivate` to use generator expression so it links conditionally |
| `CMakeLists.txt` | Update or remove hard-coded `C:/Qt/Qt-6.9.0/bin` install path |
| `CMakePresets.json` | Add new preset if targeting standalone Qt 6.10.2 install (optional) |
| `.github/workflows/build-win64-x86.yml` | No changes required; MSYS2 `qt6:p` tracks latest |
| Source files | Fix any deprecated API identified in Step 5 |
