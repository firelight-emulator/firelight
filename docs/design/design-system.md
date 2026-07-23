# Design system

Two token singletons, one rule: **colour comes from `Theme`, size/metrics from `AppStyle`, never a
literal at a call site.** Reusable widgets are the `FL*` components in `qml/components/v2/`; new screens
use the `FLPage` scaffold. This document is the token reference and the migration status; the working
conventions live in `CLAUDE.md` and `docs/making-a-screen.md`.

## Token tiers

Colour and metrics live in two singletons; a call site only ever touches those, never a raw value.

| Layer | File | Holds |
|---|---|---|
| Colour | `qml/Theme.qml` | raw primitives kept **private** (`_slate1`..`_slate12`, `_ember400`, `_status*`, `_pink`, `_link`) plus the semantic roles built from them (`surface`, `accent`, `focusRing`, `danger`, …), reactive to appearance settings |
| Metrics | `qml/AppStyle.qml` + the `FL*` components | sizes only — fonts, spacing, radii, heights, motion, elevation |

Keeping the primitives **private inside `Theme`** is deliberate: a call site can't reach a raw hex, so it
can't bypass the semantic layer — which is exactly how the old public `ColorPalette` let one orange
scatter across three files. Both singletons react to user settings via `AppearanceSettings` (accent,
background, intensity, glass opacity, `uiScale`, `uiDensity`).

## Colour — `Theme`

**Primitives (private to `Theme`)**
- Radix Slate ramp `_slate1`..`_slate12` — the neutral backbone (see `docs/design/…` and the in-app
  swatch reference for the elevation model).
- `_ember400` — the fixed focus step. `_statusGreen/_statusYellow/_statusRed` — status solids (Radix
  step 9). `_pink` — the favourite/heart colour. `_link` — hyperlink blue.
- Every raw colour hex lives here; nothing outside `Theme` should name one (bar a handful of call-site
  one-offs still pending — see `docs/qml-literals.md`).

**Semantic (`Theme`)**
- Surface stack: `background · backgroundSubtle · surface · surfaceElevated · surfaceHover · border · borderStrong`
  (Slate tinted toward the theme colour by `intensity`).
- Text: `textPrimary · textMuted`.
- `accent` (user-set) · `onAccent` · `focusRing` (**fixed** `_ember400`, never the user accent).
- `success · warning · danger` → the status primitives. `favorite` → `_pink`. `link` → `_link`.
- `folderColors` — the pickable folder/label swatch palette (data, not a role).
- `glass · glassElevated · glassBorder` — translucent, for expressive surfaces over the background.
- `shadow` — drop-shadow colour (currently aliases `background`).
- `dark` — theme-identity flag; dark-first today, the hook for a future light mode.
- Chrome `buttonBgOpacity* / buttonText* / titleBar*` — button/title-bar state values (the dead
  `buttonBg*` colour tokens are gone; `buttonTextDisabled` reads `textMuted`).

**Glass vs solid:** glass where the backdrop should show through (home, library, chrome); solid where
content must stay readable over anything (settings, dialogs, in-game menus).

## Metrics — `AppStyle`

- Type scale (px, `× scale`): `fontSizeSmall/Medium/Large/XLarge` = 14/15/22/32. Never `font.pointSize`.
- Typeface: `fontFamily` (Lexend) · `symbolFontFamily` (Material Symbols).
- Spacing (`× scale × density`): `spacingXs..Xl` = 4/8/12/16/24. Radii: `radiusSm/Md/Lg` = 4/8/12.
- Heights: `controlHeight` 36, `rowHeight` 44, floored at `minTarget` 24 (WCAG 2.5.8).
- Icons: `iconSizeSm/Md/Lg` = 16/24/32.
- Motion: `durationFast/Base/Slow` = 120/200/320, `easingStandard`.
- Elevation: `elevationBlur` (normalised 0..1) + `elevationOffset` (px), for `MultiEffect` drop shadows.
- Layers: `zBase/zDropdown/zOverlay/zToast/zModal` (defined; see migration).

## Conventions

- Colour ← `Theme`, size ← `AppStyle`; add an `FL*` rather than a one-off.
- **Assistant-added comments carry a bare `// TODO` marker** on the line above, for the author to reword
  or drop (`grep -rn '// TODO$'`).
- One UI typeface: the **Lexend** variable font — `AppStyle.fontFamily` resolves to it, and weight
  is chosen per call site via `font.weight`. Icons use Material Symbols (`AppStyle.symbolFontFamily`). A stray
  `font.family: "Consolas"` (netplay chat) still awaits a mono token.

## Migration status

**Done (value-preserving unless noted)**
- Ember ramp: three orange literals collapsed to one accent step; no orange literal outside `Theme`.
- `focusRing` token; `FLFocusHighlight` reads it instead of a hardcoded hex.
- `dark` theme-identity flag.
- Motion tokens defined and wired at the exact-match call sites (200/120 durations, `Easing.OutCubic`).
- Elevation reshaped to the `MultiEffect` model and wired at the four identical chrome shadows.
- Status colours tokenised through `Theme`.
- Favourite/heart colour and the folder swatch palette de-duplicated onto shared sources.
- Typeface consolidated onto the Lexend variable font: killed the `Segoe UI` hardcode (250 sites), fixed
  the qrc aliases that pointed at Inter/NunitoSans, and deleted the 23 unused font files.
- `Constants.qml` fully dismantled and deleted: font loaders + family tokens and the quick-menu banner
  height moved to `AppStyle`, the folder swatch palette and the last `colorTest`/right-click-menu
  consumers repointed at `Theme`; CMake singleton registration removed.
- **`ColorPalette.qml` deleted**: its live primitives (`ember400`, `status*`, `pink`) folded into `Theme`
  as private `_`-prefixed vars; the whole legacy `neutral*`/`red*`/`hyperlink` scale migrated onto
  semantic `Theme` tokens across 18 files (66 refs); dead `buttonBg*` colour tokens dropped; `link` and
  `folderColors` added to `Theme`. The system is now two-tier — primitives are private, not a public file.

**Pending**
- z-layer tokens are defined but unwired — the live `z: 100` sites are window resize edges, not dropdowns,
  so wiring wants a per-site semantic decision.
- `Theme.shadow` aliases `background` to match current use; decoupling to a real translucent shadow is a
  visual decision.
- `FLSuspendPointCard` keeps its own card shadow (a second elevation level would fold it in).
- `LibrarySidebar`'s folder-colour menu is a named menu; it still lists the swatch colours inline.
- The remaining raw colour/size literals at call sites are a per-site decision (see `docs/qml-literals.md`),
  not a mechanical sweep.
- A monospace token is still needed for the one hardcoded `font.family: "Consolas"` (netplay chat).
