import QtQuick

Item {
    id: gameplay

    anchors.left: parent.left
    anchors.right: parent.right
    anchors.bottom: parent.bottom

    property string mode: "none"
    readonly property bool foregrounded: mode === "playing" || mode === "quickMenu"
    readonly property bool launching: mode === "launching"
    property int barHeight: 72

    property bool _blackFull: false
    property bool _gameReady: false

    visible: state !== "none" || height > 0
    enabled: mode !== "none"
    clip: true

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

    onModeChanged: {
        if (mode === "playing") {
            FocusCursor.endBlink();
            Qt.callLater(function () {
                emulatorLoader.forceActiveFocus();
            });
        } else if (mode === "quickMenu") {
            FocusCursor.endBlink();
            Qt.callLater(function () {
                quickMenu.forceActiveFocus();
            });
        } else if (mode === "launching") {
            FocusCursor.startBlink();
        } else {
            FocusCursor.endBlink();
        }
    }

    Keys.onPressed: event => {
        if (gameplay.foregrounded && (event.key === Qt.Key_Tab || event.key === Qt.Key_Backtab)) {
            event.accepted = true;
        }
    }

    function markBlackFull() {
        _blackFull = true;
        _maybeReveal();
    }

    signal readyToReveal

    function _maybeReveal() {
        if (_blackFull && _gameReady) {
            if (emulatorLoader.status !== Loader.Ready) {
                emulatorLoader.setSource("NewEmulatorPage.qml", {});
            }
            emulatorLoader.startGame();

            _blackFull = false;
            _gameReady = false;
            mode = "playing";
            gameplay.readyToReveal();
        }
    }

    function leaveQuickMenuRoute() {
        if (Router.path === "/quick-menu") {
            Router.back();
        }
    }

    function foreground() {
        mode = "playing";
        gameplay.leaveQuickMenuRoute();
    }

    function openQuickMenu() {
        if (mode === "playing") {
            mode = "quickMenu";
            if (Router.path !== "/quick-menu") {
                Router.navigate("/quick-menu");
            }
        }
    }

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
        gameplay.leaveQuickMenuRoute();
    }

    Rectangle {
        anchors.fill: parent
        color: Theme.background
        visible: !gameplay.launching
    }

    EmulatorLoader {
        id: emulatorLoader
        anchors.fill: parent
        anchors.topMargin: AppStyle.titleBarHeight
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

    Binding {
        target: emulatorLoader.item
        property: "paused"
        value: gameplay.mode !== "playing"
        when: emulatorLoader.status === Loader.Ready
    }

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


    Connections {
        target: EmulationService

        function onGameLoadStarted() {
            gameplay._blackFull = false;
            gameplay._gameReady = false;

            emulatorLoader.source = "";

            gameplay.mode = "launching";
        }

        function onGameLoaded() {
            gameplay._gameReady = true;
            gameplay._maybeReveal();
        }

        function onEmulationStopped() {
            if (StartupOptions.exitOnClose) {
                Qt.quit();
                return;
            }

            emulatorLoader.source = "";
            if (gameplay.mode !== "launching") {
                gameplay.mode = "none";
            }

            gameplay.leaveQuickMenuRoute();
        }
    }
}
