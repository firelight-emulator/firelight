#include "firelight/library/rc_hash_logging.hpp"

#include <mutex>
#include <rcheevos/rc_hash.h>
#include <spdlog/spdlog.h>

namespace firelight::library {

namespace {
// rcheevos reports a console candidate not matching through the same channel as a real read
// failure, and a disc walks up to nine candidates before one sticks
void onError(const char *message) { spdlog::debug("[rc_hash] {}", message); }

// One line per candidate tried, per file
void onVerbose(const char *message) { spdlog::trace("[rc_hash] {}", message); }
} // namespace

void installRcHashLogging() {
  static std::once_flag once;

  std::call_once(once, [] {
    rc_hash_init_error_message_callback(onError);
    rc_hash_init_verbose_message_callback(onVerbose);
  });
}

} // namespace firelight::library
