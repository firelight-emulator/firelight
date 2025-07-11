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
    property bool pseudoChildFocused: false
    padding: 8

    implicitHeight: 42

    contentItem: Item {
        FLIcon {
            id: icon
            visible: control.labelIcon !== ""
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter

            icon: control.labelIcon ? control.labelIcon : "info"
            width: parent.height - 8
            height: parent.height - 8
            size: parent.height - 8
            fillMode: Image.PreserveAspectFit
            color: control.enabled ? control.activeFocus ? (!InputMethodManager.usingMouse ? "black" : "white") : "white" : "#aaaaaa"
        }

        Text {
            id: buttonText
            visible: control.width > 52
            anchors.left: icon.visible ? icon.right : parent.left
            anchors.leftMargin: icon.visible ? 16 : 0
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            // anchors.fill: parent

            text: control.labelText

            font.pixelSize: 16
            font.family: Constants.regularFontFamily
            font.weight: Font.DemiBold
            color: control.enabled ? control.activeFocus ? (!InputMethodManager.usingMouse ? "black" : "white") : "white" : "#aaaaaa"
            horizontalAlignment: alignRight ? Text.AlignRight : Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
        }
    }

    background: Rectangle {
        color: control.enabled ? "white" : "transparent"
        opacity: parent.activeFocus && !InputMethodManager.usingMouse ? 1 : control.pseudoChildFocused || parent.checked ? 0.1 : parent.pressed ? 0.1 : parent.hovered ? 0.2 : 0
        // opacity: control.enabled ? control.activeFocus ? (!InputMethodManager.usingMouse ? 1 : 0) : (mouse.containsMouse ? (control.pressed ? 0.1 : 0.2) : (control.pseudoChildFocused ? 0.3 : "transparent")) : "transparent"
        radius: 4
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