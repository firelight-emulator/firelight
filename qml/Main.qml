import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Window
import QtQuick.Layouts 1.0
import QtQuick.Effects
import Firelight 1.0
import FirelightStyle 1.0

ApplicationWindow {
    id: window

    width: 1280
    height: 720

    minimumHeight: 720
    minimumWidth: 1280

    visible: true

    title: qsTr("Firelight")

    background: Rectangle {
        color: "#1a1b1e"
    }

    Component {
        id: libraryPage
        LibraryPage {
            property bool topLevel: true
            property string topLevelName: "library"

            onEntryClicked: function (id) {
                gameLoader.loadGame(id)
            }
        }
    }

    Component {
        id: modsPage
        DiscoverPage {
            property bool topLevel: true
            property string topLevelName: "mods"
        }
    }


    Component {
        id: controllerPage
        ControllersPage {
            property bool topLevel: true
            property string topLevelName: "controllers"
        }
    }

    Component {
        id: settingsPage
        SettingsPage {
            property bool topLevel: true
            property string topLevelName: "settings"
        }
    }

    Component {
        id: nowPlayingPage
        NowPlayingPage {
            id: me
            property bool topLevel: true
            property string topLevelName: "nowPlaying"

            onBackToMainMenuPressed: function () {
                stackView.push(mainMenu)
            }

            onResumeGamePressed: function () {
                emulatorStack.pop()
            }

            onRestartGamePressed: function () {
                emulator.resetGame()
                emulatorStack.pop()
            }

            onCloseGamePressed: function () {
                closeGameAnimation.start()
            }
        }
    }

    SequentialAnimation {
        id: closeGameAnimation
        ScriptAction {
            script: {
                stackView.push(mainMenu)
            }
        }
        ScriptAction {
            script: {
                emulator.stopEmulation()
            }
        }
        ScriptAction {
            script: {
                emulatorStack.pop()
            }
        }
    }

    Component {
        id: mainMenu

        Item {
            Pane {
                id: drawer
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                width: 250
                background: Rectangle {
                    color: "#101114"
                }
                padding: 4
                KeyNavigation.right: stackview

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 0

                    Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 10
                    }

                    Text {
                        text: "Firelight"
                        opacity: parent.width > 48 ? 1 : 0
                        color: "#dadada"
                        font.pointSize: 12
                        font.weight: Font.DemiBold
                        font.family: Constants.regularFontFamily
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Text {
                        text: "alpha (0.4.0a)"
                        opacity: parent.width > 48 ? 1 : 0
                        color: "#dadada"
                        font.pointSize: 8
                        font.weight: Font.DemiBold
                        font.family: Constants.regularFontFamily
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Text {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 12
                    }
                    NavMenuButton {
                        id: homeNavButton
                        KeyNavigation.down: libraryNavButton
                        labelText: "Home"
                        labelIcon: "\ue88a"
                        Layout.preferredWidth: parent.width
                        Layout.preferredHeight: 48
                        enabled: false

                        checked: stackview.topLevelName === "home"
                    }
                    NavMenuButton {
                        id: modNavButton
                        KeyNavigation.down: controllersNavButton
                        labelText: "Discover"
                        labelIcon: "\ue87a"
                        Layout.preferredWidth: parent.width
                        Layout.preferredHeight: 48

                        checked: stackview.topLevelName === "mods"

                        onToggled: function () {
                            stackview.push(modsPage)
                        }
                    }
                    NavMenuButton {
                        id: libraryNavButton
                        KeyNavigation.down: modNavButton
                        labelText: "My Library"
                        labelIcon: "\uf53e"
                        Layout.preferredWidth: parent.width
                        Layout.preferredHeight: 48

                        checked: stackview.topLevelName === "library"

                        onToggled: function () {
                            stackview.push(libraryPage)
                        }
                    }
                    NavMenuButton {
                        id: controllersNavButton
                        // KeyNavigation.down: nowPlayingNavButton
                        labelText: "Controllers"
                        labelIcon: "\uf135"
                        Layout.preferredWidth: parent.width
                        Layout.preferredHeight: 48

                        enabled: false

                        checked: stackview.topLevelName === "controllers"

                        onToggled: function () {
                            stackview.push(controllerPage)
                        }
                    }
                    // Rectangle {
                    //     Layout.topMargin: 8
                    //     Layout.bottomMargin: 8
                    //     Layout.preferredWidth: parent.width
                    //     Layout.preferredHeight: 1
                    //     opacity: 0.3
                    //     color: "#dadada"
                    // }
                    // NavMenuButton {
                    //     id: nowPlayingNavButton
                    //     KeyNavigation.down: settingsNavButton
                    //     labelText: "Now Playing"
                    //     labelIcon: "\ue037"
                    //     Layout.preferredWidth: parent.width
                    //     Layout.preferredHeight: 48
                    //
                    //     onToggled: function () {
                    //         stackview.replace(nowPlayingPage)
                    //     }
                    // }
                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }
                    NavMenuButton {
                        id: nowPlayingNavButton
                        KeyNavigation.down: settingsNavButton
                        labelText: "Back to game"
                        labelIcon: "\ue037"
                        Layout.preferredWidth: parent.width
                        Layout.preferredHeight: 48

                        visible: emulator.running

                        checkable: false

                        onClicked: function () {
                            stackView.pop()
                            // stackview.push(nowPlayingPage)
                        }
                    }
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.topMargin: 8
                        Layout.bottomMargin: 8
                        Layout.preferredHeight: 1
                        color: "#404143"
                    }
                    NavMenuButton {
                        id: settingsNavButton
                        labelText: "Settings"
                        labelIcon: "\ue8b8"
                        Layout.preferredWidth: parent.width
                        Layout.preferredHeight: 48

                        checked: stackview.topLevelName === "settings"

                        onToggled: function () {
                            stackview.push(settingsPage)
                        }
                    }

                    NavMenuButton {
                        labelText: "Profile"
                        labelIcon: "\ue853"
                        Layout.preferredWidth: parent.width
                        Layout.preferredHeight: 48

                        checked: stackview.topLevelName === "profile"

                        enabled: false
                    }
                }
            }

            Pane {
                id: rightSide

                anchors.top: parent.top
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.left: drawer.right

                background: Item {
                }

                Pane {
                    width: parent.width
                    height: 48

                    z: 2

                    background: Item {
                    }

                    Button {
                        id: melol
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        // horizontalPadding: 12
                        // verticalPadding: 8

                        enabled: stackview.depth > 1

                        hoverEnabled: false

                        HoverHandler {
                            id: myHover
                            cursorShape: melol.enabled ? Qt.PointingHandCursor : Qt.ForbiddenCursor
                        }

                        background: Rectangle {
                            color: enabled ? myHover.hovered ? "#4e535b" : "#3e434b" : "#3e434b"
                            radius: height / 2
                            // border.color: "#7d848c"
                        }

                        contentItem: Text {
                            text: "\ue5c4"
                            color: enabled ? "white" : "#7d848c"
                            font.pointSize: 11
                            font.family: Constants.symbolFontFamily
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            stackview.pop()
                        }
                    }
                }

                StackView {
                    id: stackview
                    anchors.fill: parent

                    property string topLevelName: ""

                    onCurrentItemChanged: {
                        if (currentItem) {
                            let top = stackview.find(function (item, index) {
                                return item.topLevel === true
                            })

                            stackview.topLevelName = top ? top.topLevelName : ""
                        }
                    }

                    initialItem: libraryPage

                    pushEnter: Transition {
                        // PropertyAnimation {
                        //     property: "opacity"
                        //     from: 0
                        //     to: 1
                        //     duration: 200
                        // }
                    }
                    pushExit: Transition {
                        // PropertyAnimation {
                        //     property: "opacity"
                        //     from: 1
                        //     to: 0
                        //     duration: 200
                        // }
                    }
                    popEnter: Transition {
                        // PropertyAnimation {
                        //     property: "opacity"
                        //     from: 0
                        //     to: 1
                        //     duration: 200
                        // }
                    }
                    popExit: Transition {
                        // PropertyAnimation {
                        //     property: "opacity"
                        //     from: 1
                        //     to: 0
                        //     duration: 200
                        // }
                    }
                    replaceEnter: Transition {
                        // ParallelAnimation {
                        //     PropertyAnimation {
                        //         property: "opacity"
                        //         from: 0
                        //         to: 1
                        //         duration: 400
                        //     }
                        //     PropertyAnimation {
                        //         property: "x"
                        //         from: 20
                        //         to: 0
                        //         duration: 250
                        //     }
                        // }
                    }
                    replaceExit: Transition {
                    }
                }
            }
        }
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
                emulator.startEmulation()
                // emulator.resumeGame()
            }
        }
    }

    GameLoader {
        id: gameLoader

        onGameLoaded: function (entryId, romData, saveData, corePath) {
            emulator.loadTheThing(entryId, romData, saveData, corePath)
            overlayFadeIn.start()
        }

        onGameLoadFailedOrphanedPatch: function (entryId) {
            patchClickedDialog.open()
        }
    }

    EmulatorPage {
        id: emulator
        visible: false

        states: [
            State {
                name: "stopped"
            },
            State {
                name: "suspended"
                PropertyChanges {
                    target: emulatorDimmer
                    opacity: 0.4
                }
                PropertyChanges {
                    emulator {
                        layer.enabled: true
                        blurAmount: 1
                    }
                }
            },
            State {
                name: "running"
                PropertyChanges {
                    target: emulatorDimmer
                    opacity: 0
                }
                PropertyChanges {
                    emulator {
                        layer.enabled: false
                        blurAmount: 0
                    }
                }
            }
        ]

        transitions: [
            Transition {
                from: "*"
                to: "suspended"
                SequentialAnimation {
                    ScriptAction {
                        script: {
                            emulator.pauseGame()
                        }
                    }
                    PropertyAction {
                        target: emulator
                        property: "layer.enabled"
                        value: true
                    }
                    ParallelAnimation {
                        NumberAnimation {
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
                from: "*"
                to: "running"
                SequentialAnimation {
                    ParallelAnimation {
                        NumberAnimation {
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
                        property: "layer.enabled"
                        value: false
                    }

                    ScriptAction {
                        script: {
                            emulator.resumeGame()
                        }

                    }
                }
            }
        ]

        StackView.visible: true

        StackView.onActivating: {
            state = "running"
        }

        StackView.onDeactivating: {
            state = "suspended"
        }

        property double blurAmount: 0

        Behavior on blurAmount {
            NumberAnimation {
                duration: 250
                easing.type: Easing.InOutQuad
            }
        }

        layer.enabled: false
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
                if (emulator.StackView.view.currentItem === emulator) {
                    emulator.pauseGame()
                }
            }

            function onWindowResizeFinished() {
                if (emulator.StackView.view.currentItem === emulator) {
                    emulator.resumeGame()
                }
            }
        }
    }

    StackView {
        id: emulatorStack
        visible: false

        initialItem: emulator

        Keys.onEscapePressed: {
            if (emulatorStack.currentItem === emulator) {
                // emulatorStack.pop()
                emulatorStack.push(nowPlayingPage)
            } else {
                emulatorStack.pop()
                // emulatorStack.push(mainMenu)
            }
        }

        property bool suspended: false
        property bool running: false

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

    StackView {
        id: stackView
        anchors.fill: parent

        Component.onCompleted: stackView.push([emulatorStack, mainMenu])

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
