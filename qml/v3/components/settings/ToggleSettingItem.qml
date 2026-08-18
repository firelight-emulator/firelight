import QtQuick
import QtQuick.Controls

FLMenuItem {
    id: root

    controlItem: FLToggleIndicator {
        enabled: root.enabled
        focusPolicy: Qt.NoFocus
        checked: root.checked
    }
}
