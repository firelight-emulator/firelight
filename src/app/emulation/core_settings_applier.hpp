#pragma once

#include "emulation_context.hpp"

#include <firelight/event_dispatcher.hpp>
#include <string>

namespace firelight::emulation {

class EmulatorInstance;

// Resolves this game's common settings (game -> platform -> global -> catalog
// default) and applies them to the EmulatorInstance, refreshing whenever a
// change at any tier could affect the effective value
class CoreSettingsApplier {
public:
  CoreSettingsApplier(EmulatorInstance &instance,
                      const EmulationContext &context, std::string contentHash,
                      int platformId);

  void refresh();

private:
  EmulatorInstance &m_instance;
  const EmulationContext &m_context;
  std::string m_contentHash;
  int m_platformId;

  ScopedConnection m_platformSettingChangedConnection;
  ScopedConnection m_gameSettingChangedConnection;
  ScopedConnection m_globalSettingChangedConnection;
};

} // namespace firelight::emulation
