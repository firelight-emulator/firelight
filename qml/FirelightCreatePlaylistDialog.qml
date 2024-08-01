import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Effects

Dialog {
    id: control
    property alias text: playlistNameInput.text

    modal: true
    parent: Overlay.overlay
    anchors.centerIn: parent
    padding: 12

    background: Rectangle {
        color: Constants.colorTestSurfaceContainerLowest
        radius: 6
    }

    header: Text {
        padding: 12
        color: Constants.colorTestTextActive
        font.family: Constants.regularFontFamily
        text: qsTr("Create Playlist")
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.pointSize: 16
    }

    contentItem: Rectangle {
        color: Constants.colorTestSurface
        radius: 4
        clip: true

        HoverHandler {
            acceptedDevices: PointerDevice.Mouse
            cursorShape: Qt.IBeamCursor
        }

        TextInput {
            id: playlistNameInput
            anchors.fill: parent
            text: "text"
            color: Constants.colorTestTextActive
            padding: 12
            font.pointSize: 12
            font.family: Constants.lightFontFamily
            validator: RegularExpressionValidator {
                regularExpression: /^(?!\s*$).+/
            }

            onAccepted: function () {
                control.accept()
            }
        }
    }

    onAboutToShow: function () {
        playlistNameInput.clear()
        playlistNameInput.forceActiveFocus(Qt.PopupFocusReason)
    }

    footer: DialogButtonBox {
        visible: true
        alignment: Qt.AlignHCenter | Qt.AlignVCenter
        background: Rectangle {
            color: "transparent"
        }
        Button {
            background: Rectangle {
                color: enabled ? "white" : "grey"
                radius: 50
                implicitWidth: 160
                implicitHeight: 48
            }
            enabled: playlistNameInput.acceptableInput
            contentItem: Text {
                text: qsTr("Create")
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

    // onAccepted: {
    //     appRoot.state = "notPlayingGame"
    //     // closeGameAnimation.startWith(outPage, inPage)
    // }

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