import QtCore
import QtQuick

pragma Singleton

Settings {
    category: "Application"

    property bool fullscreen: false
    property bool showNewUserFlow: true
    property bool showAdvancedSettings: false
    // Experimental TAS Studio (Phase 1). Off by default; gates the piano-roll screen.
    property bool enableTasStudio: false
    property real mainWindowX: 200
    property real mainWindowY: 200
    property real mainWindowWidth: 1280
    property real mainWindowHeight: 720

    property string librarySortMethod: "alphabetical"
}