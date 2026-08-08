pragma Singleton
import QtQuick

Item {
    // The metrics half of the design system: sizes only (fonts, spacing,
    // dimensions). Color lives in Theme — read color from Theme, size from here,
    // and never a literal at a call site
    //
    // Two user-facing multipliers feed every token, from the Interface scale /
    // Density settings. `scale` zooms everything (text and metrics); `density`
    // tightens/loosens spacing and row heights without touching font size
    // Tokens round to whole px so text stays crisp. This depends on
    // AppearanceSettings, which depends only on SettingBinding — no cycle
    readonly property real scale: AppearanceSettings.uiScale
    readonly property real density: AppearanceSettings.uiDensity

    // Interactive controls must never resolve below 24px (WCAG 2.5.8), whatever
    // scale/density do
    readonly property int minTarget: 24

    // Font size scale — device-independent px. Use these for text; never font.pointSize
    // (pointSize renders ~25% smaller on macOS than Windows due to a 72-vs-96 logical-DPI baseline)
    // TODO
    // Overline / section-label size (uppercase group headers)
    readonly property int fontSizeXSmall: Math.round(12 * scale)  // section labels, overlines
    readonly property int fontSizeSmall: Math.round(14 * scale)  // captions, muted/secondary labels
    readonly property int fontSizeMedium: Math.round(15 * scale)  // body / default
    readonly property int fontSizeLarge: Math.round(22 * scale)   // subtitles, section headers
    readonly property int fontSizeXLarge: Math.round(32 * scale)  // page / hero titles

    // TODO
    //*************************************************************
    // Fonts - one UI font plus the Material Symbols icon font
    //*************************************************************
    readonly property string fontFamily: _notosans.name
    readonly property string symbolFontFamily: _symbols.name

    FontLoader {
        id: _lexend
        source: "qrc:/fonts/lexend"
    }

    FontLoader {
        id: _notosans
        source: "qrc:/fonts/noto-sans"
    }

    FontLoader {
        id: _symbols
        source: "qrc:/fonts/symbols"
    }

    // Spacing / gaps / padding — scale AND density (density is about how tightly
    // content packs)
    readonly property int spacingXs: Math.round(4 * scale * density)
    readonly property int spacingSm: Math.round(8 * scale * density)
    readonly property int spacingMd: Math.round(12 * scale * density)
    readonly property int spacingLg: Math.round(16 * scale * density)
    readonly property int spacingXl: Math.round(24 * scale * density)

    // Interactive heights — floored at minTarget
    readonly property int controlHeight: Math.max(minTarget, Math.round(36 * scale * density))
    readonly property int rowHeight: Math.max(minTarget, Math.round(36 * scale * density))

    // Icon glyph sizes — scale only (an icon isn't a density concern)
    readonly property int iconSizeSm: Math.round(16 * scale)
    readonly property int iconSizeMd: Math.round(24 * scale)
    readonly property int iconSizeLg: Math.round(32 * scale)

    // TODO
    // Rounded tile behind a list-row icon (sidebar folder/platform chips)
    readonly property int iconChipSize: Math.round(26 * scale)

    // Corner radii
    readonly property int radiusSm: Math.round(4 * scale)
    readonly property int radiusMd: Math.round(8 * scale)
    readonly property int radiusLg: Math.round(12 * scale)

    // TODO
    // Border thickness of the controller-focus ring
    readonly property int focusRingWidth: Math.round(3 * scale)

    readonly property int windowPadding: Math.round(18 * scale)

    // v2 button metrics: one height + a compact option, one fixed icon size
    readonly property int buttonHeight: Math.max(minTarget, Math.round(42 * scale * density))
    readonly property int buttonHeightCompact: Math.max(minTarget, Math.round(28 * scale * density))
    readonly property int iconSizeButton: Math.round(22 * scale)

    // TODO
    // Comfortable default width for value inputs (slider/combobox) so they don't
    // collapse to their content width in a right-aligned settings slot
    readonly property int inputWidth: Math.round(200 * scale)

    // Button metrics (color/opacity moved to Theme)
    readonly property int buttonStandardWidth: Math.round(200 * scale)
    readonly property int buttonStandardHeight: Math.max(minTarget, Math.round(42 * scale * density))
    readonly property int buttonTextFontSize: fontSizeMedium
    readonly property var buttonTextFontWeight: Font.DemiBold
    readonly property int buttonIconSize: iconSizeMd
    readonly property var buttonIconWeight: Font.Normal


    readonly property int listRowHeight: Math.max(minTarget, Math.round(46 * scale * density))



    //*************************************************************
    // Title Bar
    //*************************************************************
    readonly property int titleBarHeight: Math.round(48 * scale)
    // TODO
    // Banner height for the in-game quick menu
    readonly property int standardTitleBarHeight: Math.round(60 * scale)
    readonly property int titleBarPadding: Math.round(4 * scale)
    readonly property int titleBarUtilityButtonWidth: Math.round(48 * scale)
    readonly property int titleBarUtilityButtonIconSize: Math.round(10 * scale)


    readonly property int defaultPopupMinimumWidth: Math.round(240 * scale)

    // TODO
    //*************************************************************
    // Library layout — sidebar full width, its collapsed icon rail, the width
    // below which sidebar item labels hide, and the right-side detail dock
    //*************************************************************
    readonly property int sidebarWidth: Math.round(280 * scale)
    readonly property int sidebarRailWidth: Math.round(56 * scale)
    readonly property int sidebarLabelThreshold: Math.round(64 * scale)
    readonly property int detailPanelWidth: Math.round(320 * scale)

    readonly property int artIconSizeSm: Math.round(48 * scale)
    readonly property int artIconSizeMd: Math.round(64 * scale)
    readonly property int artIconSizeLg: Math.round(96 * scale)

    // TODO
    //*************************************************************
    // Motion — durations (ms) and a default easing curve, replacing the raw
    // duration literals scattered through the QML
    //*************************************************************
    readonly property int durationVeryFast: 50
    readonly property int durationFast: 80
    readonly property int durationBase: 120
    readonly property int durationSlow: 200
    // TODO
    // Near-instant crossfade for the focus cursor's tile-to-tile handoff
    readonly property int durationSnap: 40
    // TODO
    // Beat between a menu choice showing as selected and the menu closing, so
    // the selection is seen before the surface goes away
    readonly property int confirmPause: 180
    // TODO
    // Outward velocity impulse of the focus cursor's edge bump
    readonly property real focusBumpKick: 1200
    // TODO
    // One full revolution of the focus ring's rotating gradient
    readonly property int durationSpin: 6600
    readonly property int easingStandard: Easing.OutCubic

    // TODO
    //*************************************************************
    // Elevation — MultiEffect drop-shadow preset: normalised blur (0..1) and
    // offset (px). Shadow colour is Theme.shadow
    //*************************************************************
    readonly property real elevationBlur: 1.0
    readonly property int elevationOffset: 4

    // TODO
    // Game-tile bevel lighting: rim brighten on top-facing edges, rim darken on
    // bottom-facing edges, and how far the bevel reaches in (fraction of the tile)
    readonly property real tileTopLight: 0.12
    readonly property real tileBottomShade: 0.16
    readonly property real tileBevelDepth: 0.06

    // TODO
    //*************************************************************
    // Stacking layers — one z scale, replacing ad-hoc z values
    //*************************************************************
    readonly property int zBase: 0
    readonly property int zDropdown: 100
    readonly property int zOverlay: 200
    readonly property int zToast: 300
    readonly property int zModal: 400
}
