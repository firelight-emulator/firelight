import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Window
import QtQml.Models
import QtQuick.Layouts 1.0
import QtNetwork
import QtMultimedia
import QtQuick.Effects
import Firelight 1.0

ApplicationWindow {
    id: window
    objectName: "Application Window"

    width: GeneralSettings.mainWindowWidth
    height: GeneralSettings.mainWindowHeight
    x: GeneralSettings.mainWindowX
    y: GeneralSettings.mainWindowY

    onWidthChanged: {
        GeneralSettings.mainWindowWidth = width
    }

    onHeightChanged: {
        GeneralSettings.mainWindowHeight = height
    }

    onXChanged: {
        GeneralSettings.mainWindowX = x
    }

    onYChanged: {
        GeneralSettings.mainWindowY = y
    }

    // onActiveFocusItemChanged: {
    //     console.log("Active focus item changed to: " + window.activeFocusItem)
    //     let item = window.activeFocusItem
    //     let level = 0
    //     while (item) {
    //         let spaces = " ".repeat(level * 2)
    //
    //         console.log(spaces + item)
    //         item = item.parent
    //         level++
    //     }
    // }

    // flags: Qt.NoTitleBarBackgroundHint

    visible: true
    visibility: GeneralSettings.fullscreen ? Window.FullScreen : Window.Windowed

    title: qsTr("Firelight")

    property var maxContentWidth: 1400

    property bool blur: false

    background: FLUserBackground {
        blur: window.blur
        defaultColor: "#222328"
        usingCustomBackground: AppearanceSettings.usingCustomBackground
        backgroundFile: AppearanceSettings.backgroundFile
    }

    function startGame(entryId) {
        if (emulatorLoader.status === Loader.Ready) {
            closeGameDialog.openAndDoOnAccepted(function () {
                emulatorLoader.source = ""

                startGameAnimation.entryId = entryId
                startGameAnimation.start()

                // emulatorLoader.entryId = entryId
                // emulatorLoader.active = true
                // libPage.playLaunchAnimation()
            })
        } else {
            startGameAnimation.entryId = entryId
            startGameAnimation.start()
        }
    }

    Loader {
        id: emulatorLoader
        // visible: status === Loader.Ready
        focus: true

        StackView.visible: true

        property real blurAmount: 0

        Behavior on blurAmount {
            NumberAnimation {
                easing.type: Easing.InOutQuad
                duration: 250
            }
        }

        layer.enabled: blurAmount !== 0
        layer.effect: MultiEffect {
            // enabled: root.blurAmount !== 0
            source: emulatorLoader
            anchors.fill: emulatorLoader
            blurEnabled: true
            blurMultiplier: 0
            blurMax: 64
            autoPaddingEnabled: false
            blur: emulatorLoader.blurAmount
        }

        Rectangle {
            id: dimmer
            color: "black"
            visible: emulatorLoader.status === Loader.Ready
            opacity: emulatorLoader.blurAmount * 0.55
            anchors.fill: parent

            z: 10
        }

        Keys.onEscapePressed: function(event) {
            Router.navigateTo("/quick-menu")
            mainContentStack.pushItem(content, {}, StackView.PushTransition)
        }

        StackView.onActivating: {
            emulatorLoader.blurAmount = 0
        }

        StackView.onActivated: {
            emulatorLoader.item.paused = false
        }

        StackView.onDeactivating: {
            emulatorLoader.blurAmount = 1
            emulatorLoader.item.paused = true
        }
    }

    MainContent {
        id: content
        visible: false

        gameRunning: emulatorLoader.status === Loader.Ready

        Keys.onEscapePressed: function(event) {
            if (Router.currentRoute !== "/quick-menu") {
                Router.navigateTo("/quick-menu")
                return
            }

            mainContentStack.popCurrentItem()
        }
    }

    StackView {
        id: mainContentStack
        focus: true
        anchors.fill: parent
        Component.onCompleted: {
            pushItems([emulatorLoader, content], StackView.Immediate)
        }

        onActiveFocusChanged: {
            if (activeFocus && depth > 0) {
                mainContentStack.currentItem.forceActiveFocus()
            }
        }

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
        pushExit: Transition {}
        popEnter: Transition {}
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

    Rectangle {
        id: overlay
        color: "black"
        anchors.fill: parent
        opacity: 0
    }

    SequentialAnimation {
        id: startGameAnimation

        property var entryId: -1
        NumberAnimation {
            target: overlay
            property: "opacity"
            from: 0
            to: 1
            duration: 400
            easing.type: Easing.InOutQuad
        }

        ScriptAction {
            script: {
                mainContentStack.popCurrentItem(StackView.Immediate)
                emulatorLoader.setSource("NewEmulatorPage.qml", {entryId: startGameAnimation.entryId, stackView: mainContentStack})
                overlay.opacity = 0

            }
        }
    }

    FLFocusHighlight {
        target: window.activeFocusItem
        usingMouse: InputMethodManager.usingMouse
    }

    LibraryPage {
        id: allGamesPage
        visible: false
        onStartGame: function (entryId) {
            window.startGame(entryId)
        }
    }

    FirelightDialog {
        id: closeGameDialog
        text: "You're currently playing:\n\n" + emulatorLoader.item.gameName + "\n\nDo you want to close it?"

    }

    Component {
        id: quickMenuPage
        QuickMenu {
            onResumeGame: {
                mainContentStack.popCurrentItem()
            }

            onResetGame: {
                emulatorLoader.item.resetGame()
                mainContentStack.popCurrentItem()
            }

            onCloseGame: {
                Router.navigateTo("/library")
                emulatorLoader.source = ""
            }

            onRewindPressed: {
                 emulatorLoader.item.createRewindPoints()
            }

        }
    }

    Component {
        id: gameDetailsPage
        FLGameDetailsPanel {

        }
    }

    Component {
        id: shopPage
        ShopLandingPage {
            model: shop_item_model
        }
    }

    Component {
        id: shopItemPage
        ShopItemPage {
        }
    }

    Component {
        id: settingsScreen

        SettingsScreen {
            gameRunning: emulatorLoader.status === Loader.Ready
        }
    }

    Component {
        id: controllersPage

        ControllersPage {
            onEditProfileButtonClicked: function (name, playerNumber) {
                if (name === "Keyboard") {
                    Router.navigateTo("/controllers/keyboard/" + playerNumber)
                    // screenStack.pushItem(keyboardProfileEditor, {playerNumber: playerNumber}, StackView.PushTransition)
                } else {
                    Router.navigateTo("/controllers/profiles/" + playerNumber)
                    // screenStack.pushItem(profileEditor, {playerNumber: playerNumber}, StackView.PushTransition)
                }
            }

        }
    }

    Component {
        id: controllerInputMappingPage

        ControllerProfilePage {

        }
    }

    Component {
        id: keyboardInputMappingPage

        KeyboardProfilePage {

        }
    }

    Component {
        id: lol
        Text {
            text: "How did you even get here"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color: "white"
        }
    }

    Connections {
            target: Router

            function onRouteChanged(route) {
                // if (!content.visible) {
                //     console.log("NOT VISIBLE")
                //     return
                // }

                if (route === "/shop") {
                    content.goToContent("Mod Shop", shopPage, {}, StackView.ReplaceTransition)
                } else if (route === "/library") {
                    content.goToContent("Library", allGamesPage, {}, StackView.ReplaceTransition)
                } else if (route.startsWith("/library/entries/")) {
                    let things = route.split("/")
                    if (things.length === 4) {
                        content.goToContent("Game Details", gameDetailsPage, {entryId: things[3]}, StackView.PushTransition)
                    } else {
                        content.goToContent("Not found", lol, {}, StackView.PushTransition)
                    }
                } else if (route.startsWith("/shop/mods/")) {
                    let things = route.split("/")
                    content.goToContent("Mod Details", shopItemPage, {modId: things[3]}, StackView.PushTransition)
                } else if (route === "/settings") {
                    content.goToContent("Settings", settingsScreen, {}, StackView.ReplaceTransition)
                } else if (route === "/controllers") {
                    content.goToContent("Controllers", controllersPage, {}, StackView.ReplaceTransition)
                } else if (route.startsWith("/controllers/keyboard/")) {
                    let things = route.split("/")
                    if (things.length === 4) {
                        content.goToContent("Edit keyboard profile", keyboardInputMappingPage, {playerNumber: things[3]}, StackView.PushTransition)
                    } else {
                        content.goToContent("Not found", lol, {}, StackView.PushTransition)
                    }
                } else if (route.startsWith("/controllers/profiles/")) {
                    let things = route.split("/")
                    if (things.length === 4) {
                        content.goToContent("Edit controller profile", controllerInputMappingPage, {playerNumber: things[3]}, StackView.PushTransition)
                    } else {
                        content.goToContent("Not found", lol, {}, StackView.PushTransition)
                    }
                } else if (route === "/quick-menu") {
                    content.goToContent("Quick Menu", quickMenuPage, {gameSettings: emulatorLoader.item.gameSettings}, StackView.ReplaceTransition)
                } else {
                        content.goToContent("Not found", lol, {}, StackView.ReplaceTransition)
                }
            }
        }
}
