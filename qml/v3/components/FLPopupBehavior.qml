import QtQuick
import QtQuick.Templates as T
import Firelight 1.0

/**
 * What a Firelight popup does besides being a popup: takes focus when it opens, answers Back,
 * dispatches the actions its contents declare, and holds the dim while it is up
 */
QtObject {
    id: behavior

    // Using the template since a Menu and a Popup share only that base
    required property T.Popup popup

    property FLSoundEffect openSound: null

    // What the popup was opened from, which is what the dim reads to leave a hole for it
    property Item caller: null

    readonly property Item surface: behavior.popup ? behavior.popup.contentItem : null

    // Whether this popup is the one holding a dim up, so the two calls stay paired
    property bool isDimming: false

    property bool deadHeld: false

    property FLAction backAction: FLAction {
        label: "Back"
        keys: [Qt.Key_Back, Qt.Key_Escape]
        sound: SoundEffects.back
        onTriggered: behavior.popup.close()
    }

    property list<QtObject> wiring: [
        Connections {
            target: behavior.popup

            function onAboutToShow() {
                // TODO
                behavior.isDimming = behavior.popup.modal;

                if (behavior.isDimming) {
                    FLDimmer.show(behavior.caller);
                }

                if (behavior.openSound) {
                    behavior.openSound.play();
                }

                FocusCursor.startBlink();
            }

            function onOpened() {
                behavior.surface.FLFocus.barrier = true;
                behavior.surface.forceActiveFocus();

                Qt.callLater(() => FocusCursor.endBlink());
            }

            function onAboutToHide() {
                FocusCursor.startBlink();
            }

            function onClosed() {
                if (behavior.isDimming) {
                    behavior.isDimming = false;
                    FLDimmer.hide();
                }

                Qt.callLater(() => FocusCursor.endBlink());
            }
        },
        Connections {
            target: behavior.surface ? behavior.surface.Keys : null

            function onPressed(event) {
                if (event.accepted) {
                    return;
                }

                const focused = behavior.surface.Window.activeFocusItem;

                if (!event.isAutoRepeat && behavior.surface.FLFocus.dispatch(focused, event.key, event.modifiers)) {
                    event.accepted = true;
                    return;
                }

                if (FocusNavigator.move(focused, event.key, event.isAutoRepeat) !== FocusNavigator.NoTarget) {
                    event.accepted = true;
                    return;
                }

                if (behavior.deadHeld) {
                    return;
                }

                behavior.deadHeld = FocusCursor.bump(event.key);
            }

            function onReleased(event) {
                if (event.isAutoRepeat) {
                    return;
                }

                if (event.key === Qt.Key_Up || event.key === Qt.Key_Down || event.key === Qt.Key_Left || event.key === Qt.Key_Right) {
                    behavior.deadHeld = false;
                }
            }
        },
        Connections {
            target: FocusCursor.highlight

            function onCursorItemChanged() {
                behavior.deadHeld = false;
            }
        }
    ]

    onSurfaceChanged: {
        if (behavior.surface) {
            behavior.surface.FLFocus.actions = [behavior.backAction];
        }
    }
}
