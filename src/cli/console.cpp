#include "cli/console.hpp"

#ifdef _WIN32
#include <cstdio>
#include <windows.h>
#endif

namespace firelight::cli {

void attachParentConsole() {
#ifdef _WIN32
  // Only attach when launched from an existing console; if none, this fails and
  // we leave stdio alone (a double-click launch stays windowless)
  if (AttachConsole(ATTACH_PARENT_PROCESS)) {
    FILE *stream = nullptr;
    freopen_s(&stream, "CONOUT$", "w", stdout);
    freopen_s(&stream, "CONOUT$", "w", stderr);
    freopen_s(&stream, "CONIN$", "r", stdin);
  }
#endif
}

} // namespace firelight::cli
