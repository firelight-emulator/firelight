import QtQuick
import Firelight 1.0

FLMenuItem {
    id: control
    objectName: "FLToggleMenuItem|" + label

    signal selected(bool selected)

    FLFocus.actions: [
        FLAction {
            keys: [Qt.Key_Space, Qt.Key_Return, Qt.Key_Enter, Qt.Key_Select]
            label: qsTr("Select")
            onTriggered: control.click()
        }
    ]

    onClicked: {
        if (control.checked) {
            SoundEffects.uncheck.play()
        } else {
            SoundEffects.check.play()
        }

        control.selected(!control.checked)
    }

    FLToggleIndicator {
        checked: control.checked
    }
}