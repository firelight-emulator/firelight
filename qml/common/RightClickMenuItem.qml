import QtQuick
import QtQuick.Controls

MenuItem {
    id: control
    padding: 6

    property bool externalLink: false

    Text {
        id: externalIndicator
        visible: control.externalLink
        x: control.width - width - 8
        y: control.height / 2 - height / 2
        width: parent.height - parent.padding * 2
        height: parent.height - parent.padding * 2
        font.family: Constants.symbolFontFamily
        font.pointSize: 12
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        text: "\ue89e"
        color: "white"
    }

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
        x: 1
        y: 1
        width: control.width - 2
        height: control.height - 2
        radius: Constants.rightClickMenuItem_BackgroundRadius

        HoverHandler {
            id: renameHover
            acceptedDevices: PointerDevice.Mouse
        }

        implicitWidth: 260
        implicitHeight: Constants.rightClickMenuItem_DefaultHeight
        color: enabled ? (renameHover.hovered ? Constants.rightClickMenuItem_HoverColor : "transparent") : "transparent"
    }

    contentItem: CarouselText {
        hovered: renameHover.hovered
        text: control.text
        color: enabled ? Constants.rightClickMenuItem_TextColor : "grey"
        font.pointSize: 10
        font.family: Constants.regularFontFamily
        font.weight: Font.DemiBold
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
    }
}