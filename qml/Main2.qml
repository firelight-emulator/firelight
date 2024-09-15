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

    // font.family: "Segoe UI"

    // flags: Qt.FramelessWindowHint

    visible: true
    visibility: GeneralSettings.fullscreen ? Window.FullScreen : Window.Windowed

    title: qsTr("Firelight")

    background: Rectangle {
        color: ColorPalette.neutral1000
    }

    Connections {
        target: InputMethodManager

        function onInputMethodChanged() {
            console.log("Input method changed: ", InputMethodManager.usingMouse)
        }
    }

    Rectangle {
        id: activeFocusHighlight
        color: "transparent"
        border.color: "lightblue"
        border.width: 3
        radius: 2
        visible: !InputMethodManager.usingMouse && (parent.hasOwnProperty('showGlobalCursor') && parent.showGlobalCursor)
        parent: window.activeFocusItem ? window.activeFocusItem : window
        anchors.fill: parent
        anchors.margins: -3

        Behavior on width {
            NumberAnimation {
                duration: 100
                easing.type: Easing.InOutQuad
            }
        }

        Behavior on height {
            NumberAnimation {
                duration: 100
                easing.type: Easing.InOutQuad
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
                    let section = parts.length > 1 ? parts[1] : "library"
                    screenStack.push(settingsScreen, {section: section})
                }
            }
        }
    }

    onActiveFocusItemChanged: {
        if (activeFocusItem === null) {
            return
        }

        activeFocusItem.grabToImage(function (result) {
            debugImage.width = result.width
            debugImage.height = result.height
            debugImage.source = result.url
        })
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
        id: emulatorScreen
        EmulatorScreen {

            // Component.onCompleted: {
            //     console.log("Graphics Info:")
            //     console.log("  API:", emulatorScreen.GraphicsInfo.api)
            //     console.log("  Major Version:", emulatorScreen.GraphicsInfo.majorVersion)
            //     console.log("  Minor Version:", emulatorScreen.GraphicsInfo.minorVersion)
            //     console.log("  Profile:", emulatorScreen.GraphicsInfo.profile)
            // }

            onGameAboutToStop: function () {
                screenStack.pushItem(homeScreen, {}, StackView.PushTransition)
            }

        }
    }

    // Rectangle {
    //     id: cursor
    //     parent: window.activeFocusItem
    //     anchors.fill: parent
    //     anchors.margins: -2
    //     color: "transparent"
    //     border.color: "lightblue"
    //     border.width: 3
    //     radius: 4
    // }

    StackView {
        id: screenStack
        anchors.fill: parent
        focus: true

        Component.onCompleted: {
            screenStack.pushItems([emulatorScreen, homeScreen])
        }

        Keys.onPressed: function (event) {
            if (event.key === Qt.Key_F12) {
                debugWindow.visible = !debugWindow.visible
            }

            if (event.key === Qt.Key_F11) {
                console.log("Going offline")
                achievement_manager.setOnlineForTesting(false)
            }

            if (event.key === Qt.Key_F10) {
                console.log("Going online")
                achievement_manager.setOnlineForTesting(true)
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

    SequentialAnimation {
        id: gameStartAnimation
        running: false

        property int entryId: -1

        PropertyAction {
            target: overlay
            property: "opacity"
            value: 0
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
                screenStack.popCurrentItem(StackView.Immediate)
                screenStack.currentItem.loadGame(gameStartAnimation.entryId)
                // emulatorScreen
            }
        }

        PauseAnimation {
            duration: 1000
        }

        PropertyAction {
            target: overlay
            property: "visible"
            value: false
        }
    }

    Component {
        id: homeScreen

        HomeScreen {
            onStartGame: function (entryId) {
                gameStartAnimation.entryId = entryId
                gameStartAnimation.running = true
            }
        }
    }

    Component {
        id: settingsScreen

        SettingsScreen {
        }
    }
}
