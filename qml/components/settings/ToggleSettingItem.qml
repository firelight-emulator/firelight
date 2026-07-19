import QtQuick
import QtQuick.Controls

BaseSettingItem {
    id: root
    property alias checked: theControl.checked

    // The row owns focus, so the toggle itself never takes it and its focus ring
    // stays hidden
    control: FLToggle {
        id: theControl

        enabled: root.enabled
        focusPolicy: Qt.NoFocus
    }
}
