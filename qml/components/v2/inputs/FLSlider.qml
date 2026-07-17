import QtQuick
import QtQuick.Controls
import Firelight 1.0

// Themed slider. Track/handle from Theme; the control's touch height is floored
// at minTarget.
Slider {
    id: control

    implicitHeight: Math.max(AppStyle.minTarget, AppStyle.controlHeight)

    background: Rectangle {
        x: control.leftPadding
        y: control.topPadding + control.availableHeight / 2 - height / 2
        width: control.availableWidth
        height: Math.round(4 * AppStyle.scale)
        radius: height / 2
        color: Theme.surfaceHover

        Rectangle {
            width: control.visualPosition * parent.width
            height: parent.height
            radius: height / 2
            color: Theme.accent
        }
    }

    handle: Rectangle {
        x: control.leftPadding + control.visualPosition * (control.availableWidth - width)
        y: control.topPadding + control.availableHeight / 2 - height / 2
        width: AppStyle.iconSizeSm
        height: width
        radius: width / 2
        color: "white"
        border.width: 1
        border.color: control.activeFocus ? Theme.accent : Theme.border
    }
}
