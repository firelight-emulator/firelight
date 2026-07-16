import QtQuick
import QtQuick.Controls
import QtQuick.Layouts 1.0

MenuItem {
    id: control

    property string iconSource: ""
    property bool externalLink: false
    property bool dangerous: false
    property real minWidth: 0
    property real maxWidth: -1

    implicitHeight: 36
    implicitWidth: contentItem.width
    padding: 4
    horizontalPadding: 8

    Icon {
        id: externalIndicator
        visible: control.externalLink
        x: control.width - width
        y: control.height / 2 - height / 2
        width: parent.height - parent.padding * 2
        height: parent.height - parent.padding * 2
        size: 16
        name: "open_in_new"
        color: Theme.textPrimary
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

    contentItem: RowLayout {
        width: {
            if (control.maxWidth > 0 && implicitWidth > control.maxWidth) {
                return control.maxWidth
            } else if (implicitWidth < control.minWidth) {
                return control.minWidth
            } else {
                return implicitWidth
            }
        }
        spacing: 12
        Image {
            Layout.preferredHeight: parent.height * 0.84
            Layout.preferredWidth: height * 0.84
            sourceSize.width: 32
            sourceSize.height: 32
            source: control.iconSource
            fillMode: Image.PreserveAspectFit
            visible: control.iconSource !== undefined && control.iconSource !== ""
        }

        CarouselText {
            Layout.fillWidth: true
            Layout.topMargin: -1
            hovered: renameHover.hovered
            text: control.text
            color: enabled ? renameHover.hovered ? (control.dangerous ? ColorPalette.red700 : "white") : (control.dangerous ? ColorPalette.red500 : "#dfdfdf") : "grey"
            font.pixelSize: AppStyle.fontSizeMedium
            font.weight: Font.DemiBold
            font.family: Constants.regularFontFamily
            // font.weight: Font.DemiBold
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
        }
    }

    // contentItem: CarouselText {
    //     hovered: renameHover.hovered
    //     text: control.text
    //     color: enabled ? hovered ? "white" : (control.dangerous ? ColorPalette.red500 : "#dfdfdf") : "grey"
    //     leftPadding: 8
    //     rightPadding: 8
    //     font.pixelSize: 16
    //     font.weight: Font.DemiBold
    //     font.family: Constants.regularFontFamily
    //     // font.weight: Font.DemiBold
    //     horizontalAlignment: Text.AlignLeft
    //     verticalAlignment: Text.AlignVCenter
    // }
}