import QtQuick

// The running game as a full-window layer above the router (not a route). It is
// bottom-anchored and changes HEIGHT by state, so the game render itself shrinks
// down into a bottom bar when backgrounded (it becomes the "now playing" bar)
// and grows back to full-screen when resumed. Modes:
//   none         - no game loaded (height 0)
//   launching    - full height, fading to black over the menus before the game
//   playing      - full height, game running & focused
//   quickMenu    - full height, game paused + blurred, QuickMenu on top
//   backgrounded - shrunk to the bottom bar, game paused & alive
//
// Launch fades to black on gameLoadStarted and reveals only once BOTH the fade
// finished and the game is ready. "Back to Menu" shrinks it to the bar (kept
// alive, resumable); only "Close Game" tears it down
Item {
    id: gameplay

    anchors.left: parent.left
    anchors.right: parent.right
    anchors.bottom: parent.bottom

    property string mode: "none"
    readonly property bool foregrounded: mode === "playing" || mode === "quickMenu"
    readonly property bool launching: mode === "launching"
    property int barHeight: 72

    // Reveal is gated on both, so we always fully fade to black before the game
    property bool _blackFull: false
    property bool _gameReady: false

    visible: state !== "none" || height > 0
    enabled: mode !== "none"
    clip: true

    // Height is driven by state; only the shrink/grow between full and bar (and
    // the close-away) animate — launch snaps to full and lets the black cover fade
    state: mode === "none" ? "none" : mode === "backgrounded" ? "docked" : "full"

    states: [
        State {
            name: "none"
            PropertyChanges {
                target: gameplay
                height: 0
            }
        },
        State {
            name: "docked"
            PropertyChanges {
                target: gameplay
                height: gameplay.barHeight
            }
        },
        State {
            name: "full"
            PropertyChanges {
                target: gameplay
                height: gameplay.parent.height
            }
        }
    ]

    transitions: [
        Transition {
            to: "docked"
            NumberAnimation {
                property: "height"
                duration: 300
                easing.type: AppStyle.easingStandard
            }
        },
        Transition {
            from: "docked"
            to: "full"
            NumberAnimation {
                property: "height"
                duration: 300
                easing.type: AppStyle.easingStandard
            }
        },
        Transition {
            to: "none"
            NumberAnimation {
                property: "height"
                duration: 250
                easing.type: AppStyle.easingStandard
            }
        }
    ]

    // Focus after the state settles (enabled/visible have updated), so keys —
    // notably Esc → quick menu — reach the emulator when it's foregrounded
    onModeChanged: {
        if (mode === "playing") {
            Qt.callLater(function () {
                emulatorLoader.forceActiveFocus();
            });
        } else if (mode === "quickMenu") {
            Qt.callLater(function () {
                quickMenu.forceActiveFocus();
            });
        }
    }

    function _maybeReveal() {
        if (_blackFull && _gameReady) {
            _blackFull = false;
            _gameReady = false;
            mode = "playing";
        }
    }

    function foreground() {
        mode = "playing";
        if (Router.path === "/quick-menu") {
            Router.back();
        }
    }

    function openQuickMenu() {
        if (mode === "playing") {
            mode = "quickMenu";
            if (Router.path !== "/quick-menu") {
                Router.navigate("/quick-menu");
            }
        }
    }

    // The quick menu is a route as well as a hotkey target, so the address and
    // the menu agree however it was opened. Each direction checks the other's
    // state first, so the round trip settles instead of looping
    Connections {
        target: Router

        function onNavigated() {
            if (Router.path === "/quick-menu") {
                gameplay.openQuickMenu();
            } else if (gameplay.mode === "quickMenu") {
                gameplay.foreground();
            }
        }
    }

    function background() {
        mode = "backgrounded";
    }

    // Dark backdrop so the shrunk game reads as a bar
    Rectangle {
        anchors.fill: parent
        color: Theme.background
    }

    EmulatorLoader {
        id: emulatorLoader
        anchors.fill: parent
        blurAmount: gameplay.mode === "quickMenu" ? 1 : 0
        onSuspended: gameplay.openQuickMenu()
    }

    QuickMenu {
        id: quickMenu
        anchors.fill: parent
        opacity: gameplay.mode === "quickMenu" ? 1 : 0
        visible: opacity > 0
        Behavior on opacity {
            NumberAnimation {
                duration: 160
                easing.type: Easing.InOutQuad
            }
        }

        onResumeGame: gameplay.foreground()
        onResetGame: {
            EmulationService.resetGame();
            gameplay.foreground();
        }
        onRewindPressed: {
            if (emulatorLoader.item) {
                emulatorLoader.item.createRewindPoints();
            }
        }
        onBackToMenu: gameplay.background()
        onCloseGame: EmulationService.stopEmulation()

        Keys.onEscapePressed: gameplay.foreground()
    }

    // Freeze the game unless it's the foregrounded, playing thing
    Binding {
        target: emulatorLoader.item
        property: "paused"
        value: gameplay.mode !== "playing"
        when: emulatorLoader.status === Loader.Ready
    }

    // Docked-bar chrome (only when shrunk): tap anywhere to resume, X to close
    MouseArea {
        anchors.fill: parent
        visible: gameplay.mode === "backgrounded"
        enabled: visible
        hoverEnabled: true
        z: 50
        onClicked: gameplay.foreground()

        FLIconButton {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.rightMargin: AppStyle.spacingMd
            iconName: "cancel"
            onClicked: EmulationService.stopEmulation()
        }
    }

    // Fade-to-black cover for launch. Sits over everything
    Rectangle {
        id: blackCover
        anchors.fill: parent
        color: "black"
        z: 100
        opacity: gameplay.launching ? 1 : 0
        visible: opacity > 0
        Behavior on opacity {
            NumberAnimation {
                duration: 260
                easing.type: Easing.InOutQuad
            }
        }
        onOpacityChanged: {
            if (opacity >= 1.0) {
                gameplay._blackFull = true;
                gameplay._maybeReveal();
            }
        }
    }

    Connections {
        target: EmulationService

        function onGameLoadStarted() {
            gameplay._blackFull = false;
            gameplay._gameReady = false;
            gameplay.mode = "launching";
        }

        function onGameLoaded() {
            if (emulatorLoader.status !== Loader.Ready) {
                emulatorLoader.setSource("NewEmulatorPage.qml", {});
            }
            emulatorLoader.startGame();
            gameplay._gameReady = true;
            gameplay._maybeReveal();
        }

        function onEmulationStopped() {
            // External-launcher mode: the app exists only to run this one game
            if (StartupOptions.exitOnClose) {
                Qt.quit();
                return;
            }
            // Drop the old page. Keep launching mode if this stop is part of a
            // relaunch (loadEntry stops the previous game before loading the new)
            emulatorLoader.source = "";
            if (gameplay.mode !== "launching") {
                gameplay.mode = "none";
            }
        }
    }
}
