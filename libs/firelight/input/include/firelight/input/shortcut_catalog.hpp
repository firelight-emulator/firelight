#pragma once

namespace firelight::input {

// Registers the built-in shortcut actions into the global ShortcutRegistry.
// Called once at startup. Trigger bindings are configured per profile.
void registerDefaultShortcuts();

} // namespace firelight::input
