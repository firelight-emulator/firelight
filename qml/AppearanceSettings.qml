import QtCore
import QtQuick

pragma Singleton

// Persisted appearance/theme settings (QtCore Settings -> app QSettings INI,
// [Appearance] group). Every property here survives across launches for free.
Settings {
    category: "Appearance"

    property bool usingCustomBackground: false
    property string backgroundFile: ""

    property string accentColor: "#a55300"

    Behavior on accentColor {
        ColorAnimation { duration: 64 }
    }

    // --- Theme ---
    // "solid" | "gradient" | "image"
    property string backgroundMode: "solid"
    // Solid color, and gradient stop 1 / theme tint source. Stored as a "#rrggbb"
    // hex string (coerces to `color` where used, and round-trips the picker).
    property string backgroundColor: "#12131A"
    // Gradient stop 2 (only used in "gradient" mode).
    property string backgroundColor2: "#1b2333"

    // In "image" mode the theme tints itself from the image instead of the
    // colors above: these are sampled from the chosen image (top band / bottom
    // band) so surfaces adopt its palette. Empty until an image is sampled.
    property string imageColorTop: ""
    property string imageColorBottom: ""

    // Background blur amount (0..1).
    property real backgroundBlur: 0
    // Darkens an image background behind the frosted surfaces (0..1). Defaults
    // to a gentle dim so panels read dark and readable, Discord-style, instead
    // of a bright image bleeding straight through the glass.
    property real backgroundDim: 0.4
    // How strongly solid surfaces are tinted toward backgroundColor (0..1).
    property real themeIntensity: 0.12
    // Opacity of translucent "glass" surfaces over the background (0..1).
    property real glassOpacity: 0.55
}
