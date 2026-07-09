import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

// Free-text setting. Emits `edited` on commit (focus loss / Enter) rather than
// on every keystroke; stays in sync with external value changes when not being
// edited (so a Reset refreshes it).
BaseSettingItem {
    id: root

    property string value: ""
    property string placeholder: ""
    signal edited(string newValue)

    control: TextField {
        id: field
        implicitWidth: 260
        enabled: root.enabled
        placeholderText: root.placeholder
        color: Theme.textPrimary
        font.family: Constants.regularFontFamily
        font.pixelSize: 15
        focusPolicy: Qt.ClickFocus

        background: Rectangle {
            radius: 4
            color: Theme.surface
            border.width: field.activeFocus ? 2 : 0
            border.color: Theme.border
        }

        Component.onCompleted: text = root.value
        onEditingFinished: root.edited(text)

        Connections {
            target: root
            function onValueChanged() {
                if (!field.activeFocus) {
                    field.text = root.value;
                }
            }
        }
    }
}
