#pragma once

#include <set>
#include <string>

namespace firelight::platforms {
class IPlatformService;
}

namespace firelight::library {

/**
 * Every extension the scanner will take a file in for.
 *
 * The disc containers plus every platform's file associations. One expression rather than two
 * conditions at the gate, so what the scanner accepts can be enumerated and asked whether
 * anything can actually identify it
 */
[[nodiscard]] std::set<std::string> acceptedExtensions(const platforms::IPlatformService &platformService);

} // namespace firelight::library
