import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import Firelight 1.0

Menu {
    id: control
    property int minWidth: AppStyle.defaultPopupMinimumWidth
    property FLSoundEffect openSound: SoundEffects.openPopup

    padding: AppStyle.spacingMd
    implicitWidth: Math.max(minWidth, contentItem.implicitWidth + padding * 2)
    implicitHeight: contentItem.implicitHeight + padding * 2

    Overlay.modal: Item {}
    property Item caller: null

    // TODO
    // Whether this popup is the one holding a dim up, so the two calls stay paired
    property bool isDimming: false

    function popupFor(caller: Item, desiredX, desiredY) {
        control.caller = caller;

        let finalX = desiredX;
        let finalY = desiredY;

        let windowItem = caller.Window.window.contentItem;
        let mappedPoint = caller.mapToItem(windowItem, desiredX, desiredY);

        // If X is > 0 and Y is 0, we assume the popup wants to be directly to the right of the caller
        if (desiredX > 0 && desiredY === 0) {
            let roomToRight = windowItem.width - mappedPoint.x;

            if (roomToRight < control.width) {
                finalX = -(control.width + AppStyle.spacingSm);
            }
        }

        control.popup(finalX, finalY);
    }

    Connections {
        target: control

        // TODO
        // Opening moves focus inside, and this already sounds for itself
        function onAboutToShow() {
            // TODO
            // Remembered rather than re-tested on the way out, so a popup that stops being modal
            // while open does not take away a dim it never put up
            control.isDimming = control.modal;

            if (control.isDimming) {
                FLDimmer.show(control.caller);
            }

            FocusCursor.startBlink();
        }

        function onOpened() {
            control.contentItem.FLFocus.barrier = true;

            control.contentItem.forceActiveFocus();
            if (control.openSound) {
                control.openSound.play();
            }
            Qt.callLater(() => FocusCursor.endBlink());
        }

        function onAboutToHide() {
            console.log("onAboutToHide in FLPopup");
            FocusCursor.startBlink();
        }

        function onClosed() {
            if (control.isDimming) {
                control.isDimming = false;
                FLDimmer.hide();
            }

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

    // TODO
    // Runs an action and reports that it ran, so each lookup below reads as one line
    function runAction(action: FLAction): bool {
        if (action === null) {
            return false;
        }

        if (action.sound) {
            action.sound.play(false);
        }

        action.triggered();
        return true;
    }

    function activate(key: int): bool {
        const focused = control.contentItem.Window.activeFocusItem;

        if (focused === null) {
            return false;
        }

        const info = control.contentItem.FLFocus.find(focused);

        if (control.runAction(info !== null ? info.getActionFor(key) : null)) {
            return true;
        }

        // TODO
        // This handler is the content item's own, so what the content item declares answers here
        // the way a button's own actions answer in its handler
        if (control.runAction(control.contentItem.FLFocus.getActionFor(key))) {
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
