#pragma once

#include <set>
#include <string>

namespace firelight::platforms {
class IPlatformService;
}

namespace firelight::library {

/**
 * Every extension the scanner will take a file in for: the disc containers plus every platform's file associations
 */
[[nodiscard]] std::set<std::string> acceptedExtensions(const platforms::IPlatformService &platformService);

} // namespace firelight::library
