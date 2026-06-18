import QtQuick

pragma Singleton

Item {
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
    readonly property var buttonTextFontSize: 16
    readonly property var buttonTextFontWeight: Font.DemiBold
    readonly property var buttonIconSize: 24
    readonly property var buttonIconWeight: Font.Normal

    readonly property color buttonTextColorDisabled: ColorPalette.neutral500
    readonly property color buttonTextColorInactive: "white"
    readonly property color buttonTextColorFocused: "black"

    //*************************************************************
    // Top Bar
    //*************************************************************
    readonly property real topBarHeight: 58
    readonly property real topBarPadding: 8
    readonly property real topBarUtilityButtonWidth: 48
    readonly property real topBarUtilityButtonIconSize: 10

    readonly property color topBarMinimizeButtonColorHovered: "white"
    readonly property real topBarMinimizeButtonOpacityHovered: 0.08
    readonly property color topBarMinimizeButtonColorPressed: "white"
    readonly property real topBarMinimizeButtonOpacityPressed: 0.05

    readonly property color topBarMaximizeButtonColorHovered: "white"
    readonly property real topBarMaximizeButtonOpacityHovered: 0.08
    readonly property color topBarMaximizeButtonColorPressed: "white"
    readonly property real topBarMaximizeButtonOpacityPressed: 0.05

    readonly property color topBarCloseButtonColorHovered: "#c42b1c"
    readonly property real topBarCloseButtonOpacityHovered: 1
    readonly property color topBarCloseButtonColorPressed: "#941320"
    readonly property real topBarCloseButtonOpacityPressed: 1

    //*************************************************************
    // Something Else
    //*************************************************************






}