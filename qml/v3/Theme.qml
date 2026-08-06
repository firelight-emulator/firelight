pragma Singleton

import QtQuick

// Semantic color tokens for the app. One theme color drives two derivations,
// Discord-style:
//   - SOLID tokens (surface/border/text): the Radix Slate (dark) neutral scale
//     tinted toward the theme color. Used for functional surfaces that must stay
//     readable over any background (game art, gradients)
//   - GLASS tokens: translucent dark that lets the blurred background bleed
//     through. Used for expressive surfaces (library, chrome)
// Reads the persisted knobs from AppearanceSettings so the whole app re-themes
// when the user changes them
QtObject {
    id: theme

    // TODO
    // Theme identity. Dark-first today; a light mode branches the derivations
    // below on this flag rather than forking the whole token set
    readonly property bool dark: true

    // Radix Slate — dark scale. Step roles (Radix convention):
    //   1-2 backgrounds · 3-5 component surfaces (normal/hover/active) ·
    //   6-8 borders (subtle/normal/focus) · 11 muted text · 12 primary text
    readonly property color _slate1: "#111113"
    readonly property color _slate2: "#18191b"
    readonly property color _slate3: "#212225"
    readonly property color _slate4: "#272a2d"
    readonly property color _slate5: "#2e3135"
    readonly property color _slate6: "#363a3f"
    readonly property color _slate7: "#43484e"
    readonly property color _slate8: "#5a6169"
    readonly property color _slate11: "#b0b4ba"
    readonly property color _slate12: "#edeef0"

    // TODO
    // Fixed brand primitives (not the user accent) — consumed only through the
    // semantic tokens below, never referenced at a call site
    readonly property color _ember400: "#ff9f38"
    readonly property color _statusGreen: "#30a46c"
    readonly property color _statusYellow: "#ffc53d"
    readonly property color _statusRed: "#e5484d"
    readonly property color _pink: "#e55aa2"
    readonly property color _link: "#89c5f9"
    readonly property color _gold: "#e3b341"
    readonly property color _focusBlue: "#0073e6"
    readonly property color _focusPurple: "#5e2fbc"
    readonly property color _focusPale: "#5ac4f2"
    readonly property color _focusPeriwinkle: "#8379ec"

    // Persisted knobs
    readonly property real intensity: AppearanceSettings.themeIntensity
    readonly property real glassAlpha: AppearanceSettings.glassOpacity

    // Tint source. In "image" mode the app tints itself from the chosen image
    // (its top/bottom bands, sampled into AppearanceSettings); otherwise from
    // the solid/gradient colors. The translucent glass surfaces render the real
    // top->bottom gradient (they show the image through them); themeColor is the
    // blend of the two, used by the flat solid tokens
    readonly property bool _imageTint: AppearanceSettings.backgroundMode === "image" && AppearanceSettings.imageColorTop !== ""
    readonly property color tintTop: _imageTint ? AppearanceSettings.imageColorTop : AppearanceSettings.backgroundColor
    readonly property color tintBottom: _imageTint ? AppearanceSettings.imageColorBottom : (AppearanceSettings.backgroundMode === "gradient" ? AppearanceSettings.backgroundColor2 : AppearanceSettings.backgroundColor)
    readonly property color themeColor: mix(tintTop, tintBottom, 0.5)

    // Composite `tint` over `base` at `amount` (0..1), returning a solid color
    function blend(base, tint, amount) {
        return Qt.tint(base, Qt.rgba(tint.r, tint.g, tint.b, amount));
    }

    // Linear blend between two colors (t in 0..1)
    function mix(a, b, t) {
        return Qt.rgba(a.r + (b.r - a.r) * t, a.g + (b.g - a.g) * t, a.b + (b.b - a.b) * t, 1);
    }

    // A readable text/icon color to sit on top of `fill`
    function onColor(fill) {
        return (0.299 * fill.r + 0.587 * fill.g + 0.114 * fill.b) > 0.55 ? "#111113" : "#ffffff";
    }

    /***********************************************************
    * Main color tokens
    ***********************************************************/

    // Use for main backgrounds - the window, popups, etc
    readonly property color background: blend(_slate1, themeColor, intensity)

    // Use for inset items like text inputs, button group backgrounds, etc
    readonly property color backgroundInset: blend(_slate2, themeColor, intensity)

    // Use for functional surfaces like cards, panels, and menus that sit directly on the background
    readonly property color surface: blend(_slate3, themeColor, intensity)

    // Use for items that sit on top of surfaces
    readonly property color surfaceElevated: blend(_slate4, themeColor, intensity)

    // Use for hover/active states of elevated surfaces
    readonly property color surfaceHover: blend(_slate5, themeColor, intensity)

    readonly property color border: blend(_slate6, themeColor, intensity)
    readonly property color borderStrong: blend(_slate8, themeColor, intensity)

    readonly property color textPrimary: _slate12
    readonly property color textMuted: _slate11

    // Radix accent + status (dark, step 9)
    readonly property color accent: AppearanceSettings.accentColor
    readonly property color onAccent: onColor(accent)

    // TODO
    // Focus ring is a fixed ember step, never the user's accent — so the
    // controller-focus highlight stays high-contrast whatever the accent is set to
    readonly property color focusRing: _ember400

    // TODO
    // The two colors that rotate around the controller-focus ring (Switch-style)
    readonly property color focusRingBlue: _focusBlue
    readonly property color focusRingPurple: _focusPurple
    readonly property color focusRingPale: _focusPale
    readonly property color focusRingPeriwinkle: _focusPeriwinkle
    readonly property color focusRingFill: "transparent"

    readonly property color success: _statusGreen
    readonly property color warning: _statusYellow
    readonly property color danger: _statusRed

    // TODO
    // Achievement mastery / 100%-complete gold — its own token, not warning
    readonly property color gold: _gold

    // TODO
    // Link / hyperlink text
    readonly property color link: _link

    // TODO
    // Favourite (heart) colour — a fixed brand pink, independent of the accent
    readonly property color favorite: _pink

    // TODO
    // Shadow colour — currently the app ground, matching the existing MultiEffect
    // sites; decouple to a translucent dark in a later pass
    readonly property color shadow: background

    // --- chrome: button / title-bar state colors ---
    readonly property real buttonBgOpacityDisabled: 0
    readonly property real buttonBgOpacityInactive: 0.15
    readonly property real buttonBgOpacityHovered: 0.3
    readonly property real buttonBgOpacityPressed: 0.2
    // TODO
    // Faint fill behind a button the controller cursor is on, under the ring
    readonly property real buttonBgOpacityFocused: 0.1
    readonly property color buttonTextDisabled: textMuted
    readonly property color buttonTextInactive: "white"
    readonly property color buttonTextFocused: "black"

    // Title-bar min/max share one state look; close is red
    readonly property color titleBarBtnHovered: "white"
    readonly property real titleBarBtnHoveredOpacity: 0.08
    readonly property color titleBarBtnPressed: "white"
    readonly property real titleBarBtnPressedOpacity: 0.05
    readonly property color titleBarCloseHovered: "#c42b1c"
    readonly property real titleBarCloseHoveredOpacity: 1
    readonly property color titleBarClosePressed: "#941320"
    readonly property real titleBarClosePressedOpacity: 1

    // glass tokens (translucent over the blurred background)
    readonly property color glass: Qt.rgba(_slate1.r, _slate1.g, _slate1.b, glassAlpha)
    readonly property color glassElevated: Qt.rgba(_slate1.r, _slate1.g, _slate1.b, Math.min(1, glassAlpha + 0.12))
    readonly property color glassBorder: Qt.rgba(1, 1, 1, 0.08)

    // TODO
    // Pickable folder / label swatch colours (data values, not theme roles)
    readonly property var folderColors: ["#e5484d", "#f76b15", "#f5d90a", "#46a758", "#0091ff", "#8e4ec6", "#e93d82"]
}
