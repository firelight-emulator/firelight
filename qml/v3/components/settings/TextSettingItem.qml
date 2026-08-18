import QtQuick

// Free-text setting. Emits `edited` on commit (focus loss / Enter) rather than
// on every keystroke; stays in sync with external value changes when not being
// edited (so a Reset refreshes it)
FLMenuItem {
    id: root

    controlBelow: true

    property string value: ""
    property string placeholder: ""
    signal edited(string newValue)

    controlItem: FLTextField {
        id: field
        implicitWidth: Math.round(260 * AppStyle.scale)
        enabled: root.enabled
        placeholderText: root.placeholder
        focusPolicy: Qt.ClickFocus

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
