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

    Connections {
        target: EmulationService

        function onGameLoaded() {
            overlay.opacity = 0
            emulatorLoader.startGame()
        }

        function onEmulationStopped() {
            Router.navigateTo("/library")
            mainContentStack.pushItems([emulatorLoader, content], StackView.Immediate)
            emulatorLoader.source = ""
            content.forceActiveFocus()
        }
    }

    function startGame(entryId) {
        if (EmulationService.isGameRunning) {
            closeGameDialog.openAndDoOnAccepted(function () {
                EmulationService.stopEmulation()
                emulatorLoader.setSource("")
                startGameAnimation.entryId = entryId
                startGameAnimation.start()
            })
        } else {
            startGameAnimation.entryId = entryId
            startGameAnimation.start()
        }
    }

    StackView {
         id: mainContentStack
         focus: true
         anchors.fill: parent

         Component.onCompleted: {
             pushItems([emulatorLoader, content], StackView.Immediate)
         }

         onDepthChanged: {
             console.log("New depth: " + depth)
         }

         Keys.onEscapePressed: function(event) {
             if (EmulationService.isGameRunning && depth > 1) {
                 popCurrentItem()
             }
         }

         // onCurrentItemChanged: {
         //     currentItem.focus = true
         // }

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
                     property: "scale"
                     from: 1.02
                     to: 1
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
                     property: "scale"
                     from: 1
                     to: 1.02
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

    Item {
        focus: false
        EmulatorLoader {
            id: emulatorLoader

            onSuspended: {
                Router.navigateTo("/quick-menu")
                mainContentStack.pushItems([emulatorLoader, content], StackView.PushTransition)
            }
        }

        MainContent {
            id: content
            gameRunning: EmulationService.isGameRunning
        }

        LibraryPage {
            id: allGamesPage
            visible: false
            currentEntryId: EmulationService.currentEntryId
            onStartGame: function (entryId) {
                window.startGame(entryId)
            }
        }

        FirelightDialog {
            id: closeGameDialog
            text: "You're currently playing:\n\n" + EmulationService.currentGameName  + "\n\nDo you want to close it?"

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
                emulatorLoader.setSource("NewEmulatorPage.qml", {stackView: mainContentStack})
                emulatorLoader.blurAmount = 0
                EmulationService.loadEntry(startGameAnimation.entryId)
            }
        }
    }

    FLFocusHighlight {
        target: window.activeFocusItem
        usingMouse: InputMethodManager.usingMouse
    }

    Component {
        id: quickMenuPage
        QuickMenu {
            saveSlotNumber: window.runningGameSaveSlotNumber

            onResumeGame: {
                mainContentStack.popCurrentItem()
            }

            onResetGame: {
                EmulationService.resetGame()
                mainContentStack.popCurrentItem()
                emulatorLoader.forceActiveFocus()
                // mainContentStack.pop(emulatorLoader)
            }

            onCloseGame: {
                EmulationService.stopEmulation()
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
            gameRunning: EmulationService.isGameRunning
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
                    content.goToContent("Quick Menu", quickMenuPage, {saveSlotNumber: emulatorLoader.item.saveSlotNumber, gameSettings: emulatorLoader.item.gameSettings}, StackView.ReplaceTransition)
                } else {
                        content.goToContent("Not found", lol, {}, StackView.ReplaceTransition)
                }
            }
        }
}
