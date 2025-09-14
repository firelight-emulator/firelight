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

    background: Rectangle {
        color: "grey"
    }

    onActiveFocusItemChanged: {
        console.log("Active focus item changed to: " + window.activeFocusItem)
        let item = window.activeFocusItem
        let level = 0
        while (item) {
            let spaces = " ".repeat(level * 2)

            console.log(spaces + item)
            item = item.parent
            level++
        }
    }

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

    RadioIconButton {
        icon.source: "qrc:/icons/sort"
        icon.width: 60
        icon.height: 60
        anchors.centerIn: parent
        focus: true
        model: [
              { label: "Option 1", value: "opt1" },
              { label: "Option 2", value: "opt2" },
              { label: "Option 3", value: "opt3" }
          ]

          onSelectionChanged: function(index, value, text) {
              console.log("Selected:", index, value, text)
          }

    }

    FLFocusHighlight {
        target: window.activeFocusItem
        usingMouse: InputMethodManager.usingMouse
        z: 1000
    }



}
