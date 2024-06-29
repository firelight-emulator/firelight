import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Window
import QtQuick.Layouts 1.0
import QtQuick.Effects
import Firelight 1.0

ApplicationWindow {
    id: window

    width: 1280
    height: 720

    // flags: Qt.FramelessWindowHint

    visible: true
    visibility: GeneralSettings.fullscreen ? Window.FullScreen : Window.Windowed

    title: qsTr("Firelight")

    background: Rectangle {
        color: "#1a1b1e"
    }

    Connections {
        target: Router

        function onRouteChanged(route) {
            let parts = route.split("/")
            if (parts.length > 0) {
                let id = parts[0]

                if (id === "settings") {
                    let section = parts.length > 1 ? parts[1] : "library"
                    stackView.push(settingsScreen, {section: section})
                }
            }
        }
    }

    Component {
        id: homeScreen

        HomeScreen {
            showNowPlayingButton: emulatorScreen.emulatorIsRunning
        }
    }

    Component {
        id: settingsScreen
        SettingsScreen {
            property bool topLevel: true
            property string topLevelName: "settings"

            Keys.onEscapePressed: function (event) {
                if (!event.isAutoRepeat) {
                    StackView.view.pop()
                }
            }
        }
    }

    EmulatorScreen {
        id: emulatorScreen
        onGameReady: function () {
            overlayFadeIn.start()
        }
    }

    onActiveFocusControlChanged: function () {
        let str = ""
        for (let key in window.activeFocusControl) {
            str += key + ": " + window.activeFocusControl[key] + "\n"
        }

        debugText.text = str
    }

    Rectangle {
        color: "transparent"
        border.color: "red"
        anchors.fill: parent
        parent: window.activeFocusControl
    }

    Window {
        id: debugWindow
        width: 400
        height: 400
        visible: false

        ScrollView {
            anchors.fill: parent
            contentHeight: debugText.height
            contentWidth: debugText.width
            Text {
                id: debugText
                text: ""
            }
        }
    }

    GameLaunchPopup {
        id: gameLaunchPopup

        Connections {
            target: achievement_manager

            function onGameLoadSucceeded(imageUrl, title, numEarned, numTotal) {
                gameLaunchPopup.openWith(imageUrl, title, numEarned, numTotal, achievement_manager.defaultToHardcore)
            }
        }
    }

    // AchievementProgressIndicator {
    //     id: achievementProgressIndicator
    //
    //     Connections {
    //         target: achievement_manager
    //
    //         function onAchievementProgressUpdated(imageUrl, id, name, description, current, desired) {
    //             if (achievement_manager.progressNotificationsEnabled) {
    //                 achievementProgressIndicator.openWith(imageUrl, name, description, current, desired)
    //             }
    //         }
    //     }
    // }
    //
    // AchievementUnlockIndicator {
    //     id: achievementUnlockIndicator
    //
    //     Connections {
    //         target: achievement_manager
    //
    //         function onAchievementUnlocked(imageUrl, name, description) {
    //             if (achievement_manager.unlockNotificationsEnabled) {
    //                 achievementUnlockIndicator.openWith(imageUrl, name, description)
    //             }
    //         }
    //     }
    // }

    Component {
        id: libraryPage
        LibraryPage {
            property bool topLevel: true
            property string topLevelName: "library"

            onEntryClicked: function (id) {
                emulatorScreen.loadGame(id)
            }
        }
    }

    // Component {
    //     id: nowPlayingPage
    //     NowPlayingPage {
    //         id: me
    //         property bool topLevel: true
    //         property string topLevelName: "nowPlaying"
    //
    //         onBackToMainMenuPressed: function () {
    //             stackView.push(homeScreen)
    //         }
    //
    //         onResumeGamePressed: function () {
    //             emulatorStack.pop()
    //         }
    //
    //         onRestartGamePressed: function () {
    //             emulator.resetGame()
    //             emulatorStack.pop()
    //         }
    //
    //         onCloseGamePressed: function () {
    //             closeGameAnimation.start()
    //         }
    //     }
    // }

    SequentialAnimation {
        id: closeGameAnimation
        ScriptAction {
            script: {
                stackView.push(homeScreen)
            }
        }
        ScriptAction {
            script: {
                emulatorScreen.stopEmulator()
            }
        }
        // ScriptAction {
        //     script: {
        //         emulatorStack.pop()
        //     }
        // }
    }

    SequentialAnimation {
        id: overlayFadeIn
        PropertyAction {
            target: overlay
            property: "opacity"
            value: 0
        }
        PropertyAction {
            target: overlay
            property: "scale"
            value: 1
        }

        PropertyAction {
            target: overlay
            property: "visible"
            value: true
        }

        PropertyAnimation {
            target: overlay
            property: "opacity"
            from: 0
            to: 1
            duration: 350
            easing.type: Easing.InQuad
        }

        ScriptAction {
            script: {
                stackView.pop(StackView.Immediate)
                // emulator.resumeGame()
            }
        }

        PropertyAnimation {
            target: overlay
            property: "opacity"
            from: 1
            to: 0
            duration: 200
            easing.type: Easing.InQuad
        }

        ScriptAction {
            script: {
                // gameLaunchPopup.open()
                emulatorScreen.startEmulator()
                // emulator.resumeGame()
            }
        }
    }

    // EmulatorPage {
    //     id: emulator
    //     visible: false
    //
    //     onReadyToStart: function () {
    //         overlayFadeIn.start()
    //     }
    //
    //     ChallengeIndicatorList {
    //         id: challengeIndicators
    //         visible: achievement_manager.challengeIndicatorsEnabled
    //
    //         anchors.top: parent.top
    //         anchors.right: parent.right
    //         anchors.topMargin: 16
    //         anchors.rightMargin: 16
    //         height: 100
    //         width: 300
    //     }
    //
    //     states: [
    //         State {
    //             name: "stopped"
    //         },
    //         State {
    //             name: "suspended"
    //             PropertyChanges {
    //                 target: emulatorDimmer
    //                 opacity: 0.4
    //             }
    //             PropertyChanges {
    //                 emulator {
    //                     layer.enabled: true
    //                     blurAmount: 1
    //                 }
    //             }
    //         },
    //         State {
    //             name: "running"
    //             PropertyChanges {
    //                 target: emulatorDimmer
    //                 opacity: 0
    //             }
    //             PropertyChanges {
    //                 emulator {
    //                     layer.enabled: false
    //                     blurAmount: 0
    //                 }
    //             }
    //         }
    //     ]
    //
    //     transitions: [
    //         Transition {
    //             from: "*"
    //             to: "suspended"
    //             SequentialAnimation {
    //                 ScriptAction {
    //                     script: {
    //                         emulator.pauseGame()
    //                     }
    //                 }
    //                 PropertyAction {
    //                     target: emulator
    //                     property: "layer.enabled"
    //                     value: true
    //                 }
    //                 ParallelAnimation {
    //                     NumberAnimation {
    //                         properties: "blurAmount"
    //                         duration: 250
    //                         easing.type: Easing.InOutQuad
    //                     }
    //                     NumberAnimation {
    //                         target: emulatorDimmer
    //                         properties: "opacity"
    //                         duration: 250
    //                         easing.type: Easing.InOutQuad
    //                     }
    //                 }
    //             }
    //         },
    //         Transition {
    //             from: "*"
    //             to: "running"
    //             SequentialAnimation {
    //                 ParallelAnimation {
    //                     NumberAnimation {
    //                         properties: "blurAmount"
    //                         duration: 250
    //                         easing.type: Easing.InOutQuad
    //                     }
    //                     NumberAnimation {
    //                         target: emulatorDimmer
    //                         properties: "opacity"
    //                         duration: 250
    //                         easing.type: Easing.InOutQuad
    //                     }
    //                 }
    //                 PropertyAction {
    //                     target: emulator
    //                     property: "layer.enabled"
    //                     value: false
    //                 }
    //
    //                 ScriptAction {
    //                     script: {
    //                         emulator.resumeGame()
    //                     }
    //
    //                 }
    //             }
    //         }
    //     ]
    //
    //     StackView.visible: true
    //
    //     StackView.onActivating: {
    //         state = "running"
    //     }
    //
    //     StackView.onDeactivating: {
    //         state = "suspended"
    //     }
    //
    //     property double blurAmount: 0
    //
    //     Behavior on blurAmount {
    //         NumberAnimation {
    //             duration: 250
    //             easing.type: Easing.InOutQuad
    //         }
    //     }
    //
    //     layer.enabled: false
    //     layer.effect: MultiEffect {
    //         source: emulator
    //         anchors.fill: emulator
    //         blurEnabled: true
    //         blurMultiplier: 1.0
    //         blurMax: 64
    //         blur: emulator.blurAmount
    //     }
    //
    //     Rectangle {
    //         id: emulatorDimmer
    //         anchors.fill: parent
    //         color: "black"
    //         opacity: 0
    //
    //         Behavior on opacity {
    //             NumberAnimation {
    //                 duration: 250
    //                 easing.type: Easing.InOutQuad
    //             }
    //         }
    //     }
    //
    //     Connections {
    //         target: window_resize_handler
    //
    //         function onWindowResizeStarted() {
    //             if (emulator.StackView.view.currentItem === emulator) {
    //                 emulator.pauseGame()
    //             }
    //         }
    //
    //         function onWindowResizeFinished() {
    //             if (emulator.StackView.view.currentItem === emulator) {
    //                 emulator.resumeGame()
    //             }
    //         }
    //     }
    // }

    // StackView {
    //     id: emulatorStack
    //     visible: false
    //
    //     initialItem: emulator
    //
    //     Keys.onEscapePressed: function (event) {
    //         if (event.isAutoRepeat) {
    //             return
    //         }
    //
    //         if (emulatorStack.currentItem === emulator) {
    //             // emulatorStack.pop()
    //             emulatorStack.push(nowPlayingPage)
    //         } else {
    //             emulatorStack.pop()
    //             // emulatorStack.push(homeScreen)
    //         }
    //     }
    //
    //     property bool suspended: false
    //     property bool running: false
    //
    //     pushEnter: Transition {
    //         ParallelAnimation {
    //             PropertyAnimation {
    //                 property: "opacity"
    //                 from: 0
    //                 to: 1
    //                 duration: 250
    //                 easing.type: Easing.InOutQuad
    //             }
    //             PropertyAnimation {
    //                 property: "x"
    //                 from: -20
    //                 to: 0
    //                 duration: 250
    //                 easing.type: Easing.InOutQuad
    //             }
    //         }
    //     }
    //     pushExit: Transition {
    //
    //     }
    //     popEnter: Transition {
    //     }
    //     popExit: Transition {
    //         ParallelAnimation {
    //             PropertyAnimation {
    //                 property: "opacity"
    //                 from: 1
    //                 to: 0
    //                 duration: 250
    //                 easing.type: Easing.InOutQuad
    //             }
    //             PropertyAnimation {
    //                 property: "x"
    //                 from: 0
    //                 to: -20
    //                 duration: 250
    //                 easing.type: Easing.InOutQuad
    //             }
    //         }
    //     }
    //     replaceEnter: Transition {
    //     }
    //     replaceExit: Transition {
    //     }
    // }

    // Rectangle {
    //     id: bar
    //     color: "red"
    //     anchors.top: parent.top
    //     anchors.left: parent.left
    //     anchors.right: parent.right
    //     height: 40
    //
    //     DragHandler {
    //         grabPermissions: TapHandler.CanTakeOverFromAnything
    //         onActiveChanged: if (active) {
    //             window.startSystemMove();
    //         }
    //     }
    //
    // }

    StackView {
        id: stackView
        anchors.fill: parent
        focus: true

        initialItem: emulatorScreen

        Component.onCompleted: stackView.push(homeScreen, {}, StackView.Immediate)

        Keys.onReleased: function (event) {
            if (event.key === Qt.Key_F12) {
                debugWindow.visible = !debugWindow.visible
                event.accept()
            }
        }

        pushEnter: Transition {
            ParallelAnimation {
                PropertyAnimation {
                    property: "scale"
                    from: 1.05
                    to: 1
                    duration: 250
                    easing.type: Easing.InOutQuad
                }
                PropertyAnimation {
                    property: "opacity"
                    from: 0
                    to: 1
                    duration: 250
                    easing.type: Easing.InOutQuad
                }
            }
        }
        pushExit: Transition {
            ParallelAnimation {
                PropertyAnimation {
                    property: "opacity"
                    from: 1
                    to: 0
                    duration: 250
                    easing.type: Easing.InOutQuad
                }
                PropertyAnimation {
                    property: "scale"
                    from: 1
                    to: 0.95
                    duration: 250
                    easing.type: Easing.OutQuad
                }
            }
        }
        popEnter: Transition {
            ParallelAnimation {
                PropertyAnimation {
                    property: "scale"
                    from: 0.95
                    to: 1
                    duration: 250
                    easing.type: Easing.InQuad
                }
                PropertyAnimation {
                    property: "opacity"
                    from: 0
                    to: 1
                    duration: 250
                    easing.type: Easing.InOutQuad
                }
            }
        }
        popExit: Transition {
            ParallelAnimation {
                PropertyAnimation {
                    property: "opacity"
                    from: 1
                    to: 0
                    duration: 250
                    easing.type: Easing.InOutQuad
                }
                PropertyAnimation {
                    property: "scale"
                    from: 1
                    to: 1.05
                    duration: 250
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }

    Rectangle {
        id: overlay
        anchors.fill: parent
        color: "black"
        visible: false
    }
}
