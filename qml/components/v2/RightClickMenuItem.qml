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

    implicitHeight: AppStyle.controlHeight
    // Derive from the content's *implicit* width (intrinsic), not contentItem.width
    // — the Menu drives item width from its own implicitWidth, so reading the
    // laid-out width here closes a binding loop that a scale change re-triggers
    implicitWidth: {
        var cw = contentItem.implicitWidth;
        if (control.maxWidth > 0 && cw > control.maxWidth) {
            cw = control.maxWidth;
        }
        if (cw < control.minWidth) {
            cw = control.minWidth;
        }
        return cw + leftPadding + rightPadding;
    }
    padding: AppStyle.spacingXs
    horizontalPadding: AppStyle.spacingSm

    Icon {
        id: externalIndicator
        visible: control.externalLink
        x: control.width - width
        y: control.height / 2 - height / 2
        width: parent.height - parent.padding * 2
        height: parent.height - parent.padding * 2
        size: AppStyle.iconSizeSm
        name: "open_in_new"
        color: Theme.textPrimary
    }

    arrow: Canvas {
        x: parent.width - width
        implicitWidth: Math.round(40 * AppStyle.scale)
        implicitHeight: Math.round(40 * AppStyle.scale)
        visible: control.subMenu
        // Coords are fractions of the (scaled) canvas so the triangle grows with
        // the UI; beginPath keeps repaints from accumulating stale segments
        onPaint: {
            var ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);
            ctx.beginPath();
            ctx.fillStyle = enabled ? Theme.textPrimary : "grey";
            ctx.moveTo(width * 0.35, height * 0.35);
            ctx.lineTo(width * 0.63, height / 2);
            ctx.lineTo(width * 0.35, height * 0.65);
            ctx.closePath();
            ctx.fill();
        }
    }

    Keys.onPressed: function (event) {
        if (event.key === Qt.Key_Back) {
            control.menu.close();
        }
    }

    HoverHandler {
        id: renameHover
        acceptedDevices: PointerDevice.Mouse
    }

    background: Rectangle {

        color: enabled ? (renameHover.hovered ? (control.dangerous ? Theme.danger : Theme.textPrimary) : "transparent") : "transparent"
        opacity: control.dangerous ? 1.0 : 0.1
        radius: AppStyle.radiusSm
    }

    contentItem: RowLayout {
        spacing: AppStyle.spacingMd
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
            color: enabled ? renameHover.hovered ? (control.dangerous ? Theme.danger : "white") : (control.dangerous ? Theme.danger : "#dfdfdf") : "grey"
            font.pixelSize: AppStyle.fontSizeMedium
            font.weight: Font.Normal
            font.family: AppStyle.fontFamily
            // font.weight: Font.DemiBold
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
        }
    }
}
