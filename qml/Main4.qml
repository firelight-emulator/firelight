import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Window
import QtQml.Models
import QtQuick.Layouts 1.0
import QtNetwork
import QtMultimedia
import QtQuick.Effects
import Firelight 1.0

ApplicationWindow {
    id: window
    objectName: "Application Window"

    width: GeneralSettings.mainWindowWidth
    height: GeneralSettings.mainWindowHeight
    x: GeneralSettings.mainWindowX
    y: GeneralSettings.mainWindowY

    onWidthChanged: {
        GeneralSettings.mainWindowWidth = width
    }

    onHeightChanged: {
        GeneralSettings.mainWindowHeight = height
    }

    onXChanged: {
        GeneralSettings.mainWindowX = x
    }

    onYChanged: {
        GeneralSettings.mainWindowY = y
    }
    visible: true
    visibility: GeneralSettings.fullscreen ? Window.FullScreen : Window.Windowed

    title: qsTr("Firelight")



}
