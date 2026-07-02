#pragma once

#include <firelight/settings/core_option.hpp>

#include <string>
#include <vector>

namespace firelight::settings {

// Persists the raw option definitions a core declares at load, keyed by core
// name, so the advanced options editor can list them before the core runs.
class ICoreOptionRepository {
public:
  virtual ~ICoreOptionRepository() = default;

  // Replaces the full set of declared options cached for a core (options a core
  // no longer declares are removed). Preserves the given order.
  virtual void upsertCoreOptions(const std::string &coreName,
                                 const std::vector<CoreOption> &options) = 0;

  // The cached options for a core in declaration order, or empty if none.
  [[nodiscard]] virtual std::vector<CoreOption>
  getCoreOptions(const std::string &coreName) = 0;
};

} // namespace firelight::settings
