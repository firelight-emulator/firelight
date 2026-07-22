# Making a screen

Short version: **write an `FLPage`, register one route, done.** Everything below the window title
bar is a screen; the reusable parts are `FL*` components. Press **F11** in the running app to see
every component that exists.

## 1. Write the page

Screens use `FLPage` (`qml/components/v2/surfaces/FLPage.qml`) — a title + optional header actions
over a body that fills the rest. You bring the body; `FLPage` owns the chrome.

```qml
// qml/components/v2/pages/ThingPage.qml
import QtQuick
import QtQuick.Layouts

FLPage {
    title: "Thing"
    headerActions: [
        FLButton { text: "New"; variant: "primary"; onClicked: doThing() }
    ]

    // The body fills the area. Bring your own scroll / list / grid.
    FLScrollView {
        anchors.fill: parent
        ColumnLayout {
            width: parent.width
            spacing: AppStyle.spacingMd
            // ... FL* content ...
        }
    }
}
```

Conventions that keep screens consistent:
- **Color ← `Theme`, size/metrics ← `AppStyle`.** Never a literal `#hex`, `14`, or `8` at a call
  site — a raw value doesn't theme or scale.
- **Reuse `FL*`** — don't hand-roll a `Button`/`TextField`/row/panel. Add a new `FL*` rather than a
  one-off.
- Icons: `Icon { name: "settings"; size: AppStyle.iconSizeMd; color: Theme.textPrimary }`.
- No manual focus rings — the global `FLFocusHighlight` draws focus. Never key a border on
  `activeFocus`.

## 2. Register the route

1. `qml/routing.js` — add `{ pattern: "/thing" }` to `ROUTES`.
2. `qml/components/v2/RouteView.qml` — add `"/thing": thingComponent` to the `componentFor` map, plus
   a `Component { id: thingComponent; ThingPage {} }`.

A new `.qml` under `qml/` needs a `cmake --preset debug-win` reconfigure to register (the file list
is a GLOB).

## 3. Verify

```bash
QT_QPA_PLATFORM=offscreen ./build/debug-win/firelight.exe verify-ui   # mounts every route
```
Add the pattern to `getDefaultRoutes()` in `src/cli/verify_ui_command.cpp` so the sweep covers your
screen and it can't silently break later.

## The component menu

Press **F11** anywhere for the live gallery (`qml/components/v2/dev/ComponentGallery.qml`) — every
`FL*` with its variants and states. Copy from there instead of hunting. Built a new `FL*`? Add its
variants to that file so it stays discoverable.
