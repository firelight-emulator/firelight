import QtQuick
import QtQuick.Controls
import Firelight 1.0

// On/off switch. Track and thumb scale with the UI; the touch target is the
// whole control, floored at minTarget.
Switch {
    id: control

    readonly property int _h: Math.max(24, Math.round(24 * AppStyle.scale))
    readonly property int _w: Math.round(_h * 44 / 24)
    readonly property int _thumb: Math.round(_h * 18 / 24)
    readonly property int _inset: Math.round(_h * 3 / 24)

    implicitWidth: _w
    implicitHeight: Math.max(AppStyle.minTarget, _h)

    indicator: Rectangle {
        implicitWidth: control._w
        implicitHeight: control._h
        anchors.verticalCenter: parent.verticalCenter
        radius: height / 2
        color: control.checked ? Theme.accent : Theme.surfaceHover
        border.width: 1
        border.color: control.checked ? Theme.accent : Theme.border
        Behavior on color { ColorAnimation { duration: 120 } }

        Rectangle {
            width: control._thumb
            height: control._thumb
            radius: height / 2
            x: control.checked ? parent.width - width - control._inset : control._inset
            anchors.verticalCenter: parent.verticalCenter
            color: "white"
            Behavior on x { NumberAnimation { duration: 120; easing.type: Easing.InOutQuad } }
        }
    }
}
