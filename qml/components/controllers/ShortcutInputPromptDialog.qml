import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import Firelight 1.0

FirelightDialog {
    id: root

    required property GamepadStatus gamepad
    required property var shortcut
    required property string shortcutName

    showButtons: false
    closePolicy: Popup.NoAutoClose

    signal mappingAdded(var shortcut, var modifiers, var input)

    onAboutToShow: {
        frameAnimation.reset()
    }

    onOpened: function () {
        timer.start()
    }

    onClosed: function () {
        timer.stop()
    }

    Connections {
        target: gamepad
        enabled: root.visible
        function onInputChanged(input, activated) {
            if (input === 12 || input === 13) { // GamepadInput enum
                // Ignore Left Trigger and Right Trigger
                return;
            }
            if (activated) {
                console.log("Left Trigger down? " + gamepad.isButtonPressed(12))
                console.log("Right Trigger down? " + gamepad.isButtonPressed(13))

                let modifiers = [];
                if (gamepad.isButtonPressed(12)) {
                    modifiers.push(12);
                }
                if (gamepad.isButtonPressed(13)) {
                    modifiers.push(13);
                }

                console.log("Modifiers: " + modifiers)
                mappingAdded(root.shortcut, modifiers, input);
                root.accept()
                // root.mappingAdded(root.buttons[root.currentIndex].retropad_button, input)
                // if (root.buttons.length > root.currentIndex + 1) {
                //     root.currentIndex++
                //     timer.stop()
                //     frameAnimation.reset()
                //     timer.restart()
                // } else {
                //     root.accept()
                // }
            }
        }
    }

    contentItem: ColumnLayout {
        spacing: 12

        Keys.onPressed: function (event) {
            event.accept = false
        }

        Keys.onReleased: function (event) {
            event.accept = false
        }

        Text {
            text: "Press a button while holding Left Trigger and/or Right Trigger (optional) to assign it to:"
            wrapMode: Text.WordWrap
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: parent.width * 5 / 6

            color: "white"
            font.family: Constants.regularFontFamily
            font.weight: Font.Light
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: 18
        }

        Text {
            text: root.shortcutName
            wrapMode: Text.WordWrap
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            color: ColorPalette.neutral200
            font.pixelSize: 20
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
                root.reject()
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