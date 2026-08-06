import QtQuick

FLPopup {
    id: control

    property alias model: radioGroup.model
    property alias valueRole: radioGroup.valueRole
    property alias currentValue: radioGroup.currentValue

    property bool closeOnActivate: true
    property real closeDelay: InputMethodManager.usingMouse ? 0 : AppStyle.confirmPause

    Connections {
        target: control

        function onAboutToShow() {
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

    contentItem: FLRadioGroup {
        id: radioGroup
        Keys.onPressed: event => {
            event.accepted = control.navigate(event.key, event.isAutoRepeat);
        }

        onActivated: value => {
            control.currentValue = value;
            if (control.closeOnActivate) {
                closeTimer.restart();
            }
        }
    }
}
