# v3 component inventory — first pass

The primitives, sections, and behavior contracts for the v3 first pass: the library screen
(grid/list, sort/filter/search, context menus, game details), plus the shared surfaces
(dialogs, quick menu, settings, notifications). Decisions below were made deliberately;
change them here first if they change.

## Decisions

| Question | Decision |
|---|---|
| Sort + filter popups | Undecided by design — sections are components, popups are thin shells, so separate or combined is a late composition choice |
| Radio-list pick behavior | Close on pick, then grid transition (Switch style); checklists stay open |
| Ascending/descending | Re-picking the active sort flips direction; chevron on the active row shows it |
| Filter apply timing | Accumulate in a draft, apply once on dismiss with one transition |
| Search | Live filter in place — the grid is the results; no transition while typing |
| Details page | Routed page at `/library/entries/:id` (deep-linkable, fits descend/ascend transitions, hero zoom later) |
| Context menu | One menu, both inputs: right-click at pointer, gamepad button anchored to focused tile |
| Multi-select / bulk | Deferred; action signatures take `targetIds: [id]` arrays now so bulk adds later without churn |
| Dialogs | Switch-style centered modal: message + side-by-side buttons, B cancels, destructive styled with Theme.danger — one FLDialog for all |
| In-game quick menu | In scope: full-screen pause overlay (dimmed gameplay behind, centered menu, save-state thumbnail cards) |
| Settings | Stays the RouteOverlay modal (left section nav + content) |
| Notifications | One FLToast system — variants (info/success/achievement/error), corner stack, auto-dismiss |
| Sections this pass | Library only; gallery/activity/netplay/shop/help migrate in later passes |
| Main navigation | Deliberately undecided (rail vs top tabs vs home hub) — moot while only the library exists; the transition grammar's section-order table waits on it |
| Empty states | One FLEmptyState used for: fresh install (no directories, with action to settings), filters match nothing (Clear filters action), no search hits, scan in progress |
| Save states / suspend points | Thumbnail cards (FLSaveSlotCard: screenshot + slot + timestamp), shared by quick menu and details page |

## A. Surface primitives (shells)

### FLPanelPopup (new)
Base for every "popup with stuff in it".
- FLPopup styling, standard width/padding tokens, `focus: true`, Esc/B + click-away dismissal,
  default ColumnLayout content slot
- Anchors to its invoking button; focus (and ring, via blink) returns there on close
- Controller traversal up/down across its child controls (grow the FLColumnLayout focus-walker
  into this — every future panel inherits it)
- Two behavior flags cover the taxonomy: `closeOnActivate` (radio popups) and the draft/apply
  contract — `applied(changes)` emitted on dismissal when content changed (filter popups).
  Sections never know which mode the shell is in

### FLActionMenu (new)
The context menu. Menu contract: activate → perform → close.
- Custom implementation (QQC2 Menu fights the header + controller styling); reuses panel plumbing
- Optional header slot (game art + title), action rows, separators
- Two entry points, same instance: `popup(point)` at the pointer, `popupAt(item)` anchored to the
  focused tile for controller invocation
- Full controller nav: up/down rows, A activates, B closes; rows opt into the global focus ring
- Actions receive `targetIds` arrays (bulk-ready)

### FLDialog (new)
Confirmation / decision modal, one primitive for every yes-no and destructive prompt
(delete save, overwrite slot, quit game).
- Centered over a dim, message + optional detail, side-by-side buttons (Cancel | Confirm)
- Controller: left/right between buttons, A activates, B always cancels
- `destructive: true` styles the confirm button with Theme.danger
- Returns focus (with cursor blink) to the invoker on close

### FLToast (new)
The one notification surface. Corner stack, newest on top, auto-dismiss with duration tokens.
- Variants: info, success, error, achievement (icon + text + optional art)
- Shortcut feedback and achievement unlock popups migrate onto it
- Non-interactive in this pass (no actions in toasts yet)

## B. Section components (popup contents, freely composable)

- **FLRadioList (new)** — pick-one list. Model `{label, value, icon?}` + `currentValue`, emits
  `activated(value)`. Re-activating the current value is the direction-flip signal — the state
  owner interprets it; the list stays dumb. Active row shows an asc/desc chevron via `direction`
- **FLCheckList (new)** — multi-select list editing a working copy; the shell diffs on dismissal.
  Later, without contract changes: per-section search, "clear all" row
- **FLSectionLabel (new)** — small-caps group header, promoted out of GameDisplayTypePopup
- **FLSegmentedControl (port)** — from v2 unchanged

Compositions: sort popup = `FLPanelPopup { FLRadioList }` + `closeOnActivate`; filter popup =
`FLPanelPopup { FLSectionLabel + FLCheckList, ... }` + apply-on-dismiss; a combined panel is one
shell holding both — the mixed contract resolves per section.

## C. Motion & feel primitives

- **FLViewTransition (new)** — content swap: `swapOut` (fade + slide down) → `midpointReached`
  (change data while hidden; async-friendly — call `swapIn` when the model is ready) → `swapIn`
  (fade + slide up). Used by sort pick, filter apply, view-mode toggle, scope changes. NOT used
  by live search typing
- **FocusCursor (new singleton)** — cursor command relay. `blink(ms)`: ring vanishes, then fades
  in wherever focus is at expiry (leverages the highlight's existing appearing path). Called on
  popup close and page transitions. Future: SFX hooks, bump-from-anywhere
- **Focus highlight / bump / scroll (done)** — measured Switch-matched motion, see
  memory/project notes
- **Transition grammar (designed)** — relationship-keyed page transitions
  (descend/ascend/lateral/overlay/teleport + sparse overrides); the details page is its first
  client

## D. Pages & chrome

- **FLTabView (new)** — tab strip + lazy Loader per tab, underline indicator, shoulder-button
  (LB/RB) paging alongside click. Details page first; settings later
- **Toolbar rail (exists)** — one new convention: each button owns its popup, opens it anchored
- **Search field (new, small)** — toolbar area, binds live to the model filter string; Esc
  clears; the search button focuses it
- **FLEmptyState (new)** — icon + message + optional action button. Library-pass uses: fresh
  install ("add a folder" → directory settings), filters match nothing ("Clear filters"),
  no search hits, scanning in progress
- **FLSaveSlotCard (new)** — suspend-point card: screenshot thumbnail + slot number +
  timestamp + action affordance. Shared by the quick menu and the details page save tab
- **Details page** — routed page composing FLTabView + FLSaveSlotCard + FLListRow +
  FLActionMenu (per-slot actions: load, rename, icon). No hidden primitives if the above exist
- **Quick menu (in scope)** — full-screen pause overlay: dimmed gameplay behind, centered menu
  (resume / save states as FLSaveSlotCard row / screenshot / settings / quit), built entirely
  on v3 primitives with ring navigation; FLDialog for quit confirmation
- **Settings** — stays the RouteOverlay modal; its pages adopt v3 tokens/components
  incrementally, no structural change this pass

## Behavior contracts end to end

- **Sort pick**: A/click on row → popup closes → `FocusCursor.blink` → `swapOut` → apply sort at
  hidden midpoint → `swapIn`. Re-pick active row → direction flips, same flow
- **Filter session**: open panel → toggle freely (ring navigates, popup stays) → dismiss → if
  draft differs: blink + one transition + batch apply
- **Context menu**: right-click / gamepad button on focused tile → header + actions → A performs
  and closes → focus + ring return to the tile
- **Search**: focus field → grid filters live → Esc clears, focus returns to the grid

## Build order

1. FLPanelPopup + FLSectionLabel + FLRadioList → working sort popup (no transition yet)
2. FLViewTransition + FocusCursor.blink → wire the sort flow end to end (validates the feel)
3. FLCheckList + draft/apply → filter popup
4. FLActionMenu (mouse first, controller entry second)
5. FLEmptyState + search field → the library screen is complete
6. FLTabView + FLSaveSlotCard + FLDialog → details page composition
7. Quick menu overlay (reuses 6's pieces) + FLToast migration

Open decisions parked on purpose: sort+filter combined vs separate (compose when the popups
exist); main navigation form (decide when a second section migrates — it sets the lateral
transition direction table)
