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
    color: "black"

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

        Component {
            id: homePage

            Item {
                Rectangle {
                    id: homeTop
                    anchors.top: parent.top
                    anchors.topMargin: 12
                    anchors.leftMargin: 12
                    anchors.left: parent.left
                    anchors.right: parent.right
                    radius: 12
                    height: 120
                    color: "transparent"

                    Text {
                        anchors.fill: parent
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        wrapMode: Text.WordWrap
                        font.pointSize: 24
                        text: "Thanks for trying out Firelight!"
                        font.family: Constants.regularFontFamily
                        color: Constants.colorTestTextActive
                    }
                }

                Rectangle {
                    id: homeLeft
                    anchors.topMargin: 12
                    anchors.top: homeTop.bottom
                    anchors.left: parent.left
                    anchors.leftMargin: 12
                    width: parent.width / 2
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 12
                    radius: 12
                    color: Constants.colorTestSurface

                    Text {
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.margins: 12
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                        wrapMode: Text.WordWrap
                        font.pointSize: 12
                        text: "<p>Seriously, thanks so much :) This is a passion project of mine, and I'm excited to
                         share it with you. This build is a little rough around the edges, but I hope you enjoy it
                         anyway.</p>

                         <p>Here's some of the stuff I'm hoping to get done next:</p>

                         <ul>
                         <li>Save states and rewind</li>
                         <li>Controller profiles</li>
                         <li>Playlists</li>
                         </ul>

                         <p>There's a lot to look forward to, so keep an eye on the Discord for updates and to take
                         part in the discussions!</p>"
                        font.family: Constants.lightFontFamily
                        color: Constants.colorTestTextActive
                    }
                }

                Rectangle {
                    id: homeRight
                    anchors.top: homeTop.bottom
                    anchors.topMargin: 12
                    anchors.left: homeLeft.right
                    anchors.leftMargin: 12
                    anchors.right: parent.right
                    anchors.bottomMargin: 12
                    anchors.bottom: parent.bottom
                    radius: 12
                    color: Constants.colorTestSurface

                    Text {
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.margins: 12
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                        wrapMode: Text.WordWrap
                        font.pointSize: 12
                        text: "<p>I promise there will actually be cool stuff here soon.</p>"
                        font.family: Constants.lightFontFamily
                        color: Constants.colorTestTextActive
                    }
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
                    font.family: Constants.regularFontFamily
                }
            }
        }

        Component {
            id: libraryPage
            LibraryPage {
                id: thisLibraryPage
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
                        thisLibraryPage.nextEntryId = entryId
                        appRoot.state = "notPlayingGame"
                    }
                }

                Connections {
                    target: appRoot

                    function onCustomStateChanged(newState) {
                        if (newState === "notPlayingGame" && thisLibraryPage.nextEntryId !== -1) {
                            var entryId = thisLibraryPage.nextEntryId
                            thisLibraryPage.nextEntryId = -1

                            gameLoader.loadGame(entryId)
                        }
                    }
                }

            }
        }

        Component {
            id: controllersPage
            Item {
                GridView {
                    id: root
                    width: parent.width
                    height: parent.height
                    cellWidth: parent.width
                    cellHeight: parent.height / 4

                    displaced: Transition {
                        NumberAnimation {
                            properties: "x,y"
                            easing.type: Easing.OutQuad
                        }
                    }

                    model: DelegateModel {
                        id: visualModel
                        model: ListModel {
                            id: colorModel
                            ListElement {
                                color: "blue"
                            }
                            ListElement {
                                color: "green"
                            }
                            ListElement {
                                color: "red"
                            }
                            ListElement {
                                color: "yellow"
                            }
                        }

                        component Thing: Rectangle {
                            id: icon
                            required property Item dragParent

                            property int visualIndex: 0
                            width: 72
                            height: 72

                            anchors {
                                horizontalCenter: parent.horizontalCenter
                                verticalCenter: parent.verticalCenter
                            }
                            radius: 12

                            Text {
                                anchors.centerIn: parent
                                color: "white"
                                text: parent.visualIndex
                            }

                            DragHandler {
                                id: dragHandler
                            }

                            Drag.active: dragHandler.active
                            Drag.source: icon
                            Drag.hotSpot.x: 36
                            Drag.hotSpot.y: 36

                            states: [
                                State {
                                    when: dragHandler.active
                                    ParentChange {
                                        target: icon
                                        parent: icon.dragParent
                                    }

                                    AnchorChanges {
                                        target: icon
                                        anchors {
                                            horizontalCenter: undefined
                                            verticalCenter: undefined
                                        }
                                    }
                                }
                            ]
                        }

                        delegate: DropArea {
                            id: delegateRoot
                            required property color color

                            width: 80
                            height: 80

                            onEntered: function (drag) {
                                visualModel.items.move(drag.source.visualIndex, icon2.visualIndex)
                            }

                            property int visualIndex: DelegateModel.itemsIndex

                            Thing {
                                id: icon2
                                dragParent: root
                                visualIndex: delegateRoot.visualIndex
                                color: delegateRoot.color
                            }
                        }
                    }
                }
            }
        }

        Component {
            id: settingsPage
            Item {
                Text {
                    text: "Here's where the Settings menu will go!"
                    anchors.centerIn: parent
                    color: Constants.colorTestTextActive
                    font.pointSize: 16
                    font.family: Constants.regularFontFamily
                }
            }
        }

        Component {
            id: nowPlayingPage
            Item {
                Rectangle {
                    anchors.left: parent.left
                    width: 1
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    color: Constants.colorTestText
                }

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

                    FirelightMenuItem {
                        labelText: "Resume Game"
                        labelIcon: "\uf7d0"
                        height: 40
                        width: 200
                        checkable: false

                        onClicked: function () {
                            appRoot.state = "playingGame"
                        }
                    }

                    FirelightMenuItem {
                        labelText: "Restart Game"
                        labelIcon: "\uf053"
                        height: 40
                        width: 200
                        checkable: false

                        onClicked: function () {
                            emulator.resetGame()
                            appRoot.state = "playingGame"
                        }
                    }

                    FirelightMenuItem {
                        labelText: "Quit Game"
                        labelIcon: "\ue5cd"
                        height: 40
                        width: 200
                        checkable: false

                        onClicked: function () {
                            closeGameDialog.open()
                        }
                    }

                    Rectangle {
                        height: 1
                        width: parent.width / 2
                        color: Constants.colorTestTextMuted
                    }

                    Text {
                        text: "This thing doesn't remember your setting when you leave this menu and go back in but it's fine for now."
                        color: Constants.colorTestTextActive
                        font.pointSize: 10
                        font.family: Constants.regularFontFamily
                    }

                    SettingsItem {
                        label: "Picture Mode"
                        width: parent.width / 2
                        height: 50
                        thing: Component {
                            ComboBox {
                                model: ["Maintain Aspect Ratio", "Original", "Stretch"]
                                onActivated: function (index) {
                                    switch (index) {
                                        case 0:
                                            emulator.setPictureMode("aspect-ratio")
                                            break
                                        case 1:
                                            emulator.setPictureMode("original")
                                            break
                                        case 2:
                                            emulator.setPictureMode("stretched")
                                            break
                                    }
                                }
                            }
                        }
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
                dialog.open()
            }
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

            padding: 12

            background: Rectangle {
                color: "transparent"
            }

            contentItem: Item {
                NavigationRail {
                    id: navRail
                    fontFamily: Constants.regularFontFamily

                    currentIndex: mainMenu.index

                    showNowRunning: mainMenu.playingGame

                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    width: 200

                    onIndexSelected: function (index) {
                        mainMenu.index = index
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
}