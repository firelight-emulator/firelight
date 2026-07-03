#pragma once

namespace firelight::cli {

// On Windows, attach to the parent process's console (if the app was launched
// from a terminal) and rebind stdout/stderr/stdin to it, so a GUI-subsystem
// build's CLI output (--help / --version / subcommand output) is actually
// visible. No-op on other platforms and when there is no parent console.
void attachParentConsole();

} // namespace firelight::cli
