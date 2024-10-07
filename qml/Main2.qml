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

    // background: Rectangle {
    //     color: ColorPalette.neutral1000
    // }

    background: Rectangle {
        height: window.height
        width: window.width
        color: ColorPalette.neutral1000
        Image {
            id: mainBackground
            source: "https://static.nsidr.com/images/inline_images/inline-4-4205-2.jpg"
            fillMode: Image.PreserveAspectCrop
            anchors.fill: parent
            layer.enabled: true
            layer.effect: MultiEffect {
                source: mainBackground
                anchors.fill: mainBackground
                blurEnabled: true
                blurMultiplier: 1.0
                blurMax: 64
                blur: 0.5
            }
        }

        Rectangle {
            anchors.fill: parent
            color: "black"
            opacity: 0.5
        }

        Rectangle {
            anchors.topMargin: -70
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 200
            gradient: Gradient {
                GradientStop {
                    position: 0.0; color: "black"
                }
                GradientStop {
                    position: 1.0; color: "transparent"
                }
            }
        }
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
        border.width: 2
        radius: 2
        visible: !InputMethodManager.usingMouse && parent !== null
        anchors.fill: parent
        anchors.margins: -4

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
                if (window.activeFocusItem && window.activeFocusItem.hasOwnProperty('showGlobalCursor') && window.activeFocusItem.showGlobalCursor) {
                    activeFocusHighlight.parent = window.activeFocusItem
                    if (window.activeFocusItem.hasOwnProperty('background')) {
                        activeFocusHighlight.radius = window.activeFocusItem.background.radius + 4
                    }
                } else {
                    activeFocusHighlight.parent = null
                }
            }
        }

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
        property string contentHash

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
                screenStack.currentItem.loadGame(gameStartAnimation.entryId, gameStartAnimation.contentHash)
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
            onStartGame: function (entryId, hash) {
                gameStartAnimation.entryId = entryId
                gameStartAnimation.contentHash = hash
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
