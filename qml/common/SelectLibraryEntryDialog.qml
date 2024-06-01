import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: control
    property string text

    property string errorMessage: ""
    property bool showButtons: true
    modal: true
    parent: Overlay.overlay
    anchors.centerIn: parent
    padding: 12

    background: Rectangle {
        color: Constants.colorTestSurfaceContainerLowest
        radius: 6
        implicitWidth: 400
        implicitHeight: 200
    }

    header: Text {
        text: "Select a ROM from your library"
        color: "white"
        font.family: Constants.regularFontFamily
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.pointSize: 11
        leftPadding: 10
        rightPadding: 10
        topPadding: 8
        bottomPadding: 8
        font.weight: Font.DemiBold
    }

    contentItem: ColumnLayout {
        spacing: 12

        ComboBox {
            Layout.fillWidth: true
            editable: true
            model: library_short_model
            textRole: "display_name"
            valueRole: "id"
        }
    }

    footer: DialogButtonBox {
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

            onClicked: function () {
                control.reject()
            }

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
                text: qsTr("Select")
                anchors.centerIn: parent
                color: Constants.colorTestBackground
                font.family: Constants.regularFontFamily
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pointSize: 12
            }

            onClicked: function () {
                // achievement_manager.logInUserWithPassword(user.text, pass.text)
            }

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