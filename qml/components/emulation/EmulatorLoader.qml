import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import Firelight 1.0

// TODO
// Hosts the emulator page. Pause and blur are owned by the host (GameplayLayer):
// it drives `blurAmount` when something is layered over the game and toggles the
// page's `paused`. This just loads/unloads the page, applies the blur/dim, and
// reports the Esc/Home "suspend" key
Loader {
    id: emulatorLoader

    signal suspended()

    // 0 = clear, 1 = fully blurred+dimmed (game behind the quick menu)
    property real blurAmount: 0

    Behavior on blurAmount {
        NumberAnimation {
            duration: 250
            easing.type: Easing.InOutQuad
        }
    }

    function startGame() {
        if (item) {
            item.startGame()
        }
    }

    layer.enabled: blurAmount !== 0
    layer.effect: MultiEffect {
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

    Keys.onPressed: function (event) {
        if (event.key === Qt.Key_Home || event.key === Qt.Key_Escape) {
            emulatorLoader.suspended()
            event.accepted = true
            return
        }
        event.accepted = false
    }
}
