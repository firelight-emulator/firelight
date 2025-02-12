import QtCore
import QtQuick

pragma Singleton

Settings {
    category: "Application"

    property bool fullscreen: false
    property bool showNewUserFlow: true
}