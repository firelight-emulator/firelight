import QtQuick
import QtQuick.Controls

MenuItem {
    id: control
    padding: 4

    focus: true
    property bool externalLink: false
    property bool dangerous: false

    implicitHeight: 48

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

    Keys.onPressed: function (event) {
        if (event.key === Qt.Key_Back) {
            control.menu.close()
        }
    }

    HoverHandler {
        id: renameHover
        acceptedDevices: PointerDevice.Mouse
    }

    background: Rectangle {
        // x: 1
        // y: 1

        // radius: Constants.rightClickMenuItem_BackgroundRadius
        // implicitHeight: Constants.rightClickMenuItem_DefaultHeight
        color: enabled ? (renameHover.hovered ? (control.dangerous ? ColorPalette.red700 : ColorPalette.neutral100) : "transparent") : "transparent"
        opacity: control.dangerous ? 1.0 : 0.1
        radius: 4
    }

    contentItem: CarouselText {
        hovered: renameHover.hovered
        text: control.text
        color: enabled ? hovered ? "white" : (control.dangerous ? ColorPalette.red500 : "#dfdfdf") : "grey"
        leftPadding: 8
        rightPadding: 8
        font.pixelSize: 16
        font.weight: Font.DemiBold
        font.family: Constants.regularFontFamily
        // font.weight: Font.DemiBold
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
    }
}