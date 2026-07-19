import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

FirelightDialog {
    id: root

    required property GamepadStatus gamepad
    required property var shortcut
    required property string shortcutName
    required property bool isKeyboard
    // Input codes that count as a modifier when held. Comes from the model, so
    // this list and the shipped presets can't drift apart
    property var modifierCandidates: []

    showButtons: false
    closePolicy: Popup.NoAutoClose

    signal mappingAdded(var shortcut, var modifiers, var input)

    // Inputs held right now, in the order they went down
    property var heldInputs: []

    onAboutToShow: {
        frameAnimation.reset();
        heldInputs = [];
    }

    onOpened: function () {
        timer.start();
    }

    onClosed: function () {
        timer.stop();
    }

    Connections {
        target: gamepad
        enabled: root.visible && !root.isKeyboard
        // The combo is read on release, not on press: any input can be a
        // modifier, so while a button is going down there is no way to know
        // whether it is the trigger or something being held for the next one
        // Waiting means "hold Select, press X" records Select+X rather than
        // firing the instant Select goes down
        function onInputChanged(input, activated) {
            if (activated) {
                // They've started entering one; the give-up countdown is for
                // deciding, not for how long a combo may be held
                timer.stop();
                if (!root.heldInputs.includes(input)) {
                    root.heldInputs = root.heldInputs.concat([input]);
                }
                return;
            }

            if (root.heldInputs.length === 0) {
                return;
            }

            // Last one down is the trigger; anything held alongside it modifies
            // it. Sticks are excluded as modifiers — they drift, and a drifting
            // axis would silently join the combo
            const trigger = root.heldInputs[root.heldInputs.length - 1];
            const modifiers = root.heldInputs.slice(0, -1).filter(held => root.modifierCandidates.includes(held));

            root.heldInputs = [];
            mappingAdded(root.shortcut, modifiers, trigger);
            root.accept();
        }
    }

    contentItem: ColumnLayout {
        spacing: 12

        focus: true

        Keys.onPressed: function (event) {
            if (!root.visible || !root.isKeyboard || event.isAutoRepeat) {
                event.accept = false;
                return;
            }

            if (event.key === Qt.Key_Escape || event.key === Qt.Key_Alt || event.key === Qt.Key_Control || event.key === Qt.Key_Shift || event.key === Qt.Key_Meta) {
                // Ignore modifier keys alone, and ignore escape
                return;
            }

            let modifiers = [];
            if (event.modifiers & Qt.ShiftModifier) {
                modifiers.push(Qt.Key_Shift);
            }

            if (event.modifiers & Qt.ControlModifier) {
                modifiers.push(Qt.Key_Control);
            }

            if (event.modifiers & Qt.AltModifier) {
                modifiers.push(Qt.Key_Alt);
            }

            mappingAdded(root.shortcut, modifiers, event.key);
            root.accept();

        // root.mappingAdded(root.buttons[root.currentIndex].retropad_button, event.key)
        }

        Keys.onReleased: function (event) {
            event.accept = false;
        }

        Text {
            text: {
                if (!root.isKeyboard) {
                    "Press a button — hold others first to make a combo, like Select + X. Release to assign it to:";
                } else {
                    "Press a key while holding Shift, Control, Alt, or Windows (optional) to assign it to:";
                }
            }
            wrapMode: Text.WordWrap
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: parent.width * 5 / 6

            color: Theme.textPrimary
            font.family: Constants.regularFontFamily
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: AppStyle.fontSizeMedium
        }

        Text {
            text: root.shortcutName
            wrapMode: Text.WordWrap
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            color: Theme.textPrimary
            font.pixelSize: AppStyle.fontSizeLarge
            font.weight: Font.Bold
            font.family: Constants.regularFontFamily
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        Timer {
            id: timer
            interval: 4000
            running: false
            repeat: false
            onTriggered: {
                root.reject();
            }
        }

        FrameAnimation {
            id: frameAnimation
            running: timer.running
        }

        Rectangle {
            id: theBar
            property var widthThing: parent.width * ((timer.interval - frameAnimation.elapsedTime * 1000) / timer.interval)
            Layout.preferredWidth: widthThing
            Layout.topMargin: 8
            Layout.preferredHeight: 10
            color: "green"
        }
    }
}
