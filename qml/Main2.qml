import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Window
import QtQml.Models
import QtQuick.Layouts 1.0
import QtMultimedia
import QtQuick.Effects
import Firelight 1.0

ApplicationWindow {
    id: window
    objectName: "Application Window"

    width: 1280
    height: 720

    visible: true
    visibility: GeneralSettings.fullscreen ? Window.FullScreen : Window.Windowed

    title: qsTr("Firelight")

    property alias gameRunning: emulatorLoader.active
    property var currentGameName: ""
    property var currentEntryId: 0
    property var currentContentHash: ""

    // background: Rectangle {
    //     color: ColorPalette.neutral1000
    // }

    background: Rectangle {
        height: window.height
        width: window.width
        color: ColorPalette.neutral1000
        // Image {
        //     id: mainBackground
        //     source: "https://cdn2.steamgriddb.com/hero_thumb/2968213e79a3a2d48490ffd189255384.png"
        //     fillMode: Image.PreserveAspectCrop
        //     anchors.fill: parent
        //     // layer.enabled: true
        //     // layer.effect: MultiEffect {
        //     //     source: mainBackground
        //     //     anchors.fill: mainBackground
        //     //     blurEnabled: true
        //     //     blurMultiplier: 1.0
        //     //     blurMax: 64
        //     //     blur: 0.1
        //     // }
        // }

        // Video {
        //     id: video
        //     anchors.fill: parent
        //     source: "https://www.desktophut.com/files/1683988157-1683988157-sleepy-kirby.mp4"
        //     loops: MediaPlayer.Infinite
        //     autoPlay: true
        //     fillMode: VideoOutput.PreserveAspectCrop
        //
        //     // focus: true
        //     // Keys.onSpacePressed: video.playbackState == MediaPlayer.PlayingState ? video.pause() : video.play()
        //     // Keys.onLeftPressed: video.position = video.position - 5000
        //     // Keys.onRightPressed: video.position = video.position + 5000
        // }
        //
        // Rectangle {
        //     anchors.fill: parent
        //     color: "black"
        //     opacity: 0.8
        // }

        // Rectangle {
        //     anchors.topMargin: -70
        //     anchors.top: parent.top
        //     anchors.left: parent.left
        //     anchors.right: parent.right
        //     height: 200
        //     gradient: Gradient {
        //         GradientStop {
        //             position: 0.0; color: "black"
        //         }
        //         GradientStop {
        //             position: 1.0; color: "transparent"
        //         }
        //     }
        // }
        //
        // Rectangle {
        //     anchors.bottomMargin: -70
        //     anchors.bottom: parent.bottom
        //     anchors.left: parent.left
        //     anchors.right: parent.right
        //     height: 200
        //     gradient: Gradient {
        //         GradientStop {
        //             position: 1.0; color: "black"
        //         }
        //         GradientStop {
        //             position: 0.0; color: "transparent"
        //         }
        //     }
        // }
        // AnimatedImage {
        //     id: mainBackground
        //     source: "https://cdn.booooooom.com/wp-content/uploads/2022/07/PATTERN_11.gif"
        //     fillMode: Image.PreserveAspectCrop
        //     anchors.fill: parent
        //
        //     layer.enabled: true
        //     layer.effect: MultiEffect {
        //         source: mainBackground
        //         anchors.fill: mainBackground
        //         blurEnabled: true
        //         blurMultiplier: 1.0
        //         blurMax: 64
        //         blur: 0.5
        //     }
        // }
    }


    FirelightDialog {
        id: closeAppConfirmationDialog
        text: "Are you sure you want to close Firelight?"

        onAccepted: {
            Qt.callLater(Qt.quit)
        }
    }

    Rectangle {
        id: activeFocusHighlight
        color: "transparent"
        border.color: "lightblue"
        border.width: 2
        radius: 2
        visible: !InputMethodManager.usingMouse && parent !== null
        anchors.fill: parent
        anchors.margins: -4

        property int bounceAmplitude: 16

        SequentialAnimation {
            id: highlightBounceAnimationRight
            running: false
            ParallelAnimation {
                ScriptAction {
                    script: {
                        sfx_player.play("uibump")
                    }
                }
                PropertyAnimation {
                    target: activeFocusHighlight
                    property: "anchors.leftMargin"
                    duration: 40
                    from: -4
                    to: -4 + activeFocusHighlight.bounceAmplitude
                    easing.type: Easing.Linear
                }
                PropertyAnimation {
                    target: activeFocusHighlight
                    property: "anchors.rightMargin"
                    duration: 40
                    from: -4
                    to: -4 - activeFocusHighlight.bounceAmplitude
                    easing.type: Easing.Linear
                }
            }
            ParallelAnimation {
                PropertyAnimation {
                    target: activeFocusHighlight
                    property: "anchors.leftMargin"
                    duration: 40
                    from: -4 + activeFocusHighlight.bounceAmplitude
                    to: -4
                    easing.type: Easing.OutElastic
                    easing.amplitude: 1.5
                }
                PropertyAnimation {
                    target: activeFocusHighlight
                    property: "anchors.rightMargin"
                    duration: 40
                    from: -4 - activeFocusHighlight.bounceAmplitude
                    to: -4
                    easing.type: Easing.OutElastic
                    easing.amplitude: 1.5
                }
            }
        }

        SequentialAnimation {
            id: highlightBounceAnimationLeft
            running: false
            ParallelAnimation {
                ScriptAction {
                    script: {
                        sfx_player.play("uibump")
                    }
                }
                PropertyAnimation {
                    target: activeFocusHighlight
                    property: "anchors.leftMargin"
                    duration: 40
                    from: -4
                    to: -4 - activeFocusHighlight.bounceAmplitude
                    easing.type: Easing.Linear
                }
                PropertyAnimation {
                    target: activeFocusHighlight
                    property: "anchors.rightMargin"
                    duration: 40
                    from: -4
                    to: -4 + activeFocusHighlight.bounceAmplitude
                    easing.type: Easing.Linear
                }
            }
            ParallelAnimation {
                PropertyAnimation {
                    target: activeFocusHighlight
                    property: "anchors.leftMargin"
                    duration: 40
                    from: -4 - activeFocusHighlight.bounceAmplitude
                    to: -4
                    easing.type: Easing.OutElastic
                    easing.amplitude: 1.5
                }
                PropertyAnimation {
                    target: activeFocusHighlight
                    property: "anchors.rightMargin"
                    duration: 40
                    from: -4 + activeFocusHighlight.bounceAmplitude
                    to: -4
                    easing.type: Easing.OutElastic
                    easing.amplitude: 1.5
                }
            }
        }


        // NumberAnimation {
        //     target: activeFocusHighlight
        //     property: "opacity"
        //     duration: 100
        //     easing.type: Easing.InOutQuad
        //     loops: Animation.Infinite
        // }

        SequentialAnimation {
            running: true
            loops: Animation.Infinite // Loop the animation infinitely
            NumberAnimation {
                target: activeFocusHighlight
                property: "opacity"
                from: 0.65
                to: 1.0
                duration: 2000
                easing.type: Easing.InOutQuad
            }
            NumberAnimation {
                target: activeFocusHighlight
                property: "opacity"
                from: 1.0
                to: 0.65
                duration: 2000
                easing.type: Easing.InOutQuad
            }
        }

        Connections {
            target: window

            function onActiveFocusItemChanged() {
                let item = window.activeFocusItem
                if (item && item.hasOwnProperty('showGlobalCursor') && item.showGlobalCursor) {
                    let cursorItem = item

                    if (cursorItem.hasOwnProperty("globalCursorProxy")) {
                        cursorItem = cursorItem.globalCursorProxy
                    }
                    activeFocusHighlight.parent = cursorItem
                    if (cursorItem.hasOwnProperty('background')) {
                        activeFocusHighlight.radius = cursorItem.background.radius + 4
                    }
                } else {
                    activeFocusHighlight.parent = null
                }
            }
        }
    }

    Connections {
        target: Router

        function onRouteChanged(route) {
            let parts = route.split("/")
            if (parts.length > 0) {
                let id = parts[0]

                if (id === "settings") {
                    let section = parts.length > 1 ? parts[1] : "appearance"
                    screenStack.push(settingsScreen, {section: section})
                }
            }
        }
    }

    onActiveFocusItemChanged: {
        if (activeFocusItem === null || activeFocusItem.width === 0 || activeFocusItem.height === 0) {
            return
        }

        activeFocusItem.grabToImage(function (result) {
            debugImage.width = result.width
            debugImage.height = result.height
            debugImage.source = result.url
        })
    }


    Component {
        id: profileEditor
        ControllerProfilePage {

        }
    }

    Component {
        id: keyboardProfileEditor
        KeyboardProfilePage {

        }
    }

    Window {
        id: debugWindow
        objectName: "DebugWindow"

        width: 400
        height: 400

        visible: false
        title: "Debug"

        color: "black"

        Image {
            id: debugImage
            source: ""
            anchors.centerIn: parent
            fillMode: Image.PreserveAspectFit
        }
    }

    Component {
        id: nowPlayingPage
        NowPlayingPage {
            property bool topLevel: true
            property string topLevelName: "nowPlaying"

            onResumeGamePressed: function () {
                quickMenuBar.close()
            }

            onRestartGamePressed: function () {
                emulatorLoader.item.resetGame()
                quickMenuBar.close()
            }

            onCloseGamePressed: function () {
                emulatorLoader.stopGame()
            }

            onRewindPressed: function() {
                emulatorLoader.item.createRewindPoints()
            }
        }
    }

    Component {
        id: rewindMenu
        RewindMenu {

        }
    }

    Component {
        id: emuPage
        NewEmulatorPage {
            onRewindPointsReady: function(points) {
                screenStack.pushItem(rewindMenu, {model: points}, StackView.Immediate)
            }
        }
    }

    Loader {
        id: emulatorLoader
        property var entryId

        property bool shouldDeactivate: false

        active: false
        sourceComponent: emuPage

        property bool paused: !emulatorLoader.activeFocus

        function stopGame() {
            shouldDeactivate = true
            if (emulatorLoader.StackView.status === StackView.Active) {
                emulatorLoader.StackView.view.popCurrentItem()
            }
        }

        onPausedChanged: function () {
            if (emulatorLoader.item != null) {
                emulatorLoader.item.paused = emulatorLoader.paused
            }
        }

        // onActiveChanged: {
        //     if (!active && emulatorLoader.StackView.status === StackView.Active) {
        //         emulatorLoader.StackView.view.popCurrentItem()
        //     }
        // }

        StackView.onDeactivated: {
            if (shouldDeactivate) {
                active = false
                shouldDeactivate = false
            }
        }

        StackView.onActivating: {
            // setSource(emuPage, {
            //     gameData: emulatorLoader.gameData,
            //     saveData: emulatorLoader.saveData,
            //     corePath: emulatorLoader.corePath,
            //     contentHash: emulatorLoader.contentHash,
            //     saveSlotNumber: emulatorLoader.saveSlotNumber,
            //     platformId: emulatorLoader.platformId,
            //     contentPath: emulatorLoader.contentPath
            // })
            emulatorLoader.item.startGame()
            emulatorLoader.item.paused = emulatorLoader.paused
            // active = true
        }

        onLoaded: function () {
            emulatorLoader.item.loadGame(emulatorLoader.entryId)
            // emulatorLoader.item.startGame(gameData, saveData, corePath, contentHash, saveSlotNumber, platformId, contentPath)
            // emulatorLoader.item.paused = emulatorLoader.paused
        }
    }

    Component {
        id: newUserFlow
        NewUserScreen {
            onDoneButtonPressed: function () {
                screenStack.replaceCurrentItem(homeScreen, {}, StackView.PushTransition)
            }
        }
    }

    StackView {
        id: screenStack
        anchors.fill: parent
        focus: true
        initialItem: newUserFlow

        Keys.onRightPressed: function(event) {
            highlightBounceAnimationRight.running = true
        }

        Keys.onLeftPressed: function(event) {
            highlightBounceAnimationLeft.running = true
        }

        property bool blur: false

        property real blurAmount: blur ? 0.5 : 0

        Behavior on blurAmount {
            NumberAnimation {
                easing.type: Easing.InOutQuad
                duration: 250
            }
        }

        layer.enabled: blurAmount !== 0
        layer.effect: MultiEffect {
            // enabled: root.blurAmount !== 0
            source: screenStack
            anchors.fill: screenStack
            blurEnabled: true
            blurMultiplier: 0
            blurMax: 64
            blur: screenStack.blurAmount
        }

        onCurrentItemChanged: function () {
            if (screenStack.currentItem !== emulatorLoader) {
                quickMenuBar.close()
            }
        }

        Keys.onPressed: function (event) {
            if (event.key === Qt.Key_F1) {
                quickMenuBar.open()
            }

            if (event.key === Qt.Key_F12) {
                debugWindow.visible = !debugWindow.visible
            }

            if (event.key === Qt.Key_F11) {
                console.log("Going offline")
                quickMenuBar.visible = true
                achievement_manager.setOnlineForTesting(false)
            }

            if (event.key === Qt.Key_F10) {
                console.log("Going online")
                achievement_manager.setOnlineForTesting(true)
            }
            if (event.key === Qt.Key_Escape) {
                quickMenuBar.visible = true
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
                    duration: 150
                    easing.type: Easing.InOutQuad
                }
                PropertyAnimation {
                    property: "scale"
                    from: 1
                    to: 0.95
                    duration: 150
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

        replaceEnter: Transition {
            PropertyAnimation {
                property: "opacity"
                from: 0
                to: 1
                duration: 200
                easing.type: Easing.InOutQuad
            }
        }
        replaceExit: Transition {
            PropertyAnimation {
                property: "opacity"
                from: 1
                to: 0
                duration: 200
                easing.type: Easing.InOutQuad
            }
        }
    }

    Component {
        id: controllerPage
        ControllersPage {
            property bool topLevel: true
            property string topLevelName: "controllers"
            property string pageTitle: "Controllers"

            onEditProfileButtonClicked: function (name, playerNumber) {
                if (name === "Keyboard") {
                    screenStack.pushItem(keyboardProfileEditor, {playerNumber: playerNumber}, StackView.PushTransition)
                } else {
                    screenStack.pushItem(profileEditor, {playerNumber: playerNumber}, StackView.PushTransition)
                }
            }
        }
    }

    Popup {
        id: quickMenuBar
        parent: Overlay.overlay
        modal: true
        focus: true

        padding: 16
        closePolicy: Popup.NoAutoClose

        width: parent.width
        height: parent.height
        // height: 72
        // y: parent.height - height
        // width: parent.width

        onAboutToShow: function () {
            sfx_player.play("quickopen")
            screenStack.blur = true
            if (window.gameRunning && screenStack.currentItem === emulatorLoader) {
                quickMenuStack.pushItem(nowPlayingPage, {
                    entryId: currentEntryId,
                    contentHash: currentContentHash,
                    undoEnabled: false
                }, StackView.PushTransition)
                quickMenuStack.forceActiveFocus()
            } else {
                menuBar.forceActiveFocus()
            }
        }

        onAboutToHide: function () {
            screenStack.blur = false
        }

        onClosed: function () {
            quickMenuStack.clear()
        }

        Overlay.modal: Rectangle {
            color: "black"
            anchors.fill: parent
            opacity: 0.6
            Behavior on opacity {
                NumberAnimation {
                    duration: 200
                }
            }
        }

        enter: Transition {
            ParallelAnimation {
                PropertyAnimation {
                    target: contentThing
                    property: "opacity"
                    from: 0
                    to: 1
                    duration: 250
                    easing.type: Easing.InOutQuad
                }
                PropertyAnimation {
                    target: contentThing
                    property: "y"
                    from: 20
                    to: 0
                    duration: 250
                    easing.type: Easing.InOutQuad
                }
            }
        }

        exit: Transition {
            ParallelAnimation {
                PropertyAnimation {
                    target: contentThing
                    property: "opacity"
                    from: 1
                    to: 0
                    duration: 250
                    easing.type: Easing.InOutQuad
                }
                PropertyAnimation {
                    target: contentThing
                    property: "y"
                    from: 0
                    to: 20
                    duration: 250
                    easing.type: Easing.InOutQuad
                }
            }
        }

        background: Item {
        }
        contentItem: FocusScope {
            focus: true
            id: contentThing

            Keys.onEscapePressed: function (event) {
                sfx_player.play("quickclose")
                quickMenuBar.close()
            }

            Keys.onBackPressed: function (event) {
                sfx_player.play("quickclose")
                quickMenuBar.close()
            }

            StackView {
                id: quickMenuStack
                enabled: depth > 0
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: menuBar.top

                // onActiveFocusChanged: function () {
                //     if (!InputMethodManager.usingMouse && !quickMenuStack.activeFocus) {
                //         quickMenuStack.clear(StackView.PopTransition)
                //         menuBar.forceActiveFocus()
                //     }
                // }

                Keys.onBackPressed: function (event) {
                    quickMenuStack.clear(StackView.PopTransition)
                    menuBar.forceActiveFocus()
                }

                // Keys.onPressed: function(event) {
                //     if (event.key === Qt.Key_Back || event.key === Qt.Key_Escape) {
                //         menuBar.forceActiveFocus()
                //         quickMenuStack.clear()
                //         event.accept = true
                //     }
                // }

                replaceEnter: Transition {
                    PropertyAnimation {
                        property: "opacity"
                        from: 0
                        to: 1
                        duration: 150
                        easing.type: Easing.InOutQuad
                    }
                    PropertyAnimation {
                        property: "y"
                        from: 20
                        to: 0
                        duration: 150
                        easing.type: Easing.InOutQuad
                    }
                }


                replaceExit: Transition {
                    PropertyAnimation {
                        property: "opacity"
                        from: 1
                        to: 0
                        duration: 150
                        easing.type: Easing.InOutQuad
                    }
                    PropertyAnimation {
                        property: "y"
                        from: 0
                        to: 20
                        duration: 150
                        easing.type: Easing.InOutQuad
                    }
                }

                pushEnter: Transition {
                    PropertyAnimation {
                        property: "opacity"
                        from: 0
                        to: 1
                        duration: 150
                        easing.type: Easing.InOutQuad
                    }
                    PropertyAnimation {
                        property: "y"
                        from: 20
                        to: 0
                        duration: 150
                        easing.type: Easing.InOutQuad
                    }
                }

                popExit: Transition {
                    PropertyAnimation {
                        property: "opacity"
                        from: 1
                        to: 0
                        duration: 150
                        easing.type: Easing.InOutQuad
                    }
                    PropertyAnimation {
                        property: "y"
                        from: 0
                        to: 20
                        duration: 150
                        easing.type: Easing.InOutQuad
                    }
                }
            }


            Text {
                id: notPlayingAnythingText
                visible: false
                text: "You're not currently playing anything"
                color: "white"
                font.family: Constants.regularFontFamily
                font.pixelSize: 18
                font.weight: Font.Normal
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            ListView {
                id: menuBar
                KeyNavigation.up: quickMenuStack
                width: (48 * 4) + 5 * 42
                focus: true
                height: 32
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                orientation: ListView.Horizontal
                currentIndex: 0
                interactive: false

                onCurrentIndexChanged: function () {
                    if (!InputMethodManager.usingMouse) {
                        sfx_player.play("rewindscroll")
                    }
                }
                model: ListModel {
                    ListElement {
                        icon: "\uea0b"; label: "Now Playing"
                    }
                    ListElement {
                        icon: "\ue88a"; label: "Home"
                    }
                    ListElement {
                        icon: "\uf135"; label: "Controllers"
                    }
                    ListElement {
                        icon: "\ue8b8"; label: "Settings"
                    }
                    ListElement {
                        icon: "\ue8ac"; label: "Power"
                    }
                }
                spacing: 48
                delegate: FirelightButton {
                    id: theButton
                    focus: true
                    required property var model
                    required property var index
                    tooltipLabel: model.label
                    tooltipOnTop: true
                    circle: true
                    flat: true
                    iconCode: model.icon

                    Text {
                        id: nowPlayingCurrentGameText
                        color: "white"
                        font.family: Constants.regularFontFamily
                        text: "Now playing: " + window.currentGameName
                        font.pixelSize: 13
                        visible: window.gameRunning && model.label === "Now Playing"
                        anchors.top: theButton.bottom
                        anchors.horizontalCenter: theButton.horizontalCenter
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    Keys.onUpPressed: function (event) {
                        if (model.label === "Controllers") {
                            if (quickMenuStack.depth === 1) {
                                quickMenuStack.popCurrentItem(StackView.PopTransition)
                            } else {
                                quickMenuStack.replaceCurrentItem(controllerPage, {}, StackView.ReplaceTransition)
                                quickMenuStack.forceActiveFocus()
                            }
                        }

                        if (model.label === "Power") {
                            closeAppConfirmationDialog.open()
                        }
                    }

                    onClicked: function (event) {
                        console.log("Calling on click")
                        if (model.label === "Now Playing") {
                            if (!window.gameRunning) {
                                quickMenuStack.replaceCurrentItem(notPlayingAnythingText, {}, StackView.ReplaceTransition)
                                return
                            }


                            if (screenStack.currentItem !== emulatorLoader) {
                                if (screenStack.find(function (item, index) {
                                    return item === emulatorLoader
                                }) == null) {
                                    screenStack.pushItem(emulatorLoader, {}, StackView.PushTransition)
                                } else {
                                    screenStack.popToItem(emulatorLoader, StackView.PopTransition)
                                }
                            }

                            console.log(quickMenuStack.currentItem)

                            if (quickMenuStack.currentItem !== nowPlayingPage) {
                                quickMenuStack.replaceCurrentItem(nowPlayingPage, {
                                    entryId: currentEntryId,
                                    contentHash: currentContentHash,
                                    undoEnabled: false
                                }, StackView.ReplaceTransition)
                            }

                            quickMenuStack.forceActiveFocus()

                            // } else if (screenStack.currentItem === emulatorLoader) {
                            //         screenStack.popCurrentItem(StackView.ReplaceTransition)
                        }
                        // if (model.label === "Home") {
                        //     if (screenStack.currentItem === emulatorLoader) {
                        //         screenStack.popToItem(homeScreen, StackView.PopTransition)
                        //         // screenStack.replaceCurrentItem(homeScreen, {}, StackView.ReplaceTransition)
                        //     }
                        // }
                        if (model.label === "Home") {
                            screenStack.popToIndex(0, StackView.PopTransition)
                            quickMenuBar.close()
                        }
                        if (model.label === "Settings") {
                            Router.navigateTo("settings")
                            quickMenuBar.close()
                        }

                        if (model.label === "Controllers") {
                            if (quickMenuStack.currentItem == controllerPage) {
                                quickMenuStack.popCurrentItem(StackView.PopTransition)
                            } else {
                                quickMenuStack.replaceCurrentItem(controllerPage, {}, StackView.ReplaceTransition)
                                quickMenuStack.forceActiveFocus()
                            }
                        }

                        if (model.label === "Power") {
                            closeAppConfirmationDialog.open()
                        }
                    }
                }
            }

        }
    }


    Rectangle {
        id: overlay
        objectName: "Screen Overlay"
        anchors.fill: parent
        color: "black"
        opacity: 0
    }

    SequentialAnimation {
        id: gameStartAnimation
        running: false

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
                screenStack.pushItem(emulatorLoader, {}, StackView.Immediate)
                overlay.opacity = 0
            }
        }
    }

    FirelightDialog {
        id: closeGameDialog
        text: "You're currently playing:\n\n" + window.currentGameName + "\n\nDo you want to close it?"
    }

    Component {
        id: shopPage
        ShopLandingPage {
            property bool topLevel: true
            property string topLevelName: "shop"
            property string pageTitle: "Mod shop (coming soon!)"
            model: shop_item_model
        }
    }

    Component {
        id: libraryPage2
        LibraryPage2 {
            id: libPage
            objectName: "Library Page 2"
            property bool topLevel: true
            property string topLevelName: "library"
            property string pageTitle: "Library"
            // model: library_short_model
            model: LibraryEntryModel

            onPlayButtonClicked: function (entryId) {
                console.log("Gonna start: " + entryId)
                if (window.gameRunning) {
                    // closeGameDialog.entryId = entryId
                    closeGameDialog.openAndDoOnAccepted(function () {
                        emulatorLoader.active = false

                        emulatorLoader.entryId = entryId
                        emulatorLoader.active = true
                        libPage.playLaunchAnimation()
                    })
                } else {
                    emulatorLoader.entryId = entryId
                    emulatorLoader.active = true
                    libPage.playLaunchAnimation()
                }
            }

            onLaunchAnimationFinished: {
                gameStartAnimation.running = true
            }

            onOpenDetails: function (id) {
                // contentStack.push(gameDetailsPage)
            }

            Component {
                id: gameDetailsPage
                GameDetailsPage {
                    entryId: 115
                }
            }

            // onEntryClicked: function (id) {
            //     emulatorScreen.loadGame(id)
            // }
        }
    }

    Component {
        id: homeScreen

        HomeScreen {
            libraryPage: libraryPage2
            modShopPage: shopPage

            focus: true

            onMenuButtonClicked: {
                quickMenuBar.open()
            }
        }
    }

    Component {
        id: settingsScreen

        SettingsScreen {
            layer.enabled: true
        }
    }

    Popup {
        id: controllerConnectedPopup

        property int playerNumber: 0
        property string controllerName: ""
        property string iconSourceUrl: ""


        width: 400
        height: 80

        x: window.width - width - 20
        y: 100

        parent: Overlay.overlay
        modal: false
        focus: false

        Timer {
            id: timer
            interval: 4000
            running: false
            repeat: false
            onTriggered: controllerConnectedPopup.close()
        }

        Timer {
            id: startTimer
            interval: 500
            running: false
            repeat: false
            onTriggered: controllerConnectedPopup.open()
        }

        background: Rectangle {
            color: ColorPalette.neutral800
            radius: 8
        }

        RowLayout {
            anchors.fill: parent
            Image {
                source: controllerConnectedPopup.iconSourceUrl
                Layout.fillHeight: true
                Layout.preferredWidth: height
                sourceSize.width: 80
                sourceSize.height: 80
                fillMode: Image.PreserveAspectFit
            }
            ColumnLayout {
                Layout.fillHeight: true
                Layout.fillWidth: true
                Text {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "white"
                    font.pixelSize: 16
                    font.family: Constants.regularFontFamily
                    verticalAlignment: Text.AlignBottom
                    horizontalAlignment: Text.AlignLeft
                    text: "Player " + controllerConnectedPopup.playerNumber + " connected"
                }
                Text {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    font.pixelSize: 13
                    color: "white"
                    font.family: Constants.regularFontFamily
                    verticalAlignment: Text.AlignTop
                    horizontalAlignment: Text.AlignLeft
                    text: controllerConnectedPopup.controllerName
                }
            }
        }

        enter: Transition {
            ScriptAction {
                script: {
                    sfx_player.play("defaultnotification")
                }
            }
            NumberAnimation {
                target: controllerConnectedPopup
                property: "x"
                from: window.width - controllerConnectedPopup.width
                to: window.width - controllerConnectedPopup.width - 20
                duration: 250
                easing.type: Easing.InOutQuad
            }
            NumberAnimation {
                target: controllerConnectedPopup
                property: "opacity"
                from: 0
                to: 1
                duration: 250
                easing.type: Easing.InOutQuad
            }
            ScriptAction {
                script: {
                    timer.restart()
                }
            }
        }

        exit: Transition {
            NumberAnimation {
                target: controllerConnectedPopup
                property: "x"
                from: window.width - controllerConnectedPopup.width - 20
                to: window.width - controllerConnectedPopup.width
                duration: 250
                easing.type: Easing.InOutQuad
            }
            NumberAnimation {
                target: controllerConnectedPopup
                property: "opacity"
                from: 1
                to: 0
                duration: 250
                easing.type: Easing.InOutQuad
            }
            ScriptAction {
                script: {
                    timer.stop()
                }
            }
        }

        Connections {
            target: controller_manager

            function onControllerConnected(player, name, iconSourceUrl) {
                controllerConnectedPopup.playerNumber = player
                controllerConnectedPopup.controllerName = name
                controllerConnectedPopup.iconSourceUrl = iconSourceUrl
                startTimer.start()
            }
        }
    }

    Popup {
        id: controllerDisonnectedPopup

        width: 400
        height: 80

        property int playerNumber: 0
        property string controllerName: ""
        property string iconSourceUrl: ""

        x: window.width - width - 20
        y: 100

        parent: Overlay.overlay
        modal: false
        focus: false

        Timer {
            id: timer2
            interval: 4000
            running: false
            repeat: false
            onTriggered: controllerDisonnectedPopup.close()
        }

        Timer {
            id: startTimer2
            interval: 500
            running: false
            repeat: false
            onTriggered: controllerDisonnectedPopup.open()
        }

        background: Rectangle {
            color: ColorPalette.neutral800
            radius: 8
        }

        RowLayout {
            anchors.fill: parent
            Image {
                source: controllerDisonnectedPopup.iconSourceUrl
                Layout.fillHeight: true
                Layout.preferredWidth: height
                sourceSize.width: 80
                sourceSize.height: 80
                fillMode: Image.PreserveAspectFit
            }
            ColumnLayout {
                Layout.fillHeight: true
                Layout.fillWidth: true
                Text {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "white"
                    font.pixelSize: 16
                    font.family: Constants.regularFontFamily
                    verticalAlignment: Text.AlignBottom
                    horizontalAlignment: Text.AlignLeft
                    text: "Player " + controllerDisonnectedPopup.playerNumber + " disconnected"
                }
                Text {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    font.pixelSize: 13
                    color: "white"
                    font.family: Constants.regularFontFamily
                    verticalAlignment: Text.AlignTop
                    horizontalAlignment: Text.AlignLeft
                    text: controllerDisonnectedPopup.controllerName
                }
            }
        }

        enter: Transition {
            ScriptAction {
                script: {
                    sfx_player.play("defaultnotification")
                }
            }
            NumberAnimation {
                target: controllerDisonnectedPopup
                property: "x"
                from: window.width - controllerDisonnectedPopup.width
                to: window.width - controllerDisonnectedPopup.width - 20
                duration: 250
                easing.type: Easing.InOutQuad
            }
            NumberAnimation {
                target: controllerDisonnectedPopup
                property: "opacity"
                from: 0
                to: 1
                duration: 250
                easing.type: Easing.InOutQuad
            }
            ScriptAction {
                script: {
                    timer2.restart()
                }
            }
        }

        exit: Transition {
            NumberAnimation {
                target: controllerDisonnectedPopup
                property: "x"
                from: window.width - controllerDisonnectedPopup.width - 20
                to: window.width - controllerDisonnectedPopup.width
                duration: 250
                easing.type: Easing.InOutQuad
            }
            NumberAnimation {
                target: controllerDisonnectedPopup
                property: "opacity"
                from: 1
                to: 0
                duration: 250
                easing.type: Easing.InOutQuad
            }
            ScriptAction {
                script: {
                    timer2.stop()
                }
            }
        }

        Connections {
            target: controller_manager

            function onControllerDisconnected(player, name, iconSourceUrl) {
                controllerDisonnectedPopup.playerNumber = player
                controllerDisonnectedPopup.controllerName = name
                controllerDisonnectedPopup.iconSourceUrl = iconSourceUrl
                startTimer2.start()
            }
        }
    }
}
