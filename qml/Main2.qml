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
        //
        // Rectangle {
        //     anchors.fill: parent
        //     color: "black"
        //     opacity: 0.5
        // }
        //
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
                    activeFocusHighlight.parent = item
                    if (item.hasOwnProperty('background')) {
                        activeFocusHighlight.radius = item.background.radius + 4
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
                    let section = parts.length > 1 ? parts[1] : "library"
                    screenStack.push(settingsScreen, {section: section})
                }
            }
        }

        function onDidSomethingWith(thing, params) {
            console.log(thing, params)
            // screenStack.forceActiveFocus()
            // quickMenuBar.close()
            screenStack.pushItem(profileEditor, params, StackView.PushTransition)
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

            focus: true

            // Component.onCompleted: {
            //     console.log("Graphics Info:")
            //     console.log("  API:", emulatorScreen.GraphicsInfo.api)
            //     console.log("  Major Version:", emulatorScreen.GraphicsInfo.majorVersion)
            //     console.log("  Minor Version:", emulatorScreen.GraphicsInfo.minorVersion)
            //     console.log("  Profile:", emulatorScreen.GraphicsInfo.profile)
            // }

            onGameAboutToStop: function () {
                screenStack.popCurrentItem()
            }

            onGameStopped: function () {
                emulatorLoader.active = false
            }
        }
    }

    Loader {
        id: emulatorLoader
        property var gameData
        property var saveData
        property var corePath
        property var contentHash
        property var saveSlotNumber
        property var platformId
        property var contentPath

        active: false
        sourceComponent: emulatorScreen

        StackView.onActivating: {
            active = true
        }

        onLoaded: function () {
            emulatorLoader.item.startGame(gameData, saveData, corePath, contentHash, saveSlotNumber, platformId, contentPath)
        }
    }

    StackView {
        id: screenStack
        anchors.fill: parent
        focus: true
        initialItem: homeScreen

        onCurrentItemChanged: function() {
            quickMenuBar.close()
        }

        Keys.onEnterPressed: function(event) {
            console.log("showing")
            quickMenuBar.visible = true
            console.log("showed")
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
                console.log("showing")
                quickMenuBar.visible = true
                console.log("showed")
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
    }

    Component {
        id: controllerPage
        ControllersPage {
            property bool topLevel: true
            property string topLevelName: "controllers"
            property string pageTitle: "Controllers"
        }
    }

    Popup {
        id: quickMenuBar
        parent: Overlay.overlay
        modal: true
        focus: true

        padding: 16

        width: parent.width
        height: parent.height
        // height: 72
        // y: parent.height - height
        // width: parent.width

        onAboutToShow: function () {
            sfx_player.play("quickopen")
            menuBar.forceActiveFocus()
            // quickMenuStack.clear()
        }

        onClosed: function() {
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

            Keys.onEscapePressed: function(event) {
                sfx_player.play("quickclose")
                quickMenuBar.close()
            }

            Keys.onBackPressed: function(event) {
                sfx_player.play("quickclose")
                quickMenuBar.close()
            }

            StackView {
                id: quickMenuStack
                enabled: depth > 0
                height: 400
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: menuBar.top

                onActiveFocusChanged: function() {
                    if (!quickMenuStack.activeFocus) {
                        quickMenuStack.clear(StackView.PopTransition)
                        menuBar.forceActiveFocus()
                    }
                }

                Keys.onBackPressed: function(event) {
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

            ListView {
                id: menuBar
                KeyNavigation.up: quickMenuStack
                width: (48 * 5) + 6 * 42
                focus: true
                height: 60
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                orientation: ListView.Horizontal
                currentIndex: 0

                onCurrentIndexChanged: function() {
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
                        icon: "\uf53e"; label: "Library"
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
                    focus: true
                    required property var model
                    required property var index
                    tooltipLabel: model.label
                    tooltipOnTop: true
                    circle: true
                    flat: true
                    iconCode: model.icon

                    Keys.onUpPressed: function(event) {
                        if (model.label === "Controllers") {
                            if (quickMenuStack.depth === 1) {
                                quickMenuStack.popCurrentItem(StackView.PopTransition)
                            } else {
                                quickMenuStack.pushItem(controllerPage, {}, StackView.PushTransition)
                                quickMenuStack.forceActiveFocus()
                            }
                        }

                        if (model.label === "Power") {
                            closeAppConfirmationDialog.open()
                        }
                    }

                    onClicked: function() {
                        if (model.label === "Settings") {
                            Router.navigateTo("settings")
                            quickMenuBar.close()
                        }

                        if (model.label === "Controllers") {
                            if (quickMenuStack.depth === 1) {
                                quickMenuStack.popCurrentItem(StackView.PopTransition)
                            } else {
                                quickMenuStack.pushItem(controllerPage, {}, StackView.PushTransition)
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
                GameLoader.approve()
            }
        }
    }

    SequentialAnimation {
        id: gameLoadedAnimation
        running: false

        property var gameData
        property var saveData
        property var corePath
        property var contentHash
        property var saveSlotNumber
        property var platformId
        property var contentPath


        ScriptAction {
            script: {
                // screenStack.popCurrentItem(StackView.Immediate)
                // screenStack.currentItem.startGame(gameLoadedAnimation.gameData, gameLoadedAnimation.saveData, gameLoadedAnimation.corePath, gameLoadedAnimation.contentHash, gameLoadedAnimation.saveSlotNumber, gameLoadedAnimation.platformId, gameLoadedAnimation.contentPath)
                screenStack.pushItem(emulatorLoader, {
                    gameData: gameLoadedAnimation.gameData,
                    saveData: gameLoadedAnimation.saveData,
                    corePath: gameLoadedAnimation.corePath,
                    contentHash: gameLoadedAnimation.contentHash,
                    saveSlotNumber: gameLoadedAnimation.saveSlotNumber,
                    platformId: gameLoadedAnimation.platformId,
                    contentPath: gameLoadedAnimation.contentPath
                }, StackView.Immediate)
                // emulatorLoader.startGame(gameLoadedAnimation.gameData, gameLoadedAnimation.saveData, gameLoadedAnimation.corePath, gameLoadedAnimation.contentHash, gameLoadedAnimation.saveSlotNumber, gameLoadedAnimation.platformId, gameLoadedAnimation.contentPath)
                // emulatorScreen
            }
        }

        // PauseAnimation {
        //     duration: 1000
        // }

        PropertyAction {
            target: overlay
            property: "opacity"
            value: 0
        }

    }

    Connections {
        target: GameLoader

        function onGameLoaded(gameData, saveData, corePath, contentHash, saveSlotNumber, platformId, contentPath) {
            gameLoadedAnimation.gameData = gameData
            gameLoadedAnimation.saveData = saveData
            gameLoadedAnimation.corePath = corePath
            gameLoadedAnimation.contentHash = contentHash
            gameLoadedAnimation.saveSlotNumber = saveSlotNumber
            gameLoadedAnimation.platformId = platformId
            gameLoadedAnimation.contentPath = contentPath
            gameLoadedAnimation.running = true
        }
    }

    Component {
        id: homeScreen

        HomeScreen {
            focus: true
            onReadyToStartGame: {
                gameStartAnimation.running = true
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
