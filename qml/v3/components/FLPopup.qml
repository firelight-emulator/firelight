import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import Firelight 1.0

Menu {
    id: control
    property int minWidth: AppStyle.defaultPopupMinimumWidth

    padding: AppStyle.spacingMd
    implicitWidth: Math.max(minWidth, contentItem.implicitWidth + padding * 2)

    Connections {
        target: control

        // TODO
        // Opening moves focus inside, and this already sounds for itself
        function onAboutToShow() {
            FocusCursor.startBlink();
        }

        function onOpened() {
            control.contentItem.FLFocus.barrier = true;

            control.contentItem.forceActiveFocus();
            SoundEffects.openPopup.play();
            Qt.callLater(() => FocusCursor.endBlink());
        }
    }

    property bool deadHeld: false

    Connections {
        target: control.contentItem.Keys

        function onPressed(event) {
            if (!event.isAutoRepeat && control.activate(event.key)) {
                event.accepted = true;
                return;
            }

            const focused = control.contentItem.Window.activeFocusItem;

            if (FocusNavigator.move(focused, event.key, event.isAutoRepeat) !== FocusNavigator.NoTarget) {
                event.accepted = true;
                return;
            }

            if (control.deadHeld) {
                return;
            }

            control.deadHeld = FocusCursor.bump(event.key);
        }

        function onReleased(event) {
            if (event.isAutoRepeat) {
                return;
            }

            if (event.key === Qt.Key_Up || event.key === Qt.Key_Down || event.key === Qt.Key_Left || event.key === Qt.Key_Right) {
                control.deadHeld = false;
            }
        }
    }

    Connections {
        target: FocusCursor.highlight
        function onCursorItemChanged() {
            control.deadHeld = false;
        }
    }

    function activate(key: int): bool {
        const focused = control.contentItem.Window.activeFocusItem;

        if (focused === null) {
            return false;
        }

        const info = control.contentItem.FLFocus.find(focused);
        const action = info !== null ? info.getActionFor(key) : null;

        console.log("action: " + action + ", info: " + info);
        if (action !== null) {
            if (action.sound) {
                action.sound.play(false);
            }

            action.triggered();
            return true;
        }

        if (info !== null && info.getActionCount() > 0) {
            return false;
        }

        if (key !== Qt.Key_Select && key !== Qt.Key_Return && key !== Qt.Key_Enter) {
            return false;
        }

        if (typeof focused.click !== "function") {
            return false;
        }

        focused.click();
        return true;
    }

    Connections {
        target: control

        function onAboutToHide() {
            FocusCursor.startBlink();
        }

        function onClosed() {
            Qt.callLater(() => FocusCursor.endBlink());
        }
    }

    background: Rectangle {
        color: Theme.surface
        radius: AppStyle.radiusMd
        border.color: Theme.border
        border.width: 1

        layer.enabled: true
        layer.effect: MultiEffect {
            shadowEnabled: true
            shadowColor: Theme.shadow
            shadowBlur: AppStyle.elevationBlur
            shadowVerticalOffset: AppStyle.elevationOffset
            shadowHorizontalOffset: AppStyle.elevationOffset
        }
    }
}
