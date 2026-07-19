# Literal values left in the live QML

Written for the styling pass. Spacing and icon sizes are tokenized; the remaining sizes and all
colours are **not**, and this explains why — converting those mechanically would manufacture a
consistency the UI does not actually have.

Scope is `qml/components/v2/**` + `qml/components/settings/**`.

## Done — spacing

`spacing`, `padding`, `*Margin`, `anchors.margins` now use `AppStyle.spacing{Xs,Sm,Md,Lg,Xl}`. Values
that matched a token exactly went across first; the leftovers (6, 10, 11, 13, 14, 18, 20) were rounded
to the nearest step, rounding up on ties. Line count and binding count were unchanged by every edit.

What remains is deliberate:

| value | count | why it stayed |
|---|---|---|
| 0 | 50 | zero gap |
| 1 | 1 | hairline |
| 2 | 15 | a 2px nudge is not a spacing step — `spacingXs` is 4, so tokenizing would double it |
| 3 | 2 | same |
| 64 | 1 | no token within range |

## Done — icon sizes only

Converted where the element genuinely *is* an icon and the value matched a token exactly: the two
16×16 chevrons and the 32×32 list icon in `ActivityPageV2`, and the 24×24 / 32×32 icon buttons in
`GameListView` (`implicitWidth`/`implicitHeight` plus their paired `icon.width`/`icon.height` and
`sourceSize`).

## Not done — the other sizes, and why

The rest of the `width`/`height` literals are not token-shaped. Reading them by what they actually
are, rather than by value:

| what it is | values | why no token fits |
|---|---|---|
| dialog + panel widths | 800, 520, 500, 420, 399, 280, 240 | layout dimensions; the scale has nothing this large |
| art + thumbnails | 240, 84, 72, 50, 40 | sized to the artwork, not to a control |
| text column widths | 180, 140, 110, 104, 60, 30, 23 | measured to fit a string |
| status dots, radio rings, drag handles | 18, 10, 8, 6, 4, 3 | sub-token ornament |
| scrollbar widths | 8 | a scrollbar's `width: 8` is not `spacingSm` |
| square badges + buttons | 36, 34, 32 | 34 and 36 are near `controlHeight` (36) but these are badges, not controls |

The clearest evidence that the scale and the UI were never designed together: every `Icon { size: … }`
literal in the live tree was **14, 15 or 22** — not one matched `iconSizeSm/Md/Lg` (16/24/32). Those
four sites were rounded to the nearest token, but the mismatch is the real signal.

**This is a design decision, not a cleanup**: either the scale grows to fit what the UI actually uses,
or the UI moves onto the scale. Worth settling early in your pass, because everything else follows.

## Not done — colours, and why

48 literals, in three groups that want three different treatments:

**1. Themeable (18 sites)** — `"white"`, `"#FFFFFF"`, `"black"`, `"grey"`. Only **3** are on `Text`
elements; the rest are on scrims, borders and indicators where "white" may well be intentional rather
than a stand-in for `Theme.textPrimary` (which is `_slate12`, a near-white, not pure white). Converting
changes what renders, so it should be a decision, not a sweep.

**2. A user-facing colour palette — and it is duplicated.** These are not theme colours, they are
*data*: swatches the user picks from. The same set appears in two unrelated places:

- `qml/components/settings/ColorSettingItem.qml:25` —
  `["#e5484d", "#f76b15", "#f5d90a", "#46a758", "#0091ff", "#8e4ec6", "#e93d82", "#ffffff"]`
- `qml/components/v2/library/LibraryPageV2.qml:528-552` — the folder-colour menu, the same hexes
  written out one `onTriggered` at a time

Making these `Theme` tokens would be wrong — they must stay stable regardless of theme. They want to
be **one shared palette** that both read. Separately, `#e55aa2` (the favourite heart) is hardcoded in
both `GameGridView.qml:195` and `GameListView.qml:399`.

**3. Scrims and overlays (7 sites)** — 8-digit hexes carrying alpha: `#00000040`, `#66000000`,
`#99000000`, `#aa000000` ×2, `#dd000000`, `#cc9a6b12`. There is no `Theme` equivalent for a scrim.
If overlays matter to the look, the tokens don't exist yet.

## Also worth knowing

`ActivityPageV2.qml` has debug text rendering in a live page — `color: "lime"` on a
`"playtime=… | buckets=…"` string.
