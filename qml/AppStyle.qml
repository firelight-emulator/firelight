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
    readonly property int fontSizeSmall: Math.round(14 * scale)  // captions, muted/secondary labels
    readonly property int fontSizeMedium: Math.round(15 * scale)  // body / default
    readonly property int fontSizeLarge: Math.round(22 * scale)   // subtitles, section headers
    readonly property int fontSizeXLarge: Math.round(32 * scale)  // page / hero titles

    // Spacing / gaps / padding — scale AND density (density is about how tightly
    // content packs)
    readonly property int spacingXs: Math.round(4 * scale * density)
    readonly property int spacingSm: Math.round(8 * scale * density)
    readonly property int spacingMd: Math.round(12 * scale * density)
    readonly property int spacingLg: Math.round(16 * scale * density)
    readonly property int spacingXl: Math.round(24 * scale * density)

    // Interactive heights — floored at minTarget
    readonly property int controlHeight: Math.max(minTarget, Math.round(36 * scale * density))
    readonly property int rowHeight: Math.max(minTarget, Math.round(44 * scale * density))

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

    readonly property int windowPadding: Math.round(18 * scale)

    // v2 button metrics: one height + a compact option, one fixed icon size
    readonly property int buttonHeight: Math.max(minTarget, Math.round(34 * scale * density))
    readonly property int buttonHeightCompact: Math.max(minTarget, Math.round(28 * scale * density))
    readonly property int iconSizeButton: Math.round(20 * scale)

    // Button metrics (color/opacity moved to Theme)
    readonly property int buttonStandardWidth: Math.round(200 * scale)
    readonly property int buttonStandardHeight: Math.max(minTarget, Math.round(42 * scale * density))
    readonly property int buttonTextFontSize: fontSizeMedium
    readonly property var buttonTextFontWeight: Font.DemiBold
    readonly property int buttonIconSize: iconSizeMd
    readonly property var buttonIconWeight: Font.Normal

    //*************************************************************
    // Title Bar
    //*************************************************************
    readonly property int titleBarHeight: Math.round(48 * scale)
    readonly property int titleBarPadding: Math.round(4 * scale)
    readonly property int titleBarUtilityButtonWidth: Math.round(48 * scale)
    readonly property int titleBarUtilityButtonIconSize: Math.round(10 * scale)

    //*************************************************************
    // Navigation Rail
    //*************************************************************
    readonly property int navRailWidth: Math.round(48 * scale)

    // TODO
    //*************************************************************
    // Motion — durations (ms) and a default easing curve, replacing the raw
    // duration literals scattered through the QML
    //*************************************************************
    readonly property int durationFast: 120
    readonly property int durationBase: 200
    readonly property int durationSlow: 320
    readonly property int easingStandard: Easing.OutCubic

    // TODO
    //*************************************************************
    // Elevation — MultiEffect blur radii (shadow colour is Theme.shadow)
    //*************************************************************
    readonly property int elevation1: 8
    readonly property int elevation2: 16
    readonly property int elevation3: 32

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
