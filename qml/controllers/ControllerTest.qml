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
    visibility: Window.Windowed

    title: qsTr("Firelight")

    background: Rectangle {
        color: "black"
    }

    GamepadStatus {
        id: gamepad
        playerNumber: 1
    }

    Pane {
        padding: 24
        anchors.centerIn: parent
        width: 300

        background: Rectangle {
            color: "black"
        }

        contentItem: ColumnLayout {
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: width
                color: "dimgray"
                radius: 6
            }
            Text {
                text: gamepad.name !== "" ? gamepad.name : "No Gamepad Connected"
                color: "white"
                font.pixelSize: AppStyle.fontSizeMedium
                font.family: Constants.regularFontFamily
            }
        }

    }

}
