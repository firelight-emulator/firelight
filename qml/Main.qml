import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Effects
import Firelight 1.0
import FirelightStyle 1.0

ApplicationWindow {
    id: window

    property bool borderlessFullscreen: false

    width: 1280
    height: 720

    minimumWidth: 640
    minimumHeight: 480
    visible: true
    // flags: Qt.Window | Qt.FramelessWindowHint | Qt.MaximizeUsingFullscreenGeometryHint
    visibility: GeneralSettings.fullscreen ? Window.FullScreen : Window.Windowed
    title: qsTr("Firelight")
    color: "black"

    Rectangle {
        id: appRoot
        anchors.fill: parent
        focus: true
        color: "black"

        signal customStateChanged(string newState)

        state: "notPlayingGame"

        Keys.onEscapePressed: function () {
            if (state === "gameSuspended") {
                state = "playingGame"
            } else {
                closeAppDialog.open()
            }
        }

        states: [
            State {
                name: "notPlayingGame"
                PropertyChanges {
                    target: mainMenu
                    enabled: true
                    focus: true
                }
                PropertyChanges {
                    target: emulator
                    enabled: false
                    focus: false
                }
            },
            State {
                name: "playingGame"
                PropertyChanges {
                    target: mainMenu
                    enabled: false
                    focus: false
                }
                PropertyChanges {
                    target: emulator
                    enabled: true
                    focus: true
                }
            },
            State {
                name: "gameSuspended"
                PropertyChanges {
                    target: mainMenu
                    enabled: true
                    focus: true
                }
                PropertyChanges {
                    target: emulator
                    enabled: false
                    focus: false
                }
            }
        ]

        transitions: [
            Transition {
                from: "notPlayingGame"
                to: "playingGame"
                SequentialAnimation {
                    PropertyAnimation {
                        target: overlayThing
                        property: "opacity"
                        from: 0
                        to: 1
                        duration: 300
                        easing.type: Easing.InOutQuad
                    }
                    PropertyAction {
                        target: mainMenu
                        property: "visible"
                        value: false
                    }
                    PropertyAction {
                        target: mainMenu
                        property: "playingGame"
                        value: true
                    }
                    ScriptAction {
                        script: {
                            emulator.layer.enabled = false
                        }
                    }
                    PropertyAction {
                        target: emulator
                        property: "visible"
                        value: true
                    }
                    ScriptAction {
                        script: {
                            emulator.resumeGame()
                        }
                    }
                    PropertyAnimation {
                        target: overlayThing
                        property: "opacity"
                        from: 1
                        to: 0
                        duration: 300
                        easing.type: Easing.InOutQuad
                    }
                    ScriptAction {
                        script: {
                            emulator.startEmulation()
                        }
                    }
                    ScriptAction {
                        script: {
                            appRoot.customStateChanged("playingGame")
                        }
                    }
                }
            },
            Transition {
                from: "playingGame"
                to: "gameSuspended"
                SequentialAnimation {
                    PropertyAction {
                        target: mainMenu
                        property: "index"
                        value: -1
                    }
                    PropertyAction {
                        target: mainMenu
                        property: "index"
                        value: 5
                    }
                    PropertyAction {
                        target: nowPlayingButton
                        property: "checked"
                        value: true
                    }
                    ScriptAction {
                        script: {
                            emulator.pauseGame()
                            emulator.blurAmount = 0
                            emulator.layer.enabled = true
                        }
                    }
                    PropertyAction {
                        target: mainMenu
                        property: "opacity"
                        value: 0
                    }
                    PropertyAction {
                        target: mainMenu
                        property: "visible"
                        value: true
                    }
                    ParallelAnimation {
                        PropertyAnimation {
                            target: mainMenu
                            property: "opacity"
                            from: 0
                            to: 1
                            duration: 300
                            easing.type: Easing.InOutQuad
                        }
                        PropertyAnimation {
                            target: mainMenu
                            property: "scale"
                            from: 1.2
                            to: 1
                            duration: 300
                            easing.type: Easing.InOutQuad
                        }
                        PropertyAnimation {
                            target: emulator
                            property: "blurAmount"
                            from: 0
                            to: 1
                            duration: 300
                            easing.type: Easing.InOutQuad
                        }
                        PropertyAnimation {
                            target: emulatorOverlay
                            property: "opacity"
                            from: 0
                            to: 0.4
                            duration: 300
                            easing.type: Easing.InOutQuad
                        }
                    }
                    ScriptAction {
                        script: {
                            appRoot.customStateChanged("gameSuspended")
                        }
                    }
                }
            },
            Transition {
                from: "gameSuspended"
                to: "playingGame"
                SequentialAnimation {
                    ParallelAnimation {
                        PropertyAnimation {
                            target: mainMenu
                            property: "opacity"
                            from: 1
                            to: 0
                            duration: 300
                            easing.type: Easing.InOutQuad
                        }
                        PropertyAnimation {
                            target: mainMenu
                            property: "scale"
                            from: 1
                            to: 1.2
                            duration: 300
                            easing.type: Easing.InOutQuad
                        }
                        PropertyAnimation {
                            target: emulator
                            property: "blurAmount"
                            from: 1
                            to: 0
                            duration: 300
                            easing.type: Easing.InOutQuad
                        }
                        PropertyAnimation {
                            target: emulatorOverlay
                            property: "opacity"
                            from: 0.4
                            to: 0
                            duration: 300
                            easing.type: Easing.InOutQuad
                        }
                    }
                    ScriptAction {
                        script: {
                            emulator.layer.enabled = false
                            emulator.resumeGame()
                            // emulator.blurry = true
                        }
                    }
                    PropertyAction {
                        target: mainMenu
                        property: "visible"
                        value: false
                    }
                    ScriptAction {
                        script: {
                            appRoot.customStateChanged("playingGame")
                        }
                    }
                }
            },
            Transition {
                from: "gameSuspended"
                to: "notPlayingGame"
                SequentialAnimation {
                    ScriptAction {
                        script: {
                            waitingDialog.open()
                        }
                    }
                    PauseAnimation {
                        duration: 300
                    }
                    ParallelAnimation {
                        PropertyAnimation {
                            target: emulator
                            property: "opacity"
                            from: 1
                            to: 0
                            duration: 300
                            easing.type: Easing.InOutQuad
                        }
                        PropertyAnimation {
                            target: emulatorOverlay
                            property: "opacity"
                            to: 0
                            duration: 300
                            easing.type: Easing.InOutQuad
                        }
                    }
                    ScriptAction {
                        script: {
                            emulator.stopEmulation()
                        }
                    }
                    PropertyAction {
                        target: emulator
                        property: "visible"
                        value: false
                    }
                    PropertyAction {
                        target: emulator
                        property: "opacity"
                        value: 1
                    }
                    PropertyAction {
                        target: mainMenu
                        property: "playingGame"
                        value: false
                    }
                    ScriptAction {
                        script: {
                            if (mainMenu.index === 5) {
                                mainMenu.index = 0
                            }
                        }
                    }
                    PauseAnimation {
                        duration: 300
                    }
                    ScriptAction {
                        script: {
                            waitingDialog.accept()
                        }
                    }
                    ScriptAction {
                        script: {
                            appRoot.customStateChanged("notPlayingGame")
                        }
                    }
                }
            }
        ]

        FirelightDialog {
            id: waitingDialog
            text: "Please wait..."
            showButtons: false
        }

        Item {
            id: homePage
            visible: false
            Text {
                text: "Here's where the Home menu will go!"
                anchors.centerIn: parent
                color: Constants.colorTestTextActive
                font.pointSize: 16
                font.family: Constants.regularFontFamily
            }
        }

        HackPage {
            id: explorePage
            visible: false
        }

        LibraryPage {
            id: libraryPage
            visible: false
            property int nextEntryId: -1

            fontFamilyName: Constants.regularFontFamily
            onEntryClicked: function (entryId) {
                // gameLoader.loadGame(entryId)
                // fullPane.loadGame(entryId)
                if (emulator.isRunning()) {
                    dialog.entryId = entryId
                    dialog.open()
                } else {
                    gameLoader.loadGame(entryId)
                }
            }

            FirelightDialog {
                id: dialog
                property int entryId
                parent: Overlay.overlay
                anchors.centerIn: parent
                text: "<p>You need to close the current game before loading another one. Are you sure you want
                     to close the current game?</p>"

                onAccepted: {
                    libraryPage.nextEntryId = entryId
                    appRoot.state = "notPlayingGame"
                }
            }

            Connections {
                target: appRoot

                function onCustomStateChanged(newState) {
                    if (newState === "notPlayingGame" && libraryPage.nextEntryId !== -1) {
                        var entryId = libraryPage.nextEntryId
                        libraryPage.nextEntryId = -1

                        gameLoader.loadGame(entryId)
                    }
                }
            }

        }

        ControllersPage {
            id: controllersPage
            visible: false
        }

        SettingsPage {
            id: settingsPage
            visible: false
        }

        Item {
            id: nowPlayingPage
            visible: false
            Column {
                id: topPart
                spacing: 12
                anchors.left: parent.left
                anchors.top: parent.top

                anchors.leftMargin: 24
                anchors.topMargin: 24
                Text {
                    text: "You're playing:"
                    color: Constants.colorTestTextActive
                    font.pointSize: 14
                    font.family: Constants.regularFontFamily
                }
                Text {
                    text: emulator.currentGameName
                    color: Constants.colorTestTextActive
                    font.pointSize: 24
                    font.family: Constants.regularFontFamily
                }
                Item {
                    height: 60
                    width: 10
                }
            }
            Column {
                spacing: 2
                anchors.left: parent.left
                anchors.top: topPart.bottom
                anchors.right: parent.right

                anchors.leftMargin: 24

                Button {
                    text: "Resume Game"
                    // labelIcon: "\uf7d0"
                    height: 40
                    width: 200
                    checkable: false

                    onClicked: function () {
                        appRoot.state = "playingGame"
                    }
                }

                Button {
                    text: "Restart Game"
                    // labelIcon: "\uf053"
                    height: 40
                    width: 200
                    checkable: false

                    onClicked: function () {
                        emulator.resetGame()
                        appRoot.state = "playingGame"
                    }
                }

                Button {
                    text: "Quit Game"
                    // labelIcon: "\ue5cd"
                    height: 40
                    width: 200
                    checkable: false

                    onClicked: function () {
                        closeGameDialog.open()
                    }
                }
            }
        }

        GameLoader {
            id: gameLoader

            onGameLoaded: function (entryId, romData, saveData, corePath) {
                emulator.loadTheThing(entryId, romData, saveData, corePath)
            }

            onGameLoadFailedOrphanedPatch: function (entryId) {
                patchClickedDialog.open()
            }
        }

        FirelightDialog {
            id: patchClickedDialog
            text: "<p>Sorry, hot-patching is currently disabled as I build out the \"patches\" portion of the content
             database - it's coming soon!</p>"
        }

        EmulatorPage {
            id: emulator
            anchors.fill: parent
            visible: false
            // smooth: false

            property double blurAmount: 0

            layer.enabled: false
            layer.effect: MultiEffect {
                source: emulator
                anchors.fill: emulator
                blurEnabled: true
                blurMultiplier: 1.0
                blurMax: 64
                blur: emulator.blurAmount
            }

            Keys.onEscapePressed: {
                appRoot.state = "gameSuspended"
            }

            onGameLoaded: {
                appRoot.state = "playingGame"
            }

            Connections {
                target: window_resize_handler

                function onWindowResizeStarted() {
                    if (appRoot.state === "playingGame") {
                        emulator.pauseGame()
                    }
                }

                function onWindowResizeFinished() {
                    if (appRoot.state === "playingGame") {
                        emulator.resumeGame()
                    }
                }
            }
        }

        Rectangle {
            id: emulatorOverlay
            anchors.fill: emulator
            color: "black"
            opacity: 0
        }

        Pane {
            id: mainMenu
            property int index: 0
            property int lastIndex: 0
            property bool playingGame: false

            anchors.fill: parent

            visible: true

            onIndexChanged: function () {
                if (index === -1) {
                    content.clear()
                    return
                }
                content.index = index

                var method = content.depth > 0 ? content.replace : content.push

                if (index === 0) {
                    method(homePage)
                } else if (index === 1) {
                    method(explorePage)
                } else if (index === 2) {
                    method(libraryPage)
                } else if (index === 3) {
                    method(controllersPage)
                } else if (index === 4) {
                    method(settingsPage)
                } else if (index === 5) {
                    method(nowPlayingPage)
                }
            }

            padding: 8

            background: Rectangle {
                color: "transparent"
            }

            ButtonGroup {
                id: buttonGroup
                exclusive: true
            }

            contentItem: Item {
                anchors.fill: parent

                TabBar {
                    id: tabBar
                    width: 300
                    anchors.top: parent.top
                    anchors.horizontalCenter: parent.horizontalCenter
                    currentIndex: mainMenu.index

                    background: Item {
                    }

                    MainMenuNavButton {
                        iconCode: "\ue88a"
                        toolTipText: "Home"
                        ButtonGroup.group: buttonGroup
                        Layout.alignment: Qt.AlignVCenter
                    }

                    MainMenuNavButton {
                        iconCode: "\ue87a"
                        toolTipText: "Explore"
                        ButtonGroup.group: buttonGroup
                        Layout.alignment: Qt.AlignVCenter
                    }

                    MainMenuNavButton {
                        iconCode: "\uf53e"
                        toolTipText: "Library"
                        ButtonGroup.group: buttonGroup
                        Layout.alignment: Qt.AlignVCenter
                    }

                    MainMenuNavButton {
                        iconCode: "\uf135"
                        toolTipText: "Controllers"
                        ButtonGroup.group: buttonGroup
                        Layout.alignment: Qt.AlignVCenter
                    }

                    MainMenuNavButton {
                        iconCode: "\ue8b8"
                        toolTipText: "Settings"
                        ButtonGroup.group: buttonGroup
                        Layout.alignment: Qt.AlignVCenter
                    }

                    onCurrentIndexChanged: {
                        mainMenu.index = tabBar.currentIndex
                    }
                }

                Rectangle {
                    id: line
                    color: "#b3b3b3"
                    height: tabBar.height - 20
                    anchors.verticalCenter: tabBar.verticalCenter
                    width: 1
                    anchors.left: tabBar.right
                    anchors.leftMargin: 8

                    visible: mainMenu.playingGame
                }

                NowPlayingButton {
                    id: nowPlayingButton
                    ButtonGroup.group: buttonGroup
                    anchors.left: line.right
                    anchors.leftMargin: 8
                    height: tabBar.height

                    visible: mainMenu.playingGame

                    onClicked: function () {
                        mainMenu.index = 5
                    }
                }
                // NavigationRail {
                //     id: navRail
                //     fontFamily: Constants.regularFontFamily
                //
                //     currentIndex: mainMenu.index
                //
                //     showNowRunning: mainMenu.playingGame
                //
                //     anchors.left: parent.left
                //     anchors.top: parent.top
                //     anchors.bottom: parent.bottom
                //     width: 200
                //
                //     onIndexSelected: function (index) {
                //         mainMenu.index = index
                //     }
                // }

                // Header {
                //     id: header
                //     height: 28
                //     anchors.top: tabBar.bottom
                //     anchors.topMargin: 6
                //     anchors.left: parent.left
                //     anchors.right: parent.right
                // }

                StackView {
                    id: content
                    property int lastIndex: 0
                    property int index: 0

                    clip: true

                    anchors.topMargin: 6
                    anchors.top: tabBar.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom

                    initialItem: homePage

                    popEnter: Transition {
                    }
                    popExit: Transition {
                    }
                    pushEnter: Transition {
                    }
                    pushExit: Transition {
                    }

                    replaceEnter: Transition {
                        SequentialAnimation {
                            ParallelAnimation {
                                PropertyAnimation {
                                    property: "x"
                                    from: (content.index > content.lastIndex) ? 50 : -50
                                    to: 0
                                    duration: 300
                                    easing.type: Easing.InOutQuad
                                }
                                PropertyAnimation {
                                    property: "opacity"
                                    from: 0
                                    to: 1
                                    duration: 300
                                    easing.type: Easing.InOutQuad
                                }
                            }
                            ScriptAction {
                                script: {
                                    content.lastIndex = content.index
                                }
                            }
                        }
                    }

                    replaceExit: Transition {
                        SequentialAnimation {
                            ParallelAnimation {
                                PropertyAnimation {
                                    property: "x"
                                    from: 0
                                    to: (content.index > content.lastIndex) ? -50 : 50
                                    duration: 300
                                    easing.type: Easing.InOutQuad
                                }
                                PropertyAnimation {
                                    property: "opacity"
                                    from: 1
                                    to: 0
                                    duration: 300
                                    easing.type: Easing.InOutQuad
                                }
                            }
                        }
                    }
                }

                // Footer {
                //     id: footer
                //     height: 8
                //     anchors.bottom: parent.bottom
                //     anchors.left: parent.left
                //     anchors.right: parent.right
                // }
            }
        }

        FirelightDialog {
            id: closeGameDialog
            text: "Are you sure you want \nto close the game?"

            onAccepted: {
                appRoot.state = "notPlayingGame"
            }
        }
    }

    Rectangle {
        id: overlayThing
        anchors.fill: parent
        color: "black"
        opacity: 0

        Behavior on opacity {
            NumberAnimation {
                duration: 300
                easing.type: Easing.InOutQuad
            }

        }
    }

    component NowPlayingButton: TabButton {
        id: myButton

        contentItem: RowLayout {
            spacing: 8
            Text {
                // text: "Settings"
                text: "\ue037"
                color: (myHover.hovered || myButton.checked) ? "white" : "#b3b3b3"
                font.pixelSize: 28
                // font.family: Constants.strongFontFamily
                font.family: Constants.symbolFontFamily
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
            }
            Text {
                text: "Now Playing"
                color: (myHover.hovered || myButton.checked) ? "white" : "#b3b3b3"
                font.pointSize: 12
                // font.family: Constants.strongFontFamily
                font.family: Constants.strongFontFamily
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignLeft
                Layout.fillWidth: true
            }
        }

        scale: checked ? 1.1 : 1.0
        Behavior on scale {
            NumberAnimation {
                duration: 200
                easing.type: Easing.InOutQuad
            }
        }

        background: Rectangle {
            color: "white"
            opacity: myButton.checked ? 0.1 : 0.0
            radius: 4

            anchors.centerIn: parent
            height: myButton.height * (2 / 3)

            Behavior on opacity {
                NumberAnimation {
                    duration: 200
                    easing.type: Easing.InOutQuad
                }
            }
        }

        HoverHandler {
            id: myHover
            cursorShape: Qt.PointingHandCursor
        }
    }

    FirelightDialog {
        id: closeAppDialog
        text: "Are you sure you want \nto exit Firelight?"

        onAccepted: {
            window.close()
        }
    }
}
