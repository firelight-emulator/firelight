pragma Singleton

import QtQuick

// Semantic color tokens for the app (a clean layer alongside the legacy
// ColorPalette). One theme color drives two derivations, Discord-style:
//   - SOLID tokens (surface/border/text): the Radix Slate (dark) neutral scale
//     tinted toward the theme color. Used for functional surfaces that must stay
//     readable over any background (game art, gradients).
//   - GLASS tokens: translucent dark that lets the blurred background bleed
//     through. Used for expressive surfaces (library, chrome).
// Reads the persisted knobs from AppearanceSettings so the whole app re-themes
// when the user changes them.
QtObject {
    id: theme

    // Radix Slate — dark scale. Step roles (Radix convention):
    //   1-2 backgrounds · 3-5 component surfaces (normal/hover/active) ·
    //   6-8 borders (subtle/normal/focus) · 11 muted text · 12 primary text.
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

    // Persisted knobs.
    readonly property real intensity: AppearanceSettings.themeIntensity
    readonly property real glassAlpha: AppearanceSettings.glassOpacity

    // Tint source. In "image" mode the app tints itself from the chosen image
    // (its top/bottom bands, sampled into AppearanceSettings); otherwise from
    // the solid/gradient colors. The translucent glass surfaces render the real
    // top->bottom gradient (they show the image through them); themeColor is the
    // blend of the two, used by the flat solid tokens.
    readonly property bool _imageTint: AppearanceSettings.backgroundMode === "image"
                                       && AppearanceSettings.imageColorTop !== ""
    readonly property color tintTop: _imageTint ? AppearanceSettings.imageColorTop
                                                : AppearanceSettings.backgroundColor
    readonly property color tintBottom: _imageTint ? AppearanceSettings.imageColorBottom
                                       : (AppearanceSettings.backgroundMode === "gradient"
                                          ? AppearanceSettings.backgroundColor2
                                          : AppearanceSettings.backgroundColor)
    readonly property color themeColor: mix(tintTop, tintBottom, 0.5)

    // Composite `tint` over `base` at `amount` (0..1), returning a solid color.
    function blend(base, tint, amount) {
        return Qt.tint(base, Qt.rgba(tint.r, tint.g, tint.b, amount));
    }

    // Linear blend between two colors (t in 0..1).
    function mix(a, b, t) {
        return Qt.rgba(a.r + (b.r - a.r) * t,
                       a.g + (b.g - a.g) * t,
                       a.b + (b.b - a.b) * t, 1);
    }

    // A readable text/icon color to sit on top of `fill`.
    function onColor(fill) {
        return (0.299 * fill.r + 0.587 * fill.g + 0.114 * fill.b) > 0.55 ? "#111113" : "#ffffff";
    }

    // --- solid tokens (functional, Radix Slate tinted toward the theme color) ---
    readonly property color background: blend(_slate1, themeColor, intensity * 0.5)
    readonly property color surface: blend(_slate3, themeColor, intensity)
    readonly property color surfaceElevated: blend(_slate4, themeColor, intensity)
    readonly property color surfaceHover: blend(_slate5, themeColor, intensity)
    readonly property color border: blend(_slate6, themeColor, intensity * 0.6)
    readonly property color borderStrong: blend(_slate8, themeColor, intensity * 0.5)

    readonly property color textPrimary: _slate12
    readonly property color textMuted: _slate11

    // Radix accent + status (dark, step 9).
    readonly property color accent: "#f76b15"
    readonly property color onAccent: onColor(accent)

    readonly property color success: "#30a46c"
    readonly property color warning: "#ffc53d"
    readonly property color danger: "#e5484d"

    // --- glass tokens (translucent over the blurred background) ---
    // Kept a clean dark neutral (Discord-style): the color comes from the
    // background bleeding through the translucency, NOT from tinting the panel,
    // so surfaces stay dark and readable instead of muddy.
    readonly property color glass: Qt.rgba(_slate1.r, _slate1.g, _slate1.b, glassAlpha)
    readonly property color glassElevated: Qt.rgba(_slate1.r, _slate1.g, _slate1.b, Math.min(1, glassAlpha + 0.12))
    readonly property color glassBorder: Qt.rgba(1, 1, 1, 0.08)
}
