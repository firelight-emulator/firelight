import QtQuick
import QtQuick.Controls
// import QtQuick.Controls.Material
import QtQuick.Dialogs
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Effects
import Firelight 1.0

Loader {
        id: emulatorLoader
        StackView.visible: true

        signal suspended()

        states: [
            State {
                name: "inactive"
                when: emulatorLoader.status != Loader.Ready
                PropertyChanges {
                    emulatorLoader.focus: false
                    emulatorLoader.blurAmount: 0
                }
            },
            State {
                name: "unfocused"
                when: emulatorLoader.status == Loader.Ready && mainContentStack.currentItem != emulatorLoader
                PropertyChanges {
                    emulatorLoader.focus: false
                    emulatorLoader.blurAmount: 1
                }
            },
             State {
                 name: "focused"
                 when: mainContentStack.currentItem == emulatorLoader
                 PropertyChanges {
                     emulatorLoader.focus: true
                     emulatorLoader.blurAmount: 0
                 }
             }
        ]

        transitions: [
            Transition {
                from: "focused"
                to: "unfocused"
                reversible: true
                NumberAnimation {
                    target: emulatorLoader
                    property: "blurAmount"
                    duration: 250
                    easing.type: Easing.InOutQuad
                }
            }
        ]

        function startGame() {
            item.startGame()
        }

        property real blurAmount: 0

        layer.enabled: blurAmount !== 0
        layer.effect: MultiEffect {
            // enabled: root.blurAmount !== 0
            source: emulatorLoader
            anchors.fill: emulatorLoader
            blurEnabled: true
            blurMultiplier: 2
            blurMax: 64
            autoPaddingEnabled: false
            blur: emulatorLoader.blurAmount
        }

        Rectangle {
            id: dimmer
            color: "black"
            visible: emulatorLoader.status === Loader.Ready
            opacity: emulatorLoader.blurAmount * 0.55
            anchors.fill: parent

            z: 10
        }

        Keys.onPressed: function(event) {
            if (event.key === Qt.Key_Home || event.key === Qt.Key_Escape) {
                emulatorLoader.suspended()
                event.accepted = true
                return
            }

            event.accepted = false
        }

        StackView.onActivated: {
            if (emulatorLoader.item) {
                emulatorLoader.item.paused = false
            }
        }

        StackView.onDeactivating: {
            if (emulatorLoader.item) {
                emulatorLoader.item.paused = true
            }
        }
    }