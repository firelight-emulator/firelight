# Design system

Two token singletons, one rule: **colour comes from `Theme`, size/metrics from `AppStyle`, never a
literal at a call site.** Reusable widgets are the `FL*` components in `qml/components/v2/`; new screens
use the `FLPage` scaffold. This document is the token reference and the migration status; the working
conventions live in `CLAUDE.md` and `docs/making-a-screen.md`.

## Token tiers

Colour flows through three tiers; a call site only ever touches the semantic (or component) layer.

| Tier | File | Holds |
|---|---|---|
| Primitive | `qml/ColorPalette.qml` | raw values, no meaning — the **only** place a colour hex should live |
| Semantic | `qml/Theme.qml` | roles named by job (`surface`, `accent`, `focusRing`, `danger`), reactive to appearance settings |
| Component | `qml/AppStyle.qml` + the `FL*` components | control metrics that point at the semantic layer |

`AppStyle` is the metrics half (sizes only). Both singletons react to user settings via
`AppearanceSettings` (accent, background, intensity, glass opacity, `uiScale`, `uiDensity`).

## Colour — `ColorPalette` → `Theme`

**Primitives (`ColorPalette`)**
- Radix Slate ramp is private to `Theme` (`_slate1`..`_slate12`).
- `ember300/400/500/600` — the single accent ramp. `accentColor` = `ember500`.
- `statusGreen / statusYellow / statusRed` — status solids (Radix step 9).
- `pink` — the fixed favourite/heart colour.
- Legacy numbered scales (`primary*`, `accent*`, `neutral*`, `red*`) remain until their consumers migrate.

**Semantic (`Theme`)**
- Surface stack: `background · backgroundSubtle · surface · surfaceElevated · surfaceHover · border · borderStrong`
  (Slate tinted toward the theme colour by `intensity`).
- Text: `textPrimary · textMuted`.
- `accent` (user-set) · `onAccent` · `focusRing` (**fixed** `ember400`, never the user accent).
- `success · warning · danger` → the status primitives. `favorite` → `pink`.
- `glass · glassElevated · glassBorder` — translucent, for expressive surfaces over the background.
- `shadow` — drop-shadow colour (currently aliases `background`).
- `dark` — theme-identity flag; dark-first today, the hook for a future light mode.
- Chrome `buttonBg* / titleBar*` still read the legacy `ColorPalette.neutral*` — reconcile when the
  buttons move to `FLButton`.

**Glass vs solid:** glass where the backdrop should show through (home, library, chrome); solid where
content must stay readable over anything (settings, dialogs, in-game menus).

## Metrics — `AppStyle`

- Type scale (px, `× scale`): `fontSizeSmall/Medium/Large/XLarge` = 14/15/22/32. Never `font.pointSize`.
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
- One UI typeface: the **Lexend** variable font — every `Constants.*FontFamily` resolves to it, and weight
  is chosen per call site via `font.weight`. Icons use Material Symbols (`symbolFontFamily`). A stray
  `font.family: "Consolas"` (netplay chat) still awaits a mono token.

## Migration status

**Done (value-preserving unless noted)**
- Ember ramp: three orange literals collapsed to one primitive scale; no orange literal outside `ColorPalette`.
- `focusRing` token; `FLFocusHighlight` reads it instead of a hardcoded hex.
- `dark` theme-identity flag.
- Motion tokens defined and wired at the exact-match call sites (200/120 durations, `Easing.OutCubic`).
- Elevation reshaped to the `MultiEffect` model and wired at the four identical chrome shadows.
- Status colours tokenised through `ColorPalette`.
- Favourite/heart colour and the folder swatch palette de-duplicated onto shared sources.
- Removed the unused Material-3 `color_*` palette and `purple*` tokens from `Constants`.
- Typeface consolidated onto the Lexend variable font: killed the `Segoe UI` hardcode (250 sites), fixed
  the qrc aliases that pointed at Inter/NunitoSans, and deleted the 23 unused font files.

**Pending**
- z-layer tokens are defined but unwired — the live `z: 100` sites are window resize edges, not dropdowns,
  so wiring wants a per-site semantic decision.
- `Theme.shadow` aliases `background` to match current use; decoupling to a real translucent shadow is a
  visual decision.
- `FLSuspendPointCard` keeps its own card shadow (a second elevation level would fold it in).
- `LibrarySidebar`'s folder-colour menu is a named menu; it still lists the swatch colours inline.
- `Constants.colorTest*` retained — one consumer remains (`AchievementList.qml`).
- Legacy `ColorPalette` chrome inside `Theme.buttonBg*`, and the remaining raw colour/size literals, are a
  per-site decision (see `docs/qml-literals.md`), not a mechanical sweep.
- A monospace token is still needed for the one hardcoded `font.family: "Consolas"` (netplay chat).
