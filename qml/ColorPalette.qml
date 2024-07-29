import QtQuick

pragma Singleton

Item {
    readonly property color primary800: "#142850"
    readonly property color primary700: "#27496d"
    readonly property color primary600: "#0c7b93"
    readonly property color primary500: "#00a8cc"
    readonly property color primary400: "#3bc9db"
    readonly property color primary300: "#76e4ef"
    readonly property color primary200: "#c4f1fa"

    readonly property color accent800: "#623d1a"
    readonly property color accent700: "#7a4f23"
    readonly property color accent600: "#94622d"
    readonly property color accent500: "#b77c38"
    readonly property color accent400: "#d89e61"
    readonly property color accent300: "#ebc299"
    readonly property color accent200: "#f6e1c7"

    readonly property color neutral1000: "#0e0e0e"
    readonly property color neutral900: "#1e1e1e"
    readonly property color neutral800: "#2a2a2a"
    readonly property color neutral700: "#444444"
    readonly property color neutral600: "#666666"
    readonly property color neutral500: "#888888"
    readonly property color neutral400: "#aaaaaa"
    readonly property color neutral300: "#cccccc"
    readonly property color neutral200: "#e6e6e6"
    readonly property color neutral100: "#f9f9f9"


    readonly property color red900: "#b71c1c"
    readonly property color red700: "#ca1f17"
    readonly property color red500: "#d62727"
    readonly property color red100: "#f8d7da"

    readonly property color white: neutral200

    readonly property color containerVeryLowColor: neutral900
    readonly property color containerLowColor: neutral800
    readonly property color containerMidColor: neutral700


    readonly property color dropdownBackgroundColor: containerMidColor
    readonly property color dropdownPopupBackgroundColor: containerVeryLowColor
    readonly property color dropdownPopupBorderColor: containerMidColor
    readonly property color dropdownInactiveTextColor: neutral400


    readonly property color backgroundColorOne: "#2d2d32"
    readonly property color backgroundColorTwo: "#353739"
    readonly property color popupBackgroundColor: "#37393b"
    readonly property color buttonColor: "#4c4e52"
    readonly property color hyperlinkColor: "#89c5f9"
    readonly property color strokeColor: "#1d1d1d"
    readonly property color fontColorOne: "#e7ebee"
    readonly property color fontColorTwo: "#b8bcc0"
    readonly property color textFieldColor: "#242428"
    readonly property color whiteColor: "#f8fcfc"
    readonly property color blackColor: "#000000"
    readonly property color accentColor: "#ee7c1e"
}