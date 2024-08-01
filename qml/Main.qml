import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Window
import QtQml.Models
import QtQuick.Layouts 1.0
import QtQuick.Effects
import Firelight 1.0

ApplicationWindow {
    id: window
    objectName: "Application Window"

    width: 1280
    height: 720

    // flags: Qt.FramelessWindowHint

    visible: true
    visibility: GeneralSettings.fullscreen ? Window.FullScreen : Window.Windowed

    title: qsTr("Firelight")

    background: Rectangle {
        color: "#1a1b1e"
    }

    function getStuff(item, depth): string {
        if (!item) {
            return ""
        }

        let str = ""

        str += " ".repeat(depth) + item.objectName + " " + item.height + "\n"

        // if (item.children.length === 0) {
        //     return
        // }

        // console.log("num children: " + item.children.length)

        for (let i = 0; i < item.children.length; i++) {
            let child = item.children[i]
            if (!child) {
                continue
            }
            // str += " ".repeat(depth) + child.objectName + " " + child.id + "\n"
            str += getStuff(child, depth + 1)
        }

        return str
    }

    Window {
        id: debugWindow
        objectName: "Debug Window"
        width: 400
        height: 400
        visible: false


        property bool locked: false

        // ListView {
        //     anchors.fill: parent
        //     model: window.contentItem.children
        //     delegate: ListView {
        //         required property var modelData
        //         width: parent.width
        //         header: Text {
        //             width: parent.width
        //             height: 20
        //             // text: modelData.objectName
        //             text: "Item"
        //         }
        //         model: modelData.children
        //         delegate: Text {
        //             width: parent.width
        //             height: 20
        //
        //             required property var modelData
        //             text: modelData.height
        //         }
        //     }
        // }

        ColumnLayout {
            anchors.fill: parent

            Text {
                Layout.fillWidth: true
                text: debugWindow.locked ? "Locked (F11 to unlock)" : "Unlocked (F11 to lock)"
                font.family: Constants.regularFontFamily
                font.weight: Font.DemiBold
                font.pointSize: 11
                color: "black"
            }

            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                contentHeight: debugText.height
                contentWidth: debugText.width
                Text {
                    id: debugText
                    text: ""
                }
            }
        }

        Component.onCompleted: {
            // debugText.text = getStuff(window.activeFocusItem, 0)
        }
    }

    onActiveFocusItemChanged: function () {
        if (!debugWindow.visible || debugWindow.locked) {
            return
        }

        // for (let key in window.activeFocusControl) {
        //     console.log(key + ": " + window.activeFocusControl[key])
        // }

        if (window.activeFocusItem && window.activeFocusItem.parent) {
            debugText.text = "Active Focus Item: " + window.activeFocusItem.objectName + "\n  parent: "
                + window.activeFocusItem["parent"]
        }
        // debugText.text = getStuff(window.activeFocusItem, 0)
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
            objectName: "Home Screen"
            showNowPlayingButton: emulatorScreen.emulatorIsRunning
        }
    }

    Component {
        id: settingsScreen
        SettingsScreen {
            objectName: "Settings Screen"
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
        objectName: "Emulator Screen"
        onGameReady: function () {
            overlayFadeIn.start()
        }
    }

    Rectangle {
        objectName: "Active focus highlight"
        color: "transparent"
        border.color: "red"
        anchors.fill: parent
        parent: window.activeFocusItem
    }

    GameLaunchPopup {
        objectName: "Game Launch Popup"
        id: gameLaunchPopup

        Connections {
            target: achievement_manager

            function onGameLoadSucceeded(imageUrl, title, numEarned, numTotal) {
                gameLaunchPopup.openWith(imageUrl, title, numEarned, numTotal, achievement_manager.defaultToHardcore)
            }
        }
    }

    Component {
        id: libraryPage
        LibraryPage {
            objectName: "Library Page"
            property bool topLevel: true
            property string topLevelName: "library"

            onEntryClicked: function (id) {
                emulatorScreen.loadGame(id)
            }
        }
    }

    SequentialAnimation {
        id: closeGameAnimation
        objectName: "Close Game Animation"
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
    }

    SequentialAnimation {
        id: overlayFadeIn
        objectName: "Start Game Animation"
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

    StackView {
        id: stackView
        objectName: "Screen Stack View"

        anchors.fill: parent
        initialItem: emulatorScreen
        focus: true

        Component.onCompleted: stackView.push(homeScreen, {}, StackView.Immediate)

        Keys.onReleased: function (event) {
            if (event.key === Qt.Key_F12) {
                debugWindow.visible = !debugWindow.visible
                event.accepted = true
            } else if (event.key === Qt.Key_F11) {
                debugWindow.locked = !debugWindow.locked
                event.accepted = true
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
        objectName: "Screen Overlay"
        anchors.fill: parent
        color: "black"
        visible: false
    }
}
