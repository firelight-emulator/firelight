import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Window
import QtQuick.Layouts 1.0
import QtMultimedia
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

            Keys.onSpacePressed: function () {
                console.log(">:(")
                scope.rewinding = !scope.rewinding
            }

            Keys.onEscapePressed: function (event) {
                // if (event.isAutoRepeat || !emulator.loaded) {
                //     return
                // }

                emulatorStack.pushItem(nowPlayingPage, {}, StackView.PushTransition)

                if (scope.StackView.status === StackView.Active) {
                }
            }

            states: [
                State {
                    name: "rewinding"
                    when: scope.StackView.status === StackView.Active && scope.rewinding
                    PropertyChanges {
                        target: emulator
                        // width: scope.width * 3 / 4
                        // height: scope.height * 3 / 4
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

            Connections {
                target: coverImageBg

                function onHidden() {
                    emulator.paused = false
                }
            }

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
                                    // coverImage.source = image.url
                                    // theModel.insert(0, {imageSource: image.url})
                                    // theList.model.append({imageSource: image.url})
                                    coverImageBg.thing = true
                                })
                            }
                        }
                        // PropertyAction {
                        //     target: rewindList
                        //     property: "visible"
                        //     value: true
                        // }
                        // PropertyAction {
                        //     target: coverImage
                        //     property: "visible"
                        //     value: true
                        // }
                        // ParallelAnimation {
                        //     NumberAnimation {
                        //         target: emulator
                        //         properties: "width, height"
                        //         duration: 220
                        //         easing.type: Easing.InOutQuad
                        //     }
                        // }
                    }

                },

                Transition {
                    from: "rewinding"
                    to: "playing"
                    SequentialAnimation {
                        PropertyAction {
                            target: coverImageBg
                            property: "thing"
                            value: false
                        }
                        // PropertyAction {
                        //     target: emulator
                        //     property: "paused"
                        //     value: false
                        // }
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
                        // PropertyAction {
                        //     target: rewindList
                        //     property: "visible"
                        //     value: false
                        // }


                        // ScriptAction {
                        //     script: function () {
                        //         coverImageBg.goaway()
                        //     }
                        // }
                        PropertyAction {
                            target: coverImageBg
                            property: "thing"
                            value: false
                        }
                        // NumberAnimation {
                        //     target: emulator
                        //     properties: "width, height"
                        //     duration: 220
                        //     easing.type: Easing.InOutQuad
                        // }
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
                            // PropertyAction {
                            //     target: rewindList
                            //     property: "visible"
                            //     value: true
                            // }

                            ScriptAction {
                                script: function () {
                                    coverImageBg.show()
                                }
                            }
                            // NumberAnimation {
                            //     target: emulator
                            //     properties: "width, height"
                            //     duration: 220
                            //     easing.type: Easing.InOutQuad
                            // }
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
                focus: true
                // focus: state === "active"
                y: scope.height
                padding: 20
                property string imageSource: ""

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
                    focus: true
                    highlightMoveVelocity: -1
                    keyNavigationEnabled: true
                    highlightRangeMode: ListView.StrictlyEnforceRange
                    preferredHighlightBegin: width / 2 - 100
                    preferredHighlightEnd: width / 2 + 100
                    currentIndex: 0
                    highlight: Item {
                    }

                    Keys.onEscapePressed: function (event) {
                        console.log(":)")
                        rewindList.visible = false
                    }
                    spacing: 8
                    model: ListModel {
                        id: rewindListModel
                        ListElement {
                            imageSource: "file:system/_img/cursedmirror1.png"
                        }
                        ListElement {
                            imageSource: "file:system/_img/cursedmirror2.png"
                        }
                        ListElement {
                            imageSource: "file:system/_img/cursedmirror3.png"
                        }
                        ListElement {
                            imageSource: "file:system/_img/cursedmirror4.png"
                        }
                        ListElement {
                            imageSource: "file:system/_img/cursedmirror5.png"
                        }
                    }
                    delegate: Image {
                        required property var model
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
                        // source: rewindList.imageSource !== null ? rewindList.imageSource : ""
                        source: model.imageSource
                        anchors.verticalCenter: ListView.view.contentItem.verticalCenter
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
                    }
                }
            }

            EmulatorPage {
                id: emulator
                width: parent.width
                height: parent.height
                anchors.horizontalCenter: parent.horizontalCenter

                property bool loaded: false
                property bool blurred: false

                property int entryId: scope.entryId

                Component.onCompleted: {
                    emulator.loadGame(entryId)
                }

                onEmulationStarted: function () {
                    emulator.loaded = true
                }

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

                property double blurAmount: 0

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

            Rectangle {
                id: coverImageBg
                color: "black"
                width: parent.width
                height: parent.height
                state: "hidden"

                signal hidden()

                signal showing()

                property bool thing: false

                // function show() {
                //     console.log("showing")
                //     state = "showing"
                // }
                //
                // function goaway() {
                //     console.log("hiding")
                //     state = "hidden"
                // }

                onThingChanged: function () {
                    console.log("thing changed", thing)
                }

                onStateChanged: function () {
                    console.log("state changed", state)
                    theList.reset()
                }

                states: [
                    State {
                        name: "showing"
                        when: coverImageBg.thing

                        PropertyChanges {
                            target: coverImageBg
                            opacity: 1
                        }
                        PropertyChanges {
                            target: coverImage
                            width: parent.width
                            height: parent.height - 120
                        }
                        PropertyChanges {
                            target: barthing
                            y: parent.height - 120
                        }
                    },
                    State {
                        name: "hidden"
                        when: !coverImageBg.thing

                        PropertyChanges {
                            target: coverImageBg
                            opacity: 0
                        }
                        PropertyChanges {
                            target: coverImage
                            width: parent.width
                            height: parent.height
                        }
                        PropertyChanges {
                            target: barthing
                            y: parent.height
                        }
                    }
                ]

                transitions: [
                    Transition {
                        from: "hidden"
                        to: "showing"
                        SequentialAnimation {
                            PropertyAction {
                                target: coverImageBg
                                property: "opacity"
                                value: 1
                            }
                            ParallelAnimation {
                                NumberAnimation {
                                    target: coverImage
                                    properties: "width, height"
                                    duration: 220
                                    easing.type: Easing.InOutQuad
                                }
                                NumberAnimation {
                                    target: barthing
                                    properties: "y"
                                    duration: 220
                                    easing.type: Easing.InOutQuad
                                }
                            }
                        }
                    },
                    Transition {
                        from: "showing"
                        to: "hidden"
                        SequentialAnimation {
                            ParallelAnimation {
                                NumberAnimation {
                                    target: barthing
                                    properties: "y"
                                    duration: 220
                                    easing.type: Easing.InOutQuad
                                }
                                NumberAnimation {
                                    target: coverImage
                                    properties: "width, height"
                                    duration: 220
                                    easing.type: Easing.InOutQuad
                                }
                            }
                            PropertyAction {
                                target: coverImageBg
                                property: "opacity"
                                value: 0
                            }
                            ScriptAction {
                                script: {
                                    coverImageBg.hidden()
                                }
                            }
                        }
                    }
                ]

                Image {
                    id: coverImage
                    anchors.horizontalCenter: parent.horizontalCenter
                    height: parent.height
                    mipmap: true
                    smooth: false
                    cache: false
                    // source: theList.itemAtIndex(theList.currentIndex).model.image_url
                    fillMode: Image.PreserveAspectFit
                }

                Pane {
                    id: barthing
                    width: parent.width
                    height: 120
                    y: parent.height - 120
                    background: Rectangle {
                        color: "grey"
                    }

                    contentItem: ListView {
                        id: theList
                        orientation: ListView.Horizontal
                        layoutDirection: Qt.RightToLeft
                        highlightMoveDuration: 80
                        highlightMoveVelocity: -1
                        keyNavigationEnabled: true
                        highlightRangeMode: ListView.StrictlyEnforceRange
                        preferredHighlightBegin: width / 2 - 75
                        preferredHighlightEnd: width / 2 + 75
                        currentIndex: 0
                        highlight: Item {
                        }

                        // SoundEffect {
                        //     id: scrollSound
                        //     loops: 0
                        //     source: "file:system/sfx/glass_001.wav"
                        // }

                        function reset() {
                            currentIndex = 0
                            coverImage.source = "image://gameImages/0"
                        }

                        onCurrentIndexChanged: {
                            if (currentIndex == 0) {
                                coverImage.source = "image://gameImages/0"
                            } else {
                                coverImage.source = theList.itemAtIndex(currentIndex).model.image_url
                            }
                            // if (coverImageBg.state === "showing") {
                            //     sfx_player.play()
                            // }
                        }

                        // Keys.onEscapePressed: function (event) {
                        //     console.log(":)")
                        //     rewindList.visible = false
                        // }
                        spacing: 8
                        // model: ListModel {
                        //     id: theModel
                        //     ListElement {
                        //         imageSource: "file:system/_img/cursedmirror1.png"
                        //     }
                        //     ListElement {
                        //         imageSource: "file:system/_img/cursedmirror2.png"
                        //     }
                        //     ListElement {
                        //         imageSource: "file:system/_img/cursedmirror3.png"
                        //     }
                        //     ListElement {
                        //         imageSource: "file:system/_img/cursedmirror4.png"
                        //     }
                        //     ListElement {
                        //         imageSource: "file:system/_img/cursedmirror5.png"
                        //     }
                        // }
                        model: rewind_model
                        // delegate: Rectangle {
                        //     color: "red"
                        //     width: 100
                        //     height: 100
                        // }
                        delegate: Image {
                            required property var model
                            required property var index
                            Rectangle {
                                visible: parent.ListView.isCurrentItem
                                z: -1
                                width: parent.width + 8
                                height: parent.height + 8
                                x: -4
                                y: -4
                                color: "white"
                            }
                            cache: false
                            TapHandler {
                                onTapped: {
                                    theList.currentIndex = index
                                }
                            }


                            mipmap: true
                            smooth: false
                            // source: rewindList.imageSource !== null ? rewindList.imageSource : ""
                            source: model.image_url
                            anchors.verticalCenter: ListView.view.contentItem.verticalCenter
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