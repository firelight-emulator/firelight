import QtQuick
import QtQuick.Controls
import Firelight 1.0

Popup {
    id: control

    property alias openSound: behavior.openSound
    property alias caller: behavior.caller

    focus: true

    property FLPopupBehavior behavior: FLPopupBehavior {
        id: behavior
        popup: control
    }

    background: FLPopupSurface {}
}
