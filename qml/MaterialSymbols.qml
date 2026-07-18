import QtQuick

pragma Singleton

// TODO
// Material Symbols Rounded codepoint map. Keyed by the app's existing icon names
// (the qrc:/icons/* aliases) plus canonical names for the button glyphs. Generated
// from assets/fonts/MaterialSymbolsRounded[FILL,GRAD,opsz,wght].ttf via its cmap
// Used by Icon.qml: Icon { name: "settings" }
QtObject {
    readonly property var codepoints: ({
        "add": "\ue145",
        "add-circle": "\ue3ba",
        "arrow": "\ue5c8",
        "arrow-back": "\ue5c4",
        "arrow-forward": "\ue5c8",
        "arrow-down": "\ue5db",
        "arrow-up": "\ue5d8",
        "bar-chart": "\ue26b",
        "bell": "\ue7f5",
        "book": "\uf53e",
        "bookmark-star": "\uf454",
        "browse": "\ueb13",
        "cancel": "\ue888",
        "controller": "\uf135",
        "chevron-back": "\ue5cb",
        "chevron-forward": "\ue5cc",
        "close": "\ue5cd",
        "delete": "\ue92e",
        "display": "\ue30c",
        "favorite": "\ue87e",
        "folder": "\ue2c7",
        "history": "\ue8b3",
        "home": "\ue9b2",
        "info": "\ue88e",
        "kid-star": "\uf526",
        "left-panel-close": "\uf717",
        "left-panel-open": "\uf716",
        "menu": "\ue5d2",
        "more-vertical": "\ue5d4",
        "more-horizontal": "\ue5d3",
        "new-folder": "\ue2cc",
        "online": "\ue837",
        "palette": "\ue40a",
        "photo-library": "\ue413",
        "profile": "\ue853",
        "search": "\ue8b6",
        "settings": "\ue8b8",
        "shopping-bag": "\uf1cc",
        "sort": "\ue164",
        "speaker": "\ue98e",
        "star": "\uf09a",
        "trophy": "\uea23",
        "undo": "\ue166",
        "play-circle": "\ue1c4",
        "power": "\uf8c7",
        "arrow_back": "\ue5c4",
        "arrow_back_ios": "\ue5e0",
        "arrow_drop_down": "\ue5c5",
        "book_2": "\uf53e",
        "check_circle": "\ue86c",
        "more_vert": "\ue5d4",
        "open_in_new": "\ue89e",
        "subdirectory_arrow_right": "\ue5da",
        "wifi": "\ue63e",
        "wifi_off": "\ue648"
    })

    function glyph(name) {
        return codepoints[name] !== undefined ? codepoints[name] : ""
    }
}
