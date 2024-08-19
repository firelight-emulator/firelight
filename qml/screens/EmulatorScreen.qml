import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Window
import QtQuick.Layouts 1.0
import QtQuick.Effects
import Firelight 1.0

FocusScope {
    id: root

    property bool emulatorIsRunning: emulatorStack.depth > 0

    signal gameReady()

    signal gameAboutToStop()

    signal gameStopped()

    function loadGame(id) {
        emulatorStack.pushItem(emulatorComponent, {entryId: id}, StackView.Immediate)
        // emulator.loadGame(id)
    }

    Rectangle {
        id: background
        color: "black"
        anchors.fill: parent
        visible: root.emulatorIsRunning
    }

    StackView {
        id: emulatorStack
        anchors.fill: parent
        focus: true

        property bool suspended: false
        property bool running: false

        // Keys.onEscapePressed: function (event) {
        //     if (event.isAutoRepeat) {
        //         return
        //     }
        //
        //     if (emulatorStack.depth === 1) {
        //         emulatorStack.pushItem(nowPlayingPage, {}, StackView.PushTransition)
        //     } else if (emulatorStack.depth === 2) {
        //         emulatorStack.pop()
        //     }
        // }

        pushEnter: Transition {
            ParallelAnimation {
                PropertyAnimation {
                    property: "opacity"
                    from: 0
                    to: 1
                    duration: 250
                    easing.type: Easing.InOutQuad
                }
                PropertyAnimation {
                    property: "x"
                    from: -20
                    to: 0
                    duration: 250
                    easing.type: Easing.InOutQuad
                }
            }
        }
        pushExit: Transition {

        }
        popEnter: Transition {
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
                    property: "x"
                    from: 0
                    to: -20
                    duration: 250
                    easing.type: Easing.InOutQuad
                }
            }
        }
        replaceEnter: Transition {
        }
        replaceExit: Transition {
        }
    }

    Component {
        id: emulatorComponent

        FocusScope {
            id: scope
            required property int entryId
            state: "playing"

            StackView.visible: true
            property bool rewinding: false

            // StackView.onActivating: {
            //     emulator.blurred = false
            // }
            //
            // StackView.onDeactivating: {
            //     scope.rewinding = false
            // }

            // StackView.onActivating: {
            //     scope.rewinding = false
            // }

            Keys.onSpacePressed: function () {
                console.log(">:(")
                scope.rewinding = !scope.rewinding

                // if (scope.StackView.status === StackView.Active) {
                //     state = state === "active" ? "inactive" : "active"
                //     // emulator.width = emulator.width / 2
                //     // emulator.height = emulator.height / 2
                // }
            }

            Keys.onEscapePressed: function (event) {
                // if (event.isAutoRepeat || !emulator.loaded) {
                //     return
                // }

                emulatorStack.pushItem(nowPlayingPage, {}, StackView.PushTransition)

                // state = "suspended"

                // if (") {
                //     state = "playing"
                // } else {
                //     state = "rewinding"
                // }

                if (scope.StackView.status === StackView.Active) {

                    // scope.state = "inactive"
                }
            }

            onStateChanged: function () {
                console.log("State changed to", state)
            }

            states: [
                State {
                    name: "rewinding"
                    when: scope.StackView.status === StackView.Active && scope.rewinding
                    PropertyChanges {
                        target: emulator
                        width: scope.width * 3 / 4
                        height: scope.height * 3 / 4
                        blurAmount: 0
                    }
                },
                State {
                    name: "playing"
                    when: scope.StackView.status === StackView.Active && !scope.rewinding
                    PropertyChanges {
                        target: emulator
                        width: scope.width
                        height: scope.height
                        blurAmount: 0
                    }
                },
                State {
                    name: "suspended"
                    when: scope.StackView.status !== StackView.Active
                    PropertyChanges {
                        target: emulator
                        width: scope.width
                        height: scope.height
                        blurAmount: 1
                    }
                }
            ]

            transitions: [
                Transition {
                    from: "playing"
                    to: "rewinding"
                    SequentialAnimation {
                        PropertyAction {
                            target: emulator
                            property: "paused"
                            value: true
                        }
                        ScriptAction {
                            script: {
                                emulator.grabToImage(function (image) {
                                    rewindList.imageSource = image.url
                                })
                            }
                        }
                        PropertyAction {
                            target: rewindList
                            property: "visible"
                            value: true
                        }
                        ParallelAnimation {
                            NumberAnimation {
                                target: emulator
                                properties: "width, height"
                                duration: 220
                                easing.type: Easing.InOutQuad
                            }
                        }
                    }

                },

                Transition {
                    from: "rewinding"
                    to: "playing"
                    SequentialAnimation {
                        ParallelAnimation {
                            PropertyAction {
                                target: rewindList
                                property: "visible"
                                value: false
                            }
                            NumberAnimation {
                                target: emulator
                                properties: "width, height"
                                duration: 220
                                easing.type: Easing.InOutQuad
                            }
                        }
                        PropertyAction {
                            target: emulator
                            property: "paused"
                            value: false
                        }
                    }
                },
                Transition {
                    from: "playing"
                    to: "suspended"
                    SequentialAnimation {
                        PropertyAction {
                            target: emulator
                            properties: "paused"
                            value: true
                        }

                        ParallelAnimation {
                            NumberAnimation {
                                target: emulator
                                properties: "blurAmount"
                                duration: 250
                                easing.type: Easing.InOutQuad
                            }
                            NumberAnimation {
                                target: emulatorDimmer
                                properties: "opacity"
                                duration: 250
                                easing.type: Easing.InOutQuad
                            }
                        }
                    }
                },
                Transition {
                    from: "suspended"
                    to: "playing"
                    SequentialAnimation {
                        ParallelAnimation {
                            NumberAnimation {
                                target: emulator
                                properties: "blurAmount"
                                duration: 250
                                easing.type: Easing.InOutQuad
                            }
                            NumberAnimation {
                                target: emulatorDimmer
                                properties: "opacity"
                                duration: 250
                                easing.type: Easing.InOutQuad
                            }
                        }
                        PropertyAction {
                            target: emulator
                            properties: "paused"
                            value: false
                        }
                    }
                },
                Transition {
                    from: "rewinding"
                    to: "suspended"
                    ParallelAnimation {
                        PropertyAction {
                            target: rewindList
                            property: "visible"
                            value: false
                        }
                        NumberAnimation {
                            target: emulator
                            properties: "width, height"
                            duration: 220
                            easing.type: Easing.InOutQuad
                        }
                        NumberAnimation {
                            target: emulator
                            properties: "blurAmount"
                            duration: 250
                            easing.type: Easing.InOutQuad
                        }
                        NumberAnimation {
                            target: emulatorDimmer
                            properties: "opacity"
                            duration: 250
                            easing.type: Easing.InOutQuad
                        }
                    }
                },
                Transition {
                    from: "suspended"
                    to: "rewinding"
                    SequentialAnimation {

                        ScriptAction {
                            script: {
                                emulator.grabToImage(function (image) {
                                    rewindList.imageSource = image.url
                                })
                            }
                        }
                        ParallelAnimation {
                            PropertyAction {
                                target: rewindList
                                property: "visible"
                                value: true
                            }
                            NumberAnimation {
                                target: emulator
                                properties: "width, height"
                                duration: 220
                                easing.type: Easing.InOutQuad
                            }
                            NumberAnimation {
                                target: emulator
                                properties: "blurAmount"
                                duration: 250
                                easing.type: Easing.InOutQuad
                            }
                            NumberAnimation {
                                target: emulatorDimmer
                                properties: "opacity"
                                duration: 250
                                easing.type: Easing.InOutQuad
                            }
                        }
                    }
                }
            ]

            Popup {
                id: rewindList
                background: Rectangle {
                    color: "red"
                }
                closePolicy: Popup.NoAutoClose
                width: scope.width
                height: 140
                // focus: state === "active"
                y: scope.height
                padding: 20
                property string imageSource: ""

                Keys.onSpacePressed: function () {
                    console.log(":)")
                    scope.rewinding = false
                }

                enter: Transition {
                    NumberAnimation {
                        properties: "y"
                        from: scope.height
                        to: scope.height - rewindList.height
                        duration: 220
                        easing.type: Easing.InOutQuad
                    }
                }

                exit: Transition {
                    PropertyAction {
                        target: scope
                        property: "rewinding"
                        value: false
                    }
                    NumberAnimation {
                        properties: "y"
                        from: scope.height - rewindList.height
                        to: scope.height
                        duration: 220
                        easing.type: Easing.InOutQuad
                    }
                }

                contentItem: ListView {
                    id: rewindListView
                    orientation: ListView.Horizontal
                    layoutDirection: Qt.RightToLeft
                    highlightMoveDuration: 80
                    highlightMoveVelocity: -1
                    keyNavigationEnabled: true
                    highlightRangeMode: ListView.StrictlyEnforceRange
                    preferredHighlightBegin: width / 2 - 100
                    preferredHighlightEnd: width / 2 + 100
                    currentIndex: 0
                    highlight: Item {
                    }
                    onCurrentIndexChanged: {
                        console.log("Current index changed to", currentIndex)
                    }
                    spacing: 8
                    model: 11
                    delegate: Image {
                        Rectangle {
                            visible: parent.ListView.isCurrentItem
                            z: -1
                            width: parent.width + 8
                            height: parent.height + 8
                            x: -4
                            y: -4
                            color: "white"
                        }

                        mipmap: true
                        smooth: false
                        source: rewindList.imageSource
                        // Make bigger if current index is this one
                        anchors.verticalCenter: ListView.view.contentItem.verticalCenter
                        // y: ListView.view.height / 2 - height / 2
                        height: ListView.isCurrentItem ? ListView.view.height + 20 : ListView.view.height
                        Behavior on height {
                            NumberAnimation {
                                duration: 100
                                easing.type: Easing.InOutQuad
                            }
                        }
                        Behavior on width {
                            NumberAnimation {
                                duration: 100
                                easing.type: Easing.InOutQuad
                            }
                        }
                        fillMode: Image.PreserveAspectFit

                        // MultiEffect {
                        //     anchors.fill: parent
                        //     shadowEnabled: true
                        // }
                    }
                }
            }

            EmulatorPage {
                id: emulator
                width: parent.width
                height: parent.height
                anchors.horizontalCenter: parent.horizontalCenter

                // onBlurredChanged: {
                //     console.log("Blurred changed: " + blurred)
                // }

                property bool loaded: false
                property bool blurred: false

                property int entryId: scope.entryId

                Component.onCompleted: {
                    emulator.loadGame(entryId)
                }

                onEmulationStarted: function () {
                    emulator.loaded = true
                }

                // onStateChanged: {
                //     console.log("State changed to", state)
                // }

                ChallengeIndicatorList {
                    id: challengeIndicators
                    visible: achievement_manager.challengeIndicatorsEnabled

                    anchors.top: parent.top
                    anchors.right: parent.right
                    anchors.topMargin: 16
                    anchors.rightMargin: 16
                    height: 100
                    width: 300
                }

                // states: [
                //     State {
                //         name: "notSuspended"
                //         when: !emulator.blurred
                //     },
                //     State {
                //         name: "suspended"
                //         when: emulator.blurred
                //         PropertyChanges {
                //             target: emulatorDimmer
                //             opacity: 0.4
                //         }
                //         PropertyChanges {
                //             emulator {
                //                 // layer.enabled: true
                //                 blurAmount: 1
                //             }
                //         }
                //
                //     }
                //     // State {
                //     //     name: "running"
                //     //     PropertyChanges {
                //     //         target: emulatorDimmer
                //     //         opacity: 0
                //     //     }
                //     //     PropertyChanges {
                //     //         emulator {
                //     //             // layer.enabled: false
                //     //             blurAmount: 0
                //     //         }
                //     //     }
                //     // }
                // ]

                // transitions: [
                //     Transition {
                //         from: "*"
                //         to: "suspended"
                //         SequentialAnimation {
                //             PropertyAction {
                //                 target: emulator
                //                 property: "paused"
                //                 value: true
                //             }
                //             // PropertyAction {
                //             //     target: emulator
                //             //     property: "layer.enabled"
                //             //     value: true
                //             // }
                //             ParallelAnimation {
                //                 NumberAnimation {
                //                     properties: "blurAmount"
                //                     duration: 250
                //                     easing.type: Easing.InOutQuad
                //                 }
                //                 NumberAnimation {
                //                     target: emulatorDimmer
                //                     properties: "opacity"
                //                     duration: 250
                //                     easing.type: Easing.InOutQuad
                //                 }
                //             }
                //         }
                //     },
                //     Transition {
                //         from: "suspended"
                //         to: "*"
                //         SequentialAnimation {
                //             // PropertyAction {
                //             //     target: emulator
                //             //     property: "layer.enabled"
                //             //     value: true
                //             // }
                //             ParallelAnimation {
                //                 NumberAnimation {
                //                     properties: "blurAmount"
                //                     duration: 250
                //                     easing.type: Easing.InOutQuad
                //                 }
                //                 NumberAnimation {
                //                     target: emulatorDimmer
                //                     properties: "opacity"
                //                     duration: 250
                //                     easing.type: Easing.InOutQuad
                //                 }
                //             }
                //             PropertyAction {
                //                 target: emulator
                //                 property: "paused"
                //                 value: false
                //             }
                //         }
                //     }
                // Transition {
                //     from: "*"
                //     to: "running"
                //     SequentialAnimation {
                //         ParallelAnimation {
                //             NumberAnimation {
                //                 properties: "blurAmount"
                //                 duration: 250
                //                 easing.type: Easing.InOutQuad
                //             }
                //             NumberAnimation {
                //                 target: emulatorDimmer
                //                 properties: "opacity"
                //                 duration: 250
                //                 easing.type: Easing.InOutQuad
                //             }
                //         }
                //         // PropertyAction {
                //         //     target: emulator
                //         //     property: "layer.enabled"
                //         //     value: false
                //         // }
                //
                //         ScriptAction {
                //             script: {
                //                 emulator.resumeGame()
                //             }
                //
                //         }
                //     }
                // }
                // ]

                property double blurAmount: 0

                // Behavior on blurAmount {
                //     NumberAnimation {
                //         duration: 250
                //         easing.type: Easing.InOutQuad
                //     }
                // }

                layer.enabled: true
                layer.effect: MultiEffect {
                    source: emulator
                    anchors.fill: emulator
                    blurEnabled: true
                    blurMultiplier: 1.0
                    blurMax: 64
                    blur: emulator.blurAmount
                }

                Rectangle {
                    id: emulatorDimmer
                    anchors.fill: parent
                    color: "black"
                    opacity: 0

                    Behavior on opacity {
                        NumberAnimation {
                            duration: 250
                            easing.type: Easing.InOutQuad
                        }
                    }
                }

                Connections {
                    target: window_resize_handler

                    function onWindowResizeStarted() {
                        if (emulator.StackView.status === StackView.Active) {
                            emulator.pauseGame()
                        }
                    }

                    function onWindowResizeFinished() {
                        if (emulator.StackView.status === StackView.Active) {
                            emulator.resumeGame()
                        }
                    }
                }
            }
        }


    }

    SequentialAnimation {
        id: closeGameAnimation

        ScriptAction {
            script: {
                root.gameAboutToStop()
            }
        }

        PauseAnimation {
            duration: 500
        }

        ScriptAction {
            script: {
                emulatorStack.clear()
            }
        }

        ScriptAction {
            script: {
                root.gameStopped()
            }
        }


    }


    Component {
        id: nowPlayingPage
        NowPlayingPage {
            id: me
            property bool topLevel: true
            property string topLevelName: "nowPlaying"

            Keys.onEscapePressed: function (event) {
                if (event.isAutoRepeat) {
                    return
                }

                if (me.StackView.status === StackView.Active) {
                    // emulatorStack.pop()
                    me.StackView.view.popCurrentItem()
                }
            }

            onBackToMainMenuPressed: function () {
                // root.StackView.view.popCurrentItem()
                // stackView.push(homeScreen)
            }

            onResumeGamePressed: function () {
                emulatorStack.pop()
            }

            onRestartGamePressed: function () {
                const emu = emulatorStack.get(0)
                emu.resetGame()
                // emulator.resetGame()
                emulatorStack.pop()
            }

            onRewindPressed: function () {
                const emu = emulatorStack.get(0)
                emu.rewinding = true
                // emulator.resetGame()
                emulatorStack.pop()
            }

            onCloseGamePressed: function () {
                closeGameAnimation.running = true
                // emulatorStack.popCurrentItem()
                // closeGameAnimation.start()
            }
        }
    }

    AchievementProgressIndicator {
        id: achievementProgressIndicator

        Connections {
            target: achievement_manager

            function onAchievementProgressUpdated(imageUrl, id, name, description, current, desired) {
                if (achievement_manager.progressNotificationsEnabled) {
                    achievementProgressIndicator.openWith(imageUrl, name, description, current, desired)
                }
            }
        }
    }

    AchievementUnlockIndicator {
        id: achievementUnlockIndicator

        Connections {
            target: achievement_manager

            function onAchievementUnlocked(imageUrl, name, description) {
                if (achievement_manager.unlockNotificationsEnabled) {
                    achievementUnlockIndicator.openWith(imageUrl, name, description)
                }
            }
        }
    }
}