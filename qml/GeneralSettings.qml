import QtCore
import QtQuick

pragma Singleton

Settings {
    category: "Application"

    property bool fullscreen: false
    property bool showNewUserFlow: true
    property real mainWindowX: 200
    property real mainWindowY: 200
    property real mainWindowWidth: 1280
    property real mainWindowHeight: 720

    property string librarySortMethod: "alphabetical"
}