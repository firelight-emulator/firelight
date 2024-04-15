import QtQuick
import QtQuick.Controls
import FirelightStyle 1.0

MenuItem {
    id: control
    padding: 6

    arrow: Canvas {
        x: parent.width - width
        implicitWidth: 40
        implicitHeight: 40
        visible: control.subMenu
        onPaint: {
            var ctx = getContext("2d")
            ctx.fillStyle = enabled ? Constants.rightClickMenuItem_TextColor : "grey"
            ctx.moveTo(14, 14)
            ctx.lineTo(width - 15, height / 2)
            ctx.lineTo(14, height - 14)
            ctx.closePath()
            ctx.fill()
        }
    }

    background: Rectangle {
        implicitWidth: 260
        implicitHeight: Constants.rightClickMenuItem_DefaultHeight
        radius: Constants.rightClickMenuItem_BackgroundRadius
        color: enabled ? (renameHover.hovered ? Constants.rightClickMenuItem_HoverColor : "transparent") : "transparent"
        HoverHandler {
            id: renameHover
            acceptedDevices: PointerDevice.Mouse
        }
    }

    contentItem: CarouselText {
        hovered: renameHover.hovered
        text: control.text
        color: enabled ? Constants.rightClickMenuItem_TextColor : "grey"
        font.pointSize: 11
        font.family: Constants.semiboldFontFamily
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
    }
}