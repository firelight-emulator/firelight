#include <firelight/input/shortcut_catalog.hpp>

#include <firelight/input/shortcut_registry.hpp>

namespace firelight::input {
    void registerDefaultShortcuts() {
        auto &registry = ShortcutRegistry::instance();

        const auto add = [&registry](const char *id, const char *name,
                                     const char *category, ActivationType activation,
                                     int scope) {
            ShortcutAction action;
            action.id = id;
            action.displayName = name;
            action.category = category;
            action.activation = activation;
            action.scope = scope;
            registry.registerAction(action);
        };

        // Speed
        add("fast_forward", "Fast forward", "Speed", ActivationType::Hold,
            ScopeInGame);
        add("toggle_fast_forward", "Toggle fast forward", "Speed",
            ActivationType::Toggle, ScopeInGame);
        add("rewind", "Rewind", "Speed", ActivationType::Hold, ScopeInGame);
        add("slow_motion", "Slow motion", "Speed", ActivationType::Hold,
            ScopeInGame);
        add("speed_up", "Speed up", "Speed", ActivationType::Press, ScopeInGame);
        add("slow_down", "Slow down", "Speed", ActivationType::Press, ScopeInGame);

        // Save states
        add("save_state", "Save state", "Save States", ActivationType::Press,
            ScopeInGame);
        add("load_state", "Load state", "Save States", ActivationType::Press,
            ScopeInGame);
        add("state_slot_next", "Next save slot", "Save States", ActivationType::Press,
            ScopeInGame);
        add("state_slot_prev", "Previous save slot", "Save States",
            ActivationType::Press, ScopeInGame);

        // System
        add("open_rewind_menu", "Open rewind menu", "System", ActivationType::Press,
            ScopeInGame);
        add("open_quick_menu", "Open quick menu", "System", ActivationType::Press,
            ScopeInGame);
        add("frame_advance", "Frame advance", "System", ActivationType::Press,
            ScopeInGame);
        add("reset", "Reset game", "System", ActivationType::Press, ScopeInGame);
        add("pause", "Pause", "System", ActivationType::Toggle, ScopeInGame);
        add("exit_game", "Exit game", "System", ActivationType::Press, ScopeInGame);
        add("screenshot", "Screenshot", "System", ActivationType::Press,
            ScopeAlways);
        add("capture_clip", "Capture video clip", "System", ActivationType::Press,
            ScopeInGame);
        add("toggle_mute", "Mute", "System", ActivationType::Toggle, ScopeAlways);
        add("toggle_fullscreen", "Fullscreen", "System", ActivationType::Toggle,
            ScopeAlways);
    }
} // namespace firelight::input
