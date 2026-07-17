import QtCore
import QtQuick

pragma Singleton

// Where the window was last time. Not a setting: nobody chooses it, it isn't in
// the settings catalog, and it shouldn't be searchable or resettable. It just
// needs to survive a restart, which is exactly what QSettings is for.
Settings {
    category: "Window"

    property real mainWindowX: 200
    property real mainWindowY: 200
    property real mainWindowWidth: 1280
    property real mainWindowHeight: 720
}
