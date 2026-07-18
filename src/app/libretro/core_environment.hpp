#pragma once

#include "core.hpp"

namespace libretro {

// Shared by core.cpp (which defines them) and core_environment.cpp (which holds
// the environment-call dispatch table + microphone callbacks and reads them)

// The active core's callback context: set on load, cleared on unload
extern CoreCallbackContext *g_ctx;

// libretro logging callback handed to cores via GET_LOG_INTERFACE
void log(enum retro_log_level level, const char *fmt, ...);

} // namespace libretro
