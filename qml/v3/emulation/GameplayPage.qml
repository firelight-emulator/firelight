import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

FocusScope {
    id: root

    property bool suspended: true

    function resumeGame() {
        root.suspended = false;
    }

    function startGame() {
        if (emulatorLoader.status !== Loader.Ready) {
            emulatorLoader.setSource("NewEmulatorPage.qml", {});
        }

        emulatorLoader.startGame();
    }

    signal gameplayStarted

    EmulatorLoader {
        id: emulatorLoader
        anchors.fill: parent
        blurAmount: 0
        onSuspended: root.suspended = true
    }

    // Connections {
    //     target: EmulationService
    //
    //     function onGameLoadStarted() {
    //         emulatorLoader.source = "";
    //     }
    //
    //     function onGameLoaded() {
    //         root.gameplayStarted()
    //     }
    //
    //     function onEmulationStopped() {
    //         if (StartupOptions.exitOnClose) {
    //             Qt.quit();
    //             return;
    //         }
    //
    //         emulatorLoader.source = "";
    //     }
    // }
}