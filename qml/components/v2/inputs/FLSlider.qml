import QtQuick
import QtQuick.Controls

// Themed slider. Track/handle from Theme; the control's touch height is floored
// at minTarget
Slider {
    id: control

    // Gamepad and keyboard focus has to be visible; a mouse user already knows
    // where they are, so the ring is suppressed for them
    readonly property bool _focusRing: activeFocus && !InputMethodManager.usingMouse

    // The handle is its own size, not an icon. iconSizeSm only happened to match
    readonly property int _handle: Math.max(AppStyle.minTarget / 2, Math.round(16 * AppStyle.scale))

    implicitHeight: Math.max(AppStyle.minTarget, AppStyle.controlHeight)
    opacity: control.enabled ? 1 : 0.4

    HoverHandler {
        id: sliderHover
        enabled: control.enabled
        cursorShape: Qt.PointingHandCursor
    }

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
        width: control.pressed ? Math.round(control._handle * 1.15) : control._handle
        height: width
        radius: width / 2
        color: Theme.onAccent
        border.width: control._focusRing ? 2 : 1
        border.color: control._focusRing || control.pressed || sliderHover.hovered ? Theme.accent : Theme.border

        Behavior on width {
            NumberAnimation {
                duration: 80
            }
        }
    }
}
