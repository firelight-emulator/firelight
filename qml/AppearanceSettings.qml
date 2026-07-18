pragma Singleton

import QtQuick
import Firelight 1.0

// Typed read path for the appearance settings declared in the settings catalog
// The settings page writes them through SettingsModel; Theme and everything else
// bind to the named properties here. Both go to the same store, so there's one
// source of truth and one place each key's name is spelled
//
// Read-only by design: assigning to one of these would break its binding and
// silently stop tracking the stored value. Anything that needs to write a
// setting outside the settings page should use its own SettingBinding
QtObject {
    id: root

    property SettingBinding accentBinding: SettingBinding { key: "accent-color" }
    property SettingBinding modeBinding: SettingBinding { key: "background-mode" }
    property SettingBinding colorBinding: SettingBinding { key: "background-color" }
    property SettingBinding color2Binding: SettingBinding { key: "background-color-2" }
    property SettingBinding fileBinding: SettingBinding { key: "background-file" }
    property SettingBinding blurBinding: SettingBinding { key: "background-blur" }
    property SettingBinding dimBinding: SettingBinding { key: "background-dim" }
    property SettingBinding tintBinding: SettingBinding { key: "theme-intensity" }
    property SettingBinding glassBinding: SettingBinding { key: "glass-opacity" }
    property SettingBinding tileSizeBinding: SettingBinding { key: "library-tile-size" }
    property SettingBinding scaleBinding: SettingBinding { key: "interface-scale" }
    property SettingBinding densityBinding: SettingBinding { key: "interface-density" }

    // Colors are "#rrggbb" strings, not `color`: that's what consumers were
    // built against, they coerce to `color` where used, and they round-trip the
    // picker unchanged
    property string accentColor: accentBinding.value

    Behavior on accentColor {
        ColorAnimation { duration: 64 }
    }

    // "solid" | "gradient" | "image"
    readonly property string backgroundMode: modeBinding.value
    // Solid color, and gradient stop 1 / theme tint source
    readonly property string backgroundColor: colorBinding.value
    // Gradient stop 2 (only used in "gradient" mode)
    readonly property string backgroundColor2: color2Binding.value

    // The setting stores a plain path — that's what the file picker deals in —
    // but Image.source and samplePalette both need a URL. Convert once, here,
    // rather than at every use site
    readonly property string backgroundFile: fileBinding.value === ""
                                             ? "" : FilesystemUtils.prependFileURI(fileBinding.value)

    readonly property real backgroundBlur: parseFloat(blurBinding.value)
    // Darkens an image background behind the frosted surfaces, so panels read
    // dark and readable instead of a bright image bleeding through the glass
    readonly property real backgroundDim: parseFloat(dimBinding.value)
    // How strongly solid surfaces are tinted toward backgroundColor
    readonly property real themeIntensity: parseFloat(tintBinding.value)
    // Opacity of translucent "glass" surfaces over the background
    readonly property real glassOpacity: parseFloat(glassBinding.value)
    // Library grid cover-art edge, in px. The grid binds cellWidth/cellHeight
    // to this and GameTile decodes to match
    readonly property int libraryTileSize: parseInt(tileSizeBinding.value)

    // Accessibility scaling. AppStyle multiplies its metric tokens by these:
    // uiScale enlarges everything (fonts + dimensions), uiDensity only affects
    // spacing/heights. The `|| ` guards a not-yet-loaded binding from poisoning
    // every token with NaN
    readonly property real uiScale: parseFloat(scaleBinding.value) || 1.0
    readonly property real uiDensity: parseFloat(densityBinding.value) || 1.0

    // In image mode the theme tints itself from the image rather than from
    // backgroundColor. Derived, not stored: it's a function of the chosen file,
    // so sampling it on demand can't fall out of sync the way a saved copy did
    readonly property var imagePalette: (backgroundMode === "image" && backgroundFile !== "")
                                        ? ImageUtils.samplePalette(backgroundFile) : null
    readonly property string imageColorTop: (imagePalette && imagePalette.ok) ? imagePalette.top : ""
    readonly property string imageColorBottom: (imagePalette && imagePalette.ok) ? imagePalette.bottom : ""
}
