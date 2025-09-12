import QtQuick

pragma Singleton

Item {

    readonly property color purpleText: "#5a4854"
    readonly property color purpleTextLight: "#ad9cac"
    readonly property color purpleBackground: "#fafcf7"

    // RIGHT-CLICK MENU
    readonly property color rightClickMenu_BackgroundColor: "#282828"
    readonly property int rightClickMenu_BackgroundRadius: 4
    readonly property int rightClickMenu_Padding: 4

    // RIGHT-CLICK MENU ITEM
    readonly property int rightClickMenuItem_DefaultHeight: 36
    readonly property color rightClickMenuItem_HoverColor: "#3a3a3a"
    readonly property int rightClickMenuItem_BackgroundRadius: 4
    readonly property color rightClickMenuItem_TextColor: "#f1f1f1"
    readonly property int rightClickMenuItem_textPointSize: 12

    readonly property color surface_color: "#121212"


    FontLoader {
        id: symbols
        source: "qrc:/fonts/symbols"
    }

    FontLoader {
        id: main
        source: "qrc:/fonts/main"
    }

    FontLoader {
        id: localFont
        source: "qrc:/fonts/lexend"
    }

    FontLoader {
        id: lexendLight
        source: "qrc:/fonts/lexend-light"
    }

    FontLoader {
        id: lexendBlack
        source: "qrc:/fonts/lexend-black"
    }

    FontLoader {
        id: semibold
        source: "qrc:/fonts/font-semibold"
    }

    readonly property string symbolFontFamily: symbols.name
    readonly property string regularFontFamily: "Segoe UI"
    readonly property string lightFontFamily: localFont.name
    readonly property string strongFontFamily: lexendBlack.name
    readonly property string semiboldFontFamily: semibold.name

    readonly property color colorTestSurface: "#161616"
    readonly property color colorTestBackground: "#212020"
    readonly property color colorTestSurfaceContainerLowest: "#282727"
    readonly property color colorTestSurfaceVariant: "#373636"
    readonly property color colorTestTextActive: "#f0f0f0"
    readonly property color colorTestText: "#c2c2c2"
    readonly property color colorTestTextMuted: "#737373"
    readonly property color colorTestCard: "#48240c"
    readonly property color colorTestCardActive: "#833800"


    readonly property color color_primary: "#FFB787"
    readonly property color color_surfaceTint: "#FFB787"
    readonly property color color_onPrimary: "#502400"
    readonly property color color_primaryContainer: "#6E390E"
    readonly property color color_onPrimaryContainer: "#FFDCC7"
    readonly property color color_secondary: "#E5BFA8"
    readonly property color color_onSecondary: "#422B1B"
    readonly property color color_secondaryContainer: "#5B4130"
    readonly property color color_onSecondaryContainer: "#FFDCC7"
    readonly property color color_tertiary: "#C9CA93"
    readonly property color color_onTertiary: "#31320A"
    readonly property color color_tertiaryContainer: "#48491F"
    readonly property color color_onTertiaryContainer: "#E6E6AD"
    readonly property color color_error: "#FFB4AB"
    readonly property color color_onError: "#690005"
    readonly property color color_errorContainer: "#c01d1d"
    readonly property color color_onErrorContainer: "#FFDAD6"
    readonly property color color_background: "#19120D"
    readonly property color color_onBackground: "#F0DFD7"
    readonly property color color_surface: "#19120D"
    readonly property color color_onSurface: "#F0DFD7"
    readonly property color color_surfaceVariant: "#52443C"
    readonly property color color_onSurfaceVariant: "#D7C3B8"
    readonly property color color_outline: "#9F8D83"
    readonly property color color_outlineVariant: "#52443C"
    readonly property color color_shadow: "#000000"
    readonly property color color_scrim: "#000000"
    readonly property color color_inverseSurface: "#F0DFD7"
    readonly property color color_inverseOnSurface: "#382E29"
    readonly property color color_inversePrimary: "#8B4F24"
    readonly property color color_primaryFixed: "#FFDCC7"
    readonly property color color_onPrimaryFixed: "#311300"
    readonly property color color_primaryFixedDim: "#FFB787"
    readonly property color color_onPrimaryFixedVariant: "#6E390E"
    readonly property color color_secondaryFixed: "#FFDCC7"
    readonly property color color_onSecondaryFixed: "#2B1708"
    readonly property color color_secondaryFixedDim: "#E5BFA8"
    readonly property color color_onSecondaryFixedVariant: "#5B4130"
    readonly property color color_tertiaryFixed: "#E6E6AD"
    readonly property color color_onTertiaryFixed: "#1C1D00"
    readonly property color color_tertiaryFixedDim: "#C9CA93"
    readonly property color color_onTertiaryFixedVariant: "#48491F"
    readonly property color color_surfaceDim: "#19120D"
    readonly property color color_surfaceBright: "#413731"
    readonly property color color_surfaceContainerLowest: "#140D08"
    readonly property color color_surfaceContainerLow: "#221A15"
    readonly property color color_surfaceContainer: "#261E19"
    readonly property color color_surfaceContainerHigh: "#312823"
    readonly property color color_surfaceContainerHighest: "#3D332D"
}