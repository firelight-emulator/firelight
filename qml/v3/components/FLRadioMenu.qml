import QtQuick

FLMenu {
    id: control

    property alias model: radioGroup.model
    property alias valueRole: radioGroup.valueRole
    property alias currentValue: radioGroup.currentValue

    property bool closeOnActivate: true
    property real closeDelay: InputMethodManager.usingMouse ? 0 : AppStyle.confirmPause

    Connections {
        target: control

        function onAboutToShow() {
            radioGroup.focusCurrentIndex();
            closeTimer.stop();
        }

        function onClosed() {
            closeTimer.stop();
        }
    }

    Timer {
        id: closeTimer
        interval: control.closeDelay
        repeat: false
        onTriggered: control.close()
    }

    FLRadioGroup {
        id: radioGroup

        onActivated: value => {
            control.currentValue = value;
            if (control.closeOnActivate) {
                closeTimer.restart();
            }
        }
    }
}
