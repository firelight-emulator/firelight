# Shortcut System

Shortcuts are the emulator hotkeys — fast-forward, rewind, save state, open menu,
screenshot, and so on. This document describes the design: how an action is
defined, how it's triggered, and how it's dispatched to whatever actually does
the work.

## The core split: global **Action**, per-profile **Trigger**, global **Dispatch**

Three decoupled pieces (mirrors the button-binding model, which is also
per-profile):

- **Action** — *global*. A catalog entry describing what a shortcut is, how it
  activates, and where it's allowed. There is exactly one `fast_forward`.
- **Trigger** — *per-profile*. Which buttons/keys fire it, bound on each
  `GamepadProfile` using the inputs that profile's controller actually has.
- **Dispatch** — *global*. A `ShortcutId → handler` registry in the app that
  runs the behavior.

The action is singular; every device triggers it its own way. Because triggers
live on the profile, each device evaluates **its own** profile's shortcuts —
there is no "which controller wins" ambiguity, and oddball controllers (an N64
pad with no L3/R3) bind against the buttons they have.

```mermaid
flowchart LR
    subgraph Global["Global (app)"]
      REG[ShortcutRegistry\nActions: id, activation, scope]
      DISP[Dispatch\nid → handler]
    end
    subgraph PerProfile["Per GamepadProfile"]
      SM[ShortcutMapping\nid → InputSource list]
    end
    DEV[Device input] --> ENG[ShortcutEngine]
    SM --> ENG
    REG --> ENG
    ENG -->|ShortcutEvent id, phase| DISP
    DISP --> H1[EmulationService / SaveManager / UI]
```

## Data model

```cpp
using ShortcutId = std::string;               // "fast_forward", "save_state" — stable, JSON-friendly

enum class ActivationType { Press, Hold };
enum ShortcutScope { InGame = 1, InMenu = 2, Always = InGame | InMenu };

// GLOBAL, app-defined — catalog entry (metadata + behavior contract)
struct ShortcutAction {
  ShortcutId    id;
  std::string   displayName, category;
  ActivationType activation;
  int           scope;                        // ShortcutScope flags
};                                            // no defaults: presets ship those

// PER-PROFILE user config — lives on GamepadProfile, next to button bindings
//   ShortcutMapping wraps: std::map<ShortcutId, std::vector<InputSource>>

// Emitted by the engine, consumed by dispatch
enum class ShortcutPhase { Started, Ended };
struct ShortcutEvent { int playerIndex; ShortcutId id; ShortcutPhase phase; };
```

`InputSource` (`input_source.hpp`) is reused as the trigger — it already holds a
`code` (a `GamepadInput` for a pad, a `Qt::Key` for the keyboard) plus
`modifiers`, so it works identically across device types and replaces the old
`InputSequence`.

## Multiple bindings per shortcut — two axes

- **Within a profile** — a shortcut maps to a `vector<InputSource>`, so one
  device can have several trigger combos for the same action.
- **Across devices** — the keyboard is its own profile, so binding
  `fast_forward` on the keyboard profile *and* on a controller profile makes
  both fire the one action. Both devices are live simultaneously.

## Activation types (fixes hold)

Activation lives on the **action**, so the engine knows how to interpret an edge:

| Type | Behavior | Emits |
|---|---|---|
| `Press` | fires once on the rising edge | `Started` |
| `Hold` | active while the combo is held | `Started` on press, `Ended` on release |

`Hold` emitting a matching `Ended` is what makes "hold to fast-forward, release
to resume" work — the old system only fired on button-down.

**There is deliberately no `Toggle`.** An action like `pause` or `toggle_mute`
flips state that is **global to the emulator**, but the engine only ever sees
one device at a time, so a latch here is necessarily per-device — and two
devices immediately drift apart (P1 pauses; P2's latch flips `false→true` and
re-asserts a value that is already true, so P2's press does nothing). The engine
reports the edge; whoever owns the state flips it. Those actions are declared
`Press`.

## Scope — gameplay vs. menus

Each action declares a `scope`, and the engine holds a **current context** the
app updates:

- `InGame` — a game is running with no overlay (save state, rewind,
  frame-advance, reset, pause).
- `InMenu` — the library or an open overlay.
- `Always` — screenshot, mute, fullscreen, quit.

`EmulationService` sets `InGame` on load; opening the quick-menu overlay switches
to `InMenu`; the library is `InMenu`. The engine only fires an action when
`action.scope & currentContext`. Every device (keyboard included) always has a
profile, so menu-scope shortcuts still work per device.

## The detection engine

One `ShortcutEngine` (`libs/firelight/input/`) handles every device — pads are
fed from the SDL loop (buttons *and* axes, so a trigger can fire a shortcut) and
the keyboard from its event filter:

```cpp
class ShortcutEngine {
  void setContext(int scope);
  // Fed by the SDL loop (gamepad buttons) AND the keyboard (Qt keys):
  void onInput(int playerIndex, IGamepad* device, int code, bool pressed);
  //  -> tracks per-device held inputs, recomputes each shortcut's "satisfied"
  //     state (its InputSource code + all modifiers held), edge-detects,
  //     applies activation + scope, and publishes ShortcutEvent.
};
```

Detection reads the triggering device's own profile (`device->getProfile()
->getShortcutMapping()`), so pad and keyboard use one implementation.

## Dispatch + catalog

The catalog is declared in `data/shortcuts.json` and loaded into
`ShortcutRegistry` at startup. `ShortcutDispatcher` (`src/app/emulation/`) is the
only subscriber: it hops to the GUI thread and calls `ShortcutActions`, which is
where every action actually lives. Only three reach QML, as signals — the quick
menu, the rewind menu, and fullscreen. The quick menu calls back in through
`ShortcutDispatcher::trigger(id)` rather than repeating any of it.

Starter catalog (ids · activation · scope):

| id | activation | scope |
|---|---|---|
| `fast_forward` | Hold | InGame |
| `toggle_fast_forward` | Press | InGame |
| `slow_motion` | Hold | InGame |
| `speed_up` / `slow_down` | Press | InGame |
| `save_state` / `load_state` | Press | InGame |
| `state_slot_next` / `state_slot_prev` | Press | InGame |
| `open_rewind_menu` / `open_quick_menu` | Press | InGame |
| `frame_advance` / `reset` / `pause` | Press | InGame |
| `screenshot` / `toggle_mute` / `toggle_fullscreen` | Press | Always |
| `exit_game` | Press | InGame |

## Defaults: presets seed a profile once

Triggers are per-profile, so something has to fill a new profile in or it ships
unbound — which is exactly what used to happen. `data/shortcuts.json` declares
**presets** (`firelight`, `retroarch-keyboard`, `handheld`, `none`), and
`createProfile` copies one in at creation. A preset is a starting point, not a
tier: the engine never resolves through it, so an unbound action just means off,
and "reset this row" is a write rather than an erase.

Presets are keyed on `DeviceType` (Gamepad/Keyboard) only — a keyboard binding
stores a `Qt::Key` and a pad one a `GamepadInput`, which is the one difference
that forces a split. There is deliberately no per-`GamepadType` table:
`GamepadInput` already normalizes across pads, and a preset lists **alternates**
per action, so a pad without R3 simply never satisfies that source and falls
through to the next.

## Conflict with game input

**The rule: when a shortcut fires, the input that triggered it is withheld from
the game until it is physically released. Nothing else.** One rule, always on,
no per-action flag.

It works because a **combo disambiguates at the trigger's rising edge**, so the
decision is conditional with no lookahead. Bind `Select+X`: press X alone and
nothing is satisfied, so the game gets X; hold Select and press X, and the
shortcut fires and X is swallowed.

- **Modifiers leak, deliberately.** The modifier was already down when the combo
  completed, so masking it would hand the core a release it never got, and
  unmasking would hand it a second press. The trigger has no such problem — its
  rising edge *is* the shortcut's, so the core never saw it down. This is why
  the modifier belongs on a button the game rarely reads (Select/Back).
- **A modifier-less binding is the one lossy case**, and honestly so: bind
  `pause` to bare Start and Start stops reaching the game while in-game, because
  there is genuinely no way to tell the two intents apart. That is a reason for
  the editor to warn, and for every shipped pad preset to use combos — not for a
  config flag.

`InputSuppressor` (on `IGamepad`) holds the masked codes; the engine writes them
inside `onInput`, under its lock and *before* publishing, since a subscriber runs
a queued hop later — by which time the core would have sampled the button.
Masks clear on physical release, on `forgetDevice`, and on `setContext`.

## What changes vs. today

| | Today | New |
|---|---|---|
| Catalog | 5-value `Shortcut` enum in the input lib | global `ShortcutRegistry`, string ids, extensible |
| Triggers | per-profile `ShortcutMapping` (`InputSequence`) | per-profile (kept) — `map<id, vector<InputSource>>` |
| Multi-binding | one sequence per shortcut | vector per profile + across device profiles |
| Activation | press-only (hold broken) | Press / Hold |
| Scope | none | InGame / InMenu / Always |
| pad vs keyboard | two detection paths | one `ShortcutEngine` |
| dispatch | QML switch on enum int | `ShortcutId → handler` registry |
