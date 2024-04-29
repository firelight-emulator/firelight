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
    padding: 8

    contentItem: Text {
        id: buttonText
        visible: control.width > 52
        // anchors.fill: parent

        text: control.labelText

        font.pointSize: 11
        font.family: Constants.regularFontFamily
        color: control.enabled ? "white" : "#aaaaaa"
        horizontalAlignment: Text.AlignRight
        verticalAlignment: Text.AlignVCenter
    }

    background: Rectangle {
        color: control.enabled ? "white" : "transparent"
        opacity: control.enabled ? (mouse.containsMouse ? 0.2 : (control.checked ? 0.3 : "transparent")) : "transparent"
        radius: 6
    }

    MouseArea {
        id: mouse
        anchors.fill: parent
        acceptedButtons: Qt.RightButton
        hoverEnabled: true
        onClicked: function (event) {
            control.rightClicked()
        }
        cursorShape: control.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
    }
}