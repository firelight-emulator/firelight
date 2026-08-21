import QtQuick
import QtQuick.Controls

ToolTip {
    id: root

    horizontalPadding: AppStyle.spacingLg
    verticalPadding: AppStyle.spacingMd

    background: Rectangle {
        color: Theme.surface
        radius: AppStyle.radiusMd
        border.color: Theme.border
        border.width: 1
    }

    contentItem: Text {
        text: root.text
        font.pixelSize: AppStyle.fontSizeMedium
        font.family: AppStyle.fontFamily
        color: Theme.switch2Color
        font.weight: Font.DemiBold
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    enter: Transition {
        NumberAnimation {
            property: "opacity"
            from: 0
            to: 1
            duration: 50
            easing.type: Easing.InOutQuad
        }
    }

    exit: Transition {
        NumberAnimation {
            property: "opacity"
            from: 1
            to: 0
            duration: 50
            easing.type: Easing.InOutQuad
        }
    }
}
