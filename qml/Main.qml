import QtQuick
import QtQuick.Controls
// import QtQuick.Controls.Material
import QtQuick.Dialogs
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Effects
import Firelight 1.0
import FirelightStyle 1.0

ApplicationWindow {
    id: window
    width: 1280
    height: 720
    visible: true
    title: qsTr("Firelight")

    Rectangle {
        id: appRoot
        anchors.fill: parent
        focus: true
        color: Constants.colorTestBackground

        signal customStateChanged(string newState)

        state: "notPlayingGame"

        Keys.onEscapePressed: function () {
            if (state === "gameSuspended") {
                state = "playingGame"
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
                    ScriptAction {
                        script: {
                            emulator.pauseGame()
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
                    PropertyAnimation {
                        target: emulator
                        property: "opacity"
                        from: 1
                        to: 0
                        duration: 300
                        easing.type: Easing.InOutQuad
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

        Dialog {
            id: waitingDialog
            modal: true
            // title: "doing a thing"
            standardButtons: Dialog.NoButton

            parent: Overlay.overlay
            anchors.centerIn: parent

            background: Rectangle {
                color: Constants.colorTestSurfaceContainerLowest
                radius: 12
                implicitWidth: 300
                implicitHeight: 200
            }

            contentItem: Item {
                Text {
                    anchors.centerIn: parent
                    color: Constants.colorTestText
                    text: "Please wait while I do the thing"
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                    font.pointSize: 14
                }
            }
            //
            // background: Rectangle {
            //     implicitWidth: 200
            //     implicitHeight: 200
            //     border.color: "#444"
            // }

            Overlay.modal: Rectangle {
                id: backdropDim

                color: "black"
                anchors.fill: parent
                opacity: 0.4

                Behavior on opacity {
                    NumberAnimation {
                        duration: 200
                    }
                }

            }

            enter: Transition {
                NumberAnimation {
                    property: "opacity";
                    from: 0.0;
                    to: 1.0
                    duration: 200
                }
                NumberAnimation {
                    property: "scale";
                    from: 0.9;
                    to: 1.0
                    duration: 200
                }
                // NumberAnimation {
                //     property: "y";
                //     from: (waitingDialog.parent.height / 2) + (waitingDialog.height / 2);
                //     to: (waitingDialog.parent.height / 2) - (waitingDialog.height / 2);
                //     duration: 200
                // }
            }

            exit: Transition {
                NumberAnimation {
                    property: "opacity";
                    from: 1.0;
                    to: 0.0;
                    duration: 200
                }
                NumberAnimation {
                    property: "scale";
                    from: 1.0;
                    to: 0.9
                    duration: 200
                }
            }
        }

        Component {
            id: homePage

            Item {
                Text {
                    text: "Here's where the Home menu will go!"
                    anchors.centerIn: parent
                    color: Constants.colorTestTextMuted
                    font.pointSize: 16
                    font.family: localFont.name
                }
            }
        }


        Component {
            id: explorePage

            Item {
                Rectangle {
                    radius: 12
                    anchors.fill: parent
                    color: Constants.colorTestSurface
                }

                Text {
                    text: "Here's where the Explore menu will go!"
                    anchors.centerIn: parent
                    color: Constants.colorTestTextMuted
                    font.pointSize: 16
                    font.family: localFont.name
                }
            }
        }

        Component {
            id: libraryPage
            LibraryPage {
                id: thisLibraryPage
                property int nextEntryId

                fontFamilyName: localFont.name
                onEntryClicked: function (entryId) {
                    // gameLoader.loadGame(entryId)
                    // fullPane.loadGame(entryId)
                    if (emulator.isRunning()) {
                        thisLibraryPage.nextEntryId = entryId
                        appRoot.state = "notPlayingGame"
                    } else {
                        gameLoader.loadGame(entryId)
                    }
                }

                Connections {
                    target: appRoot

                    function onCustomStateChanged(newState) {
                        if (newState === "notPlayingGame" && thisLibraryPage.nextEntryId !== -1) {
                            var entryId = thisLibraryPage.nextEntryId
                            thisLibraryPage.nextEntryId = -1

                            console.log("GONNA LOAD: " + entryId)

                            gameLoader.loadGame(entryId)
                        }
                    }
                }

            }
        }

        Component {
            id: controllersPage
            Item {
                Rectangle {
                    radius: 12
                    anchors.fill: parent
                    color: Constants.colorTestSurface
                }

                Text {
                    text: "Here's where the Controllers menu will go!"
                    anchors.centerIn: parent
                    color: Constants.colorTestTextMuted
                    font.pointSize: 16
                    font.family: localFont.name
                }
            }
        }

        Component {
            id: settingsPage
            Item {
                Rectangle {
                    radius: 12
                    anchors.fill: parent
                    color: Constants.colorTestSurface
                }

                Text {
                    text: "Here's where the Settings menu will go!"
                    anchors.centerIn: parent
                    color: Constants.colorTestTextMuted
                    font.pointSize: 16
                    font.family: localFont.name
                }
            }
        }

        Component {
            id: nowPlayingPage
            Item {
                Text {
                    text: "Here's where the Now Playing menu will go!"
                    anchors.centerIn: parent
                    color: Constants.colorTestTextMuted
                    font.pointSize: 16
                    font.family: localFont.name
                }
            }
        }

        GameLoader {
            id: gameLoader

            onGameLoaded: function (entryId, romData, saveData, corePath) {
                emulator.loadTheThing(entryId, romData, saveData, corePath)
            }

            onGameLoadFailedOrphanedPatch: function (entryId) {
                dialog.open()
            }
        }

        EmulatorPage {
            id: emulator
            anchors.fill: parent
            visible: false

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
        }

        Pane {
            id: mainMenu
            property int index: 0
            property int lastIndex: 0
            property bool playingGame: false

            anchors.fill: parent

            visible: true

            onIndexChanged: function () {
                console.log("CHANGING INDEX: " + index)
                if (index === -1) {
                    console.log("its negative")
                    content.clear()
                    return
                }
                content.index = index
                //
                // console.log("current index: " + content.currentIndex + " new index: " + content.newIndex)
                // if (content.newIndex === index || content.currentIndex === index) {
                //     return
                // }

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

            padding: 12

            background: Rectangle {
                color: "transparent"
            }

            contentItem: Item {
                NavigationRail {
                    id: navRail
                    fontFamily: localFont.name

                    currentIndex: mainMenu.index

                    showNowRunning: mainMenu.playingGame

                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    width: 200

                    onIndexSelected: function (index) {
                        mainMenu.index = index
                    }

                    onCloseGameClicked: {
                        closeGameDialog.open()
                    }
                }

                Header {
                    id: header
                    height: 28
                    anchors.top: parent.top
                    anchors.left: navRail.right
                    anchors.leftMargin: 12
                    anchors.right: parent.right
                }

                StackView {
                    id: content
                    property int lastIndex: 0
                    property int index: 0

                    clip: true

                    anchors.topMargin: 12
                    anchors.top: header.bottom
                    anchors.left: navRail.right
                    anchors.leftMargin: 12
                    anchors.right: parent.right
                    anchors.bottom: footer.top
                    anchors.bottomMargin: 12

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
                            ScriptAction {
                                script: {
                                    console.log("REPLACING: " + content.lastIndex + " -> " + content.index)
                                }
                            }
                            ParallelAnimation {
                                PropertyAnimation {
                                    property: "y"
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
                                    property: "y"
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

                Footer {
                    id: footer
                    height: 8
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                }
            }
        }

        Dialog {
            id: closeGameDialog

            title: "Exit game?"
            Label {
                text: "Are you sure you want to close the game?"
            }
            standardButtons: Dialog.Ok | Dialog.Cancel
            onAccepted: {
                appRoot.state = "notPlayingGame"
                // closeGameAnimation.startWith(outPage, inPage)
            }
        }

        Dialog {
            id: dialog
            title: "Game Load Failed"
            Label {
                text: "The game you tried to load is missing or incomplete. Please try another game."
            }
            standardButtons: Dialog.Ok
            onAccepted: {
                console.log("accepted")
            }
        }


        FontLoader {
            id: localFont
            source: "qrc:/fonts/lexend"
        }

        FontLoader {
            id: lexendLight
            source: "qrc:/fonts/lexend-light"
        }

    }

    Rectangle {
        id: overlayThing
        anchors.fill: parent
        color: "black"
        opacity: 0
    }


}
