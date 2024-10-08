import QtQuick

pragma Singleton

Item {
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


}