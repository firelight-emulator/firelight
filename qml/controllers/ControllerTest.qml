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
                font.pixelSize: 18
                font.family: Constants.regularFontFamily
            }
            // Item {
            //     Image {
            //         id: image
            //         source: "file:system/_img/N64.svg"
            //         width: 512
            //         anchors.horizontalCenter: parent.horizontalCenter
            //         sourceSize.width: 512
            //         fillMode: Image.PreserveAspectFit
            //
            //         Rectangle {
            //             id: dpadUp
            //             color: "white"
            //             x: image.width * 0.244
            //             y: image.height * 0.304
            //             width: 22
            //             height: 20
            //         }
            //
            //         Rectangle {
            //             id: dpadDown
            //             color: "white"
            //             x: image.width * 0.615
            //             y: image.height * 0.325
            //             width: 30
            //             radius: height / 2
            //             height: 30
            //         }
            //     }
            // }
        }

        // contentItem: ListView {
        //     orientation: ListView.Horizontal
        //     spacing: 8
        //     model: ListModel {
        //         ListElement {
        //             name: "North Face"
        //         }
        //         ListElement {
        //             name: "South Face"
        //         }
        //         ListElement {
        //             name: "East Face"
        //         }
        //         ListElement {
        //             name: "West Face"
        //         }
        //         ListElement {
        //             name: "Start"
        //         }
        //         ListElement {
        //             name: "Select"
        //         }
        //         ListElement {
        //             name: "L1"
        //         }
        //         ListElement {
        //             name: "L2"
        //         }
        //         ListElement {
        //             name: "L3"
        //         }
        //         ListElement {
        //             name: "R1"
        //         }
        //         ListElement {
        //             name: "R2"
        //         }
        //         ListElement {
        //             name: "R3"
        //         }
        //     }
        //     delegate: Rectangle {
        //         id: dele
        //         required property var model
        //         width: 100
        //         height: 100
        //         radius: 8
        //         color: gamepad.connected ? gamepad.northFaceDown && model.name === "North Face" ? "yellow" : "dimgray" : "gray"
        //
        //         Text {
        //             text: dele.model.name
        //             anchors.centerIn: parent
        //             color: "white"
        //         }
        //     }
        // }

    }

}