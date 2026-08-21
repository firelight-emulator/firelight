#include <firelight/library/accepted_extensions.hpp>
#include <firelight/library/content_extensions.hpp>
#include <firelight/platforms/platform_service.hpp>

namespace firelight::library {

std::set<std::string> acceptedExtensions(const platforms::IPlatformService &platformService) {
  std::set<std::string> extensions(DISC_EXTENSIONS.begin(), DISC_EXTENSIONS.end());

  for (const auto &platform : platformService.listPlatforms()) {
    extensions.insert(platform.fileAssociations.begin(), platform.fileAssociations.end());
  }

  return extensions;
}

} // namespace firelight::library
