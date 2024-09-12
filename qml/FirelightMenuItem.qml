import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Button {
    id: control

    signal rightClicked()

    autoExclusive: true
    checkable: true
    property string labelText
    property string labelIcon
    property bool alignRight: false
    padding: 8

    contentItem: Text {
        id: buttonText
        visible: control.width > 52
        // anchors.fill: parent

        text: control.labelText

        font.pixelSize: 16
        font.family: Constants.regularFontFamily
        font.weight: Font.DemiBold
        color: control.enabled ? "white" : "#aaaaaa"
        horizontalAlignment: alignRight ? Text.AlignRight : Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
    }

    background: Rectangle {
        color: control.enabled ? "white" : "transparent"
        opacity: control.enabled ? (mouse.containsMouse ? (control.pressed ? 0.1 : 0.2) : (control.checked ? 0.3 : "transparent")) : "transparent"
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