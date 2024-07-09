import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Window
import QtQml.Models
import QtQuick.Layouts 1.0
import QtQuick.Effects
import Firelight 1.0

ApplicationWindow {
    id: window
    objectName: "Application Window"

    width: 1280
    height: 720

    // font.family: "Segoe UI"

    // flags: Qt.FramelessWindowHint

    visible: true

    title: qsTr("Firelight")

    background: Rectangle {
        color: "#1a1b1e"
    }

    onActiveFocusItemChanged: {
        if (activeFocusItem === null) {
            return
        }

        activeFocusItem.grabToImage(function (result) {
            debugImage.width = result.width
            debugImage.height = result.height
            debugImage.source = result.url
        })
    }

    Window {
        id: debugWindow
        objectName: "DebugWindow"

        width: 400
        height: 400

        visible: false
        title: "Debug"

        color: "black"

        Image {
            id: debugImage
            source: ""
            anchors.centerIn: parent
            fillMode: Image.PreserveAspectFit
        }

    }

    EmulatorScreen {
        id: emulatorScreen

        onGameStopped: function () {
            screenStack.pushItem(homeScreen, {}, StackView.Immediate)
        }

    }

    // Rectangle {
    //     id: cursor
    //     parent: window.activeFocusItem
    //     anchors.fill: parent
    //     anchors.margins: -2
    //     color: "transparent"
    //     border.color: "lightblue"
    //     border.width: 3
    //     radius: 4
    // }

    StackView {
        id: screenStack
        anchors.fill: parent
        focus: true

        Component.onCompleted: {
            screenStack.pushItems([emulatorScreen, homeScreen])
        }

        Keys.onPressed: function (event) {
            if (event.key === Qt.Key_F12) {
                debugWindow.visible = !debugWindow.visible
            }
        }
    }

    Component {
        id: homeScreen

        HomeScreen {
            onStartGame: function (entryId) {
                screenStack.popCurrentItem()
                emulatorScreen.loadGame(entryId)
            }
        }
    }

    Component {
        id: settingsScreen

        Rectangle {
            color: "red"
        }
    }
}
