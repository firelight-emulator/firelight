import QtQuick
import QtQuick.Controls
import Firelight 1.0

Slider {
    id: control

    // Gamepad and keyboard focus has to be visible; a mouse user already knows
    // where they are, so the ring is suppressed for them
    readonly property bool _focusRing: activeFocus && !InputMethodManager.usingMouse

    readonly property int _handle: Math.max(AppStyle.minTarget / 2, Math.round(16 * AppStyle.scale))

    implicitWidth: AppStyle.inputWidth
    implicitHeight: Math.max(AppStyle.minTarget, AppStyle.controlHeight)
    opacity: control.enabled ? 1 : 0.4

    Keys.onPressed: event => {
        if (event.key === Qt.Key_Left || event.key === Qt.Key_Right) {
            console.log("Key pressed: " + event.key);
            event.accepted = true;
        }
    }

    FLFocus.proxy: theHandle

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
        color: Theme.backgroundInset

        Rectangle {
            width: control.visualPosition * parent.width
            height: parent.height
            radius: height / 2
            color: Theme.accent
        }
    }

    handle: Rectangle {
        id: theHandle
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
