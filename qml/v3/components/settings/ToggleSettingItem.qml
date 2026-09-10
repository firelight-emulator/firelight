// TODO: NEEDS REVIEW
import QtQuick
import QtQuick.Controls

FLMenuItem {
    id: root

    controlItem: FLToggleIndicator {
        enabled: root.enabled
        focusPolicy: Qt.NoFocus
        checked: root.checked
    }

    // TODO
    // Connected rather than handled inline, so a caller's own onClicked does not replace it
    Connections {
        target: root

        function onClicked() {
            if (root.checked) {
                SoundEffects.uncheck.play();
            } else {
                SoundEffects.check.play();
            }
        }
    }
}
