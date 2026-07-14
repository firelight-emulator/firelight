import QtQuick

pragma Singleton

Item {
    // Font size scale — device-independent px. Use these for text; never font.pointSize
    // (pointSize renders ~25% smaller on macOS than Windows due to a 72-vs-96 logical-DPI baseline).
    readonly property int fontSizeSmall: 13   // captions, muted/secondary labels
    readonly property int fontSizeMedium: 16  // body / default
    readonly property int fontSizeLarge: 22   // subtitles, section headers
    readonly property int fontSizeXLarge: 32  // page / hero titles

    readonly property var windowPadding: 18
    readonly property var mainHeaderHeight: 42 + mainHeaderPadding * 2
    readonly property var mainHeaderPadding: 18

    readonly property color buttonBackgroundColorDisabled: ColorPalette.neutral700
    readonly property color buttonBackgroundColorInactive: ColorPalette.neutral800
    readonly property color buttonBackgroundColorHovered: ColorPalette.neutral700
    readonly property color buttonBackgroundColorPressed: ColorPalette.neutral900
    readonly property color buttonBackgroundColorFocused: ColorPalette.neutral100

    readonly property var buttonBackgroundOpacityDisabled: 0
    readonly property var buttonBackgroundOpacityInactive: 0.15
    readonly property var buttonBackgroundOpacityHovered: 0.3
    readonly property var buttonBackgroundOpacityPressed: 0.2
    readonly property var buttonBackgroundOpacityFocused: 1

    readonly property var buttonStandardWidth: 200
    readonly property var buttonStandardHeight: 42
    readonly property int buttonTextFontSize: fontSizeMedium
    readonly property var buttonTextFontWeight: Font.DemiBold
    readonly property var buttonIconSize: 24
    readonly property var buttonIconWeight: Font.Normal

    readonly property color buttonTextColorDisabled: ColorPalette.neutral500
    readonly property color buttonTextColorInactive: "white"
    readonly property color buttonTextColorFocused: "black"

    //*************************************************************
    // Top Bar
    //*************************************************************
    readonly property real titleBarHeight: 58
    readonly property real titleBarPadding: 8
    readonly property real titleBarUtilityButtonWidth: 48
    readonly property real titleBarUtilityButtonIconSize: 10

    readonly property color titleBarMinimizeButtonColorHovered: "white"
    readonly property real titleBarMinimizeButtonOpacityHovered: 0.08
    readonly property color titleBarMinimizeButtonColorPressed: "white"
    readonly property real titleBarMinimizeButtonOpacityPressed: 0.05

    readonly property color titleBarMaximizeButtonColorHovered: "white"
    readonly property real titleBarMaximizeButtonOpacityHovered: 0.08
    readonly property color titleBarMaximizeButtonColorPressed: "white"
    readonly property real titleBarMaximizeButtonOpacityPressed: 0.05

    readonly property color titleBarCloseButtonColorHovered: "#c42b1c"
    readonly property real titleBarCloseButtonOpacityHovered: 1
    readonly property color titleBarCloseButtonColorPressed: "#941320"
    readonly property real titleBarCloseButtonOpacityPressed: 1

    //*************************************************************
    // Something Else
    //*************************************************************






}