import QtQuick
import QtQuick.Controls
// import QtQuick.Controls.Material
import QtQuick.Dialogs
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Effects
import Firelight 1.0
import FirelightStyle 1.0

Dialog {
    id: control
    property string text
    property bool showButtons: true
    modal: true
    parent: Overlay.overlay
    anchors.centerIn: parent
    padding: 12

    background: Rectangle {
        color: Constants.colorTestSurfaceContainerLowest
        radius: 6
        implicitWidth: 480
        implicitHeight: 200
    }

    contentItem: Text {
        // anchors.centerIn: parent
        color: Constants.colorTestTextActive
        font.family: Constants.regularFontFamily
        text: control.text
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.pointSize: 14
    }

    footer: DialogButtonBox {
        visible: control.showButtons
        background: Rectangle {
            color: "transparent"
        }
        Button {
            background: Rectangle {
                color: "transparent"
            }
            contentItem: Text {
                text: qsTr("Cancel")
                // anchors.centerIn: parent
                color: Constants.colorTestTextActive
                font.family: Constants.regularFontFamily
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pointSize: 12
            }
            DialogButtonBox.buttonRole: DialogButtonBox.RejectRole

            HoverHandler {
                acceptedDevices: PointerDevice.Mouse
                cursorShape: Qt.PointingHandCursor
            }
        }
        Button {
            anchors.verticalCenter: parent.verticalCenter
            background: Rectangle {
                color: "white"
                radius: 6
            }
            contentItem: Text {
                text: qsTr("Yes")
                anchors.centerIn: parent
                color: Constants.colorTestBackground
                font.family: Constants.regularFontFamily
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pointSize: 12
            }
            DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole

            HoverHandler {
                acceptedDevices: PointerDevice.Mouse
                cursorShape: Qt.PointingHandCursor
            }
        }
    }

    Overlay.modal: Rectangle {
        color: "black"
        anchors.fill: parent
        opacity: 0.4

        Behavior on opacity {
            NumberAnimation {
                duration: 200
            }
        }
    }

    enter: Transition {
        NumberAnimation {
            property: "opacity";
            from: 0.0;
            to: 1.0
            duration: 200
        }
        NumberAnimation {
            property: "scale";
            from: 0.9;
            to: 1.0
            duration: 200
        }
    }

    exit: Transition {
        NumberAnimation {
            property: "opacity";
            from: 1.0;
            to: 0.0;
            duration: 200
        }
        NumberAnimation {
            property: "scale";
            from: 1.0;
            to: 0.9
            duration: 200
        }
    }
}