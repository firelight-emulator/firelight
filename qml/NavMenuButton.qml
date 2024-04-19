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
    leftPadding: 8
    rightPadding: 8
    contentItem: Row {
        anchors.fill: parent
        spacing: 8
        leftPadding: control.leftPadding
        rightPadding: control.rightPadding

        Text {
            id: buttonIcon
            text: control.labelIcon
            visible: labelIcon !== ""
            // width: 24
            height: parent.height

            font.family: Constants.symbolFontFamily
            font.pixelSize: 24
            color: control.checked ? "black" : "#dadada"
            opacity: control.enabled ? (control.checked ? 0.8 : 1) : 0.65
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
        Text {
            id: buttonText
            visible: control.width > 52
            text: control.labelText
            height: parent.height
            font.pointSize: 11
            font.family: Constants.semiboldFontFamily
            color: control.checked ? "black" : "#dadada"
            opacity: control.enabled ? (control.checked ? 0.8 : 1) : 0.65
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
        }
    }

    background: Rectangle {
        color: "#dadada"
        opacity: control.enabled ? (control.checked ? 1.0 : mouse.containsMouse ? 0.2 : 0.0) : 0.0
        // color: control.checked ?
        //     (mouse.pressed ?
        //         "#2b2b2b"
        //         : ((mouse.containsMouse ?
        //             "#393939"
        //             : "#232323")))
        //     : (mouse.pressed ?
        //         "#000000"
        //         : (mouse.containsMouse ?
        //             "#1a1a1a"
        //             : "transparent"))
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