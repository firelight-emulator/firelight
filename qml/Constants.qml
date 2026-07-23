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
        id: lexend
        source: "qrc:/fonts/lexend"
    }

    readonly property string symbolFontFamily: symbols.name

    // TODO
    // One UI typeface — the Lexend variable font. Weight is chosen per call site
    // via font.weight, so every family alias resolves to the same family
    readonly property string regularFontFamily: lexend.name
    readonly property string mainFontFamily: lexend.name
    readonly property string lightFontFamily: lexend.name
    readonly property string strongFontFamily: lexend.name
    readonly property string semiboldFontFamily: lexend.name

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
