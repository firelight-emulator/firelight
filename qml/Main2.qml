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

    onActiveFocusItemChanged: {
        console.log("Active focus item changed to: " + activeFocusItem)
    }

    StackView {
        id: screenStack
        anchors.fill: parent
        initialItem: homeScreen
        focus: true
    }

    Component {
        id: homeScreen

        HomeScreen {
        }
    }

    Component {
        id: settingsScreen

        Rectangle {
            color: "red"
        }
    }

    Component {
        id: emulatorScreen

        Rectangle {
            color: "red"
        }
    }
}
