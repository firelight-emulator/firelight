import QtQuick
import QtQuick.Controls

// On/off switch. Track and thumb scale with the UI; the touch target is the
// whole control, floored at minTarget
Switch {
    id: control

    readonly property int _h: Math.max(24, Math.round(24 * AppStyle.scale))
    readonly property int _w: Math.round(_h * 44 / 24)
    readonly property int _thumb: Math.round(_h * 18 / 24)
    readonly property int _inset: Math.round(_h * 3 / 24)

    // Gamepad and keyboard focus has to be visible; a mouse user already knows
    // where they are, so the ring is suppressed for them
    readonly property bool _focusRing: activeFocus && !InputMethodManager.usingMouse

    implicitWidth: _w
    implicitHeight: Math.max(AppStyle.minTarget, _h)
    opacity: control.enabled ? 1 : 0.4

    HoverHandler {
        id: toggleHover
        enabled: control.enabled
        cursorShape: Qt.PointingHandCursor
    }

    indicator: Rectangle {
        implicitWidth: control._w
        implicitHeight: control._h
        anchors.verticalCenter: parent.verticalCenter
        radius: height / 2
        color: control.checked ? Theme.accent : toggleHover.hovered ? Theme.surfaceElevated : Theme.surfaceHover
        border.width: control._focusRing ? 2 : 1
        border.color: control._focusRing ? Theme.accent : control.checked ? Theme.accent : Theme.border
        Behavior on color {
            ColorAnimation {
                duration: AppStyle.durationFast
            }
        }

        Rectangle {
            width: control._thumb
            height: control._thumb
            radius: height / 2
            x: control.checked ? parent.width - width - control._inset : control._inset
            anchors.verticalCenter: parent.verticalCenter
            color: Theme.onAccent
            Behavior on x {
                NumberAnimation {
                    duration: AppStyle.durationFast
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }
}
