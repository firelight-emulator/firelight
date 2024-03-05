import QtQuick
import QtQuick.Controls
import FirelightStyle 1.0

MenuItem {
    id: control
    required property string labelText

    arrow: Canvas {
        x: parent.width - width
        implicitWidth: 40
        implicitHeight: 40
        visible: control.subMenu
        onPaint: {
            var ctx = getContext("2d")
            ctx.fillStyle = Constants.rightClickMenuItem_TextColor
            ctx.moveTo(14, 14)
            ctx.lineTo(width - 15, height / 2)
            ctx.lineTo(14, height - 14)
            ctx.closePath()
            ctx.fill()
        }
    }

    background: Rectangle {
        implicitWidth: 200
        implicitHeight: Constants.rightClickMenuItem_DefaultHeight
        radius: Constants.rightClickMenuItem_BackgroundRadius
        color: renameHover.hovered ? Constants.rightClickMenuItem_HoverColor : "transparent"

        HoverHandler {
            id: renameHover
            acceptedDevices: PointerDevice.Mouse
        }
    }

    contentItem: Text {
        text: control.labelText
        color: Constants.rightClickMenuItem_TextColor
        font.pointSize: 12
        font.family: Constants.regularFontFamily
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
    }
}