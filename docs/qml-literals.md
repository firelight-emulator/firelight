# Literal values left in the live QML

Written for the styling pass. Spacing has been tokenized; sizes and colours have **not**, and this
explains why — in both cases converting mechanically would have manufactured false consistency.

Scope is `qml/components/v2/**` + `qml/components/settings/**`.

## Done — spacing (82 sites, 25 files)

`spacing`, `padding`, `*Margin`, `anchors.margins` whose value was exactly 4 / 8 / 12 / 16 now use
`AppStyle.spacing{Xs,Sm,Md,Lg}`. Line count and binding count were unchanged by the edit.

**91 spacing literals remain**, on values with no token: mostly 6, 10, 20, plus `0`/`1`/`2` which are
deliberate (zero gaps, hairlines). If 6 and 20 are real steps in your scale, they want tokens; if not,
they want rounding to an existing step.

## Not done — sizes, and why

Only **24 of ~103** `width`/`height` literals coincide with a token value, and the coincidence is not
meaningful — a scrollbar's `width: 8` is not `spacingSm`, and a spacer's `width: 32` is not
`iconSizeLg`. The values actually in use are mostly **off the scale entirely**:

| value | count | token |
|---|---|---|
| 32 | 9 | `iconSizeLg` |
| 40 | 6 | — |
| 34 | 6 | — |
| 24 | 6 | `iconSizeMd` / `minTarget` |
| 16 | 6 | `iconSizeSm` |
| 48 | 5 | — |
| 72 | 3 | — |
| 36 | 3 | `controlHeight` |

The clearest evidence: every `Icon { size: … }` literal in the live tree is **14, 15 or 22** — not one
matches `iconSizeSm/Md/Lg` (16/24/32). The icon sizes were never designed against the scale, so
converting the few coincidental matches would imply a consistency that isn't there.

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
