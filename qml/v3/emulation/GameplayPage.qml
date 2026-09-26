import QtQuick
import QtQuick.Controls
import Firelight 1.0

FocusScope {
    id: root

    objectName: "GameplayPage"

    // From gameLoadStarted until the cinematic reveals the game or the load fails
    property bool launching: false

    readonly property bool playing: EmulationService.isGameRunning && !EmulationService.suspended && !root.launching

    readonly property bool shown: root.launching || root.playing || (EmulationService.isGameRunning && Router.path === "/quick-menu")

    property bool _isBlackFull: false
    property bool _isGameReady: false

    signal readyToReveal

    opacity: root.shown ? 1 : 0
    visible: root.opacity > 0
    enabled: root.shown

    FLFocus.surface: true
    FLFocus.layer: 10
    FLFocus.surfaceActive: root.playing
    FLFocus.entry: emulatorLoader.item ? emulatorLoader.item.emulatorItem : emulatorLoader

    Behavior on opacity {
        enabled: !root.launching

        NumberAnimation {
            duration: AppStyle.durationBase
            easing.type: AppStyle.easingStandard
        }
    }

    // TODO: why
    Keys.onPressed: event => {
        if (root.playing && (event.key === Qt.Key_Tab || event.key === Qt.Key_Backtab)) {
            event.accepted = true;
        }
    }
    function openQuickMenu() {
        if (root.playing) {
            EmulationService.suspend();
        }
    }

    function resumeGame() {
        EmulationService.resume();
    }

    function markBlackFull() {
        if (!root.launching) {
            return;
        }

        if (Router.path !== "/quick-menu") {
            Router.navigate("/quick-menu");
        }

        root._isBlackFull = true;
        root._maybeReveal();
    }

    function _maybeReveal() {
        if (!root._isBlackFull || !root._isGameReady) {
            return;
        }

        if (emulatorLoader.status !== Loader.Ready) {
            emulatorLoader.setSource("NewEmulatorPage.qml", {});
        }

        emulatorLoader.startGame();

        root._isBlackFull = false;
        root._isGameReady = false;

        EmulationService.resume();

        root.launching = false;
        root.readyToReveal();
    }

    function _leaveQuickMenu() {
        if (Router.path !== "/quick-menu") {
            return;
        }

        if (!Router.back()) {
            Router.navigate("/library");
        }
    }

    EmulatorLoader {
        id: emulatorLoader
        anchors.fill: parent
        blurAmount: EmulationService.suspended ? 1 : 0
        onSuspended: root.openQuickMenu()
    }

    Connections {
        target: ShortcutDispatcher

        function onOpenQuickMenuRequested() {
            // TODO
            // root.openQuickMenu();
        }
    }

    Binding {
        target: emulatorLoader.item
        property: "suspended"
        value: EmulationService.suspended || emulatorLoader.blurAmount > 0
        when: emulatorLoader.status === Loader.Ready
    }

    Connections {
        target: EmulationService

        function onGameLoadStarted() {
            root._isBlackFull = false;
            root._isGameReady = false;

            emulatorLoader.source = "";
            root.launching = true;
        }

        function onGameLoaded() {
            root._isGameReady = true;
            root._maybeReveal();
        }

        function onGameLoadFailed(reason) {
            root.launching = false;
            root._isBlackFull = false;
            root._isGameReady = false;
            root._leaveQuickMenu();
        }

        function onEmulationStopped() {
            if (StartupOptions.exitOnClose) {
                Qt.quit();
                return;
            }

            emulatorLoader.source = "";

            if (!root.launching) {
                root._leaveQuickMenu();
            }
        }
    }
}
