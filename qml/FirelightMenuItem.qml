import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FirelightStyle 1.0

Button {
    id: control

    signal rightClicked()

    autoExclusive: true
    checkable: true
    padding: 6
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
            visible: labelIcon !== ""
            // width: 24

            font.family: Constants.symbolFontFamily
            font.pixelSize: 24
            color: (mouse.containsMouse || control.checked) ? "white" : "#b3b3b3"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
        Text {
            id: buttonText
            text: control.labelText
            anchors.left: labelIcon !== "" ? buttonIcon.right : parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            leftPadding: labelIcon !== "" ? 10 : 0
            font.pointSize: 11
            font.family: Constants.semiboldFontFamily
            color: (mouse.containsMouse || control.checked) ? "white" : "#b3b3b3"
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
        }
    }

    background: Rectangle {
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
        color: "transparent"
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
        cursorShape: Qt.PointingHandCursor
    }
}