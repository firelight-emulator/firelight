import QtCore
import QtQuick

pragma Singleton

Settings {
    category: "Appearance"

    property bool usingCustomBackground: false
    property string backgroundFile: ""
}