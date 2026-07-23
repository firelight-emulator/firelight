pragma Singleton
import QtQuick

Item {

    // RIGHT-CLICK MENU
    readonly property color rightClickMenu_BackgroundColor: "#282828"
    readonly property int rightClickMenu_BackgroundRadius: 4
    readonly property int rightClickMenu_Padding: 4

    // RIGHT-CLICK MENU ITEM
    readonly property int rightClickMenuItem_DefaultHeight: 36
    readonly property color rightClickMenuItem_HoverColor: "#3a3a3a"
    readonly property int rightClickMenuItem_BackgroundRadius: 4
    readonly property color rightClickMenuItem_TextColor: "#f1f1f1"

    readonly property color surface_color: "#121212"

    readonly property real standardTitleBarHeight: 60

    // TODO
    // Folder / label swatch palette — the pickable colours shared by the folder
    // colour menu and the colour setting control (data, not theme colours)
    readonly property var folderColors: ["#e5484d", "#f76b15", "#f5d90a", "#46a758", "#0091ff", "#8e4ec6", "#e93d82"]

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
    readonly property string mainFontFamily: localFont.name
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
}
