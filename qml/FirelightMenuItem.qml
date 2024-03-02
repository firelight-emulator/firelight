import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FirelightStyle 1.0

Button {
    id: control

    signal rightClicked()

    autoExclusive: true
    checkable: true
    property string labelText
    property string labelIcon
    contentItem: Item {
        anchors.fill: parent

        Text {
            id: buttonIcon
            text: control.labelIcon
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            leftPadding: 12
            visible: labelIcon !== ""
            // width: 24

            font.family: Constants.symbolFontFamily
            font.pixelSize: 24
            color: control.checked ? Constants.colorTestTextActive : Constants.colorTestTextActive
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
        Text {
            id: buttonText
            text: control.labelText
            anchors.left: labelIcon !== "" ? buttonIcon.right : parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            leftPadding: 12
            font.pointSize: 12
            font.family: Constants.regularFontFamily
            color: control.checked ? Constants.colorTestTextActive : Constants.colorTestTextActive
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
        }
    }

    background: Rectangle {
        color: control.checked ? Constants.colorTestCardActive : mouse.containsMouse ? Constants.colorTestCard : "transparent"
        radius: 8
    }

    MouseArea {
        id: mouse
        anchors.fill: parent
        acceptedButtons: Qt.RightButton
        hoverEnabled: true
        onClicked: function (event) {
            if (event.button === Qt.RightButton) {
                control.rightClicked()
            }
        }
        cursorShape: Qt.PointingHandCursor
    }
}