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
    // A CLI --fullscreen/--windowed override (session-only) wins over the saved
    // preference; -1 means no override was given.
    visibility: StartupOptions.fullscreenOverride === 1 ? Window.FullScreen
              : StartupOptions.fullscreenOverride === 0 ? Window.Windowed
              : (GeneralSettings.fullscreen ? Window.FullScreen : Window.Windowed)

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
            content.goToContent("Quick Menu", quickMenuPage, {}, StackView.Immediate)
            emulatorLoader.startGame()
        }

        function onEmulationStopped() {
            // External-launcher mode: the app exists only to run this one game.
            if (StartupOptions.exitOnClose) {
                Qt.quit()
                return
            }
            content.goToContent("Library", allGamesPage, {}, StackView.Immediate)
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

    // A CLI RetroAchievements login (via --ra-*) runs before the game launches;
    // the auto-launch waits for the result and a failure surfaces a dialog.
    property bool raLoginInProgress: false

    function maybeAutoLaunch() {
        if (StartupOptions.launchEntryId >= 0) {
            Qt.callLater(function () {
                window.startGame(StartupOptions.launchEntryId)
            })
        }
    }

    function beginRaLogin() {
        window.raLoginInProgress = true
        if (StartupOptions.raToken.length > 0) {
            achievement_manager.logInUserWithToken(StartupOptions.raUsername, StartupOptions.raToken)
        } else {
            achievement_manager.logInUserWithPassword(StartupOptions.raUsername, StartupOptions.raPassword)
        }
    }

    function onRaLoginFailed(message) {
        window.raLoginInProgress = false
        raLoginDialog.message = message
        raLoginDialog.open()
    }

    // Auto-launch a game passed on the command line (`firelight <rom>`), once
    // the window and its content stack have been set up. If a CLI login was
    // requested, do that first and gate the launch on its result.
    Component.onCompleted: {
        if (StartupOptions.raPendingLogin) {
            window.beginRaLogin()
        } else {
            window.maybeAutoLaunch()
        }
    }

    // A launch forwarded from a second `--single-instance` process (SingleInstance
    // is null unless --single-instance was passed).
    Connections {
        target: SingleInstance
        enabled: SingleInstance !== null

        function onLaunchRequested(entryId) {
            if (entryId >= 0) {
                window.startGame(entryId)
            }
        }
    }

    Connections {
        target: achievement_manager
        enabled: window.raLoginInProgress

        function onLoginSucceeded() {
            window.raLoginInProgress = false
            window.maybeAutoLaunch()
        }
        function onLoginFailedWithInvalidCredentials() {
            window.onRaLoginFailed("Invalid username or password.")
        }
        function onLoginFailedWithExpiredToken() {
            window.onRaLoginFailed("Your saved login token has expired.")
        }
        function onLoginFailedWithAccessDenied() {
            window.onRaLoginFailed("Access was denied.")
        }
        function onLoginFailedWithInternalError() {
            window.onRaLoginFailed("A RetroAchievements server error occurred.")
        }
    }

    StackView {
         id: mainContentStack
         focus: true
         anchors.fill: parent

         Component.onCompleted: {
             pushItems([emulatorLoader, content], StackView.Immediate)
         }

        Keys.onPressed: function(event) {
            if (event.key === Qt.Key_Home || event.key === Qt.Key_Escape || event.key === Qt.Key_Back) {
                if (EmulationService.isGameRunning && depth > 1) {
                    popCurrentItem()
                    event.accepted = true
                    return
                }
            }

            event.accepted = false
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
                // content.goToContent("Quick Menu", quickMenuPage, {saveSlotNumber: emulatorLoader.item.saveSlotNumber}, StackView.Immediate)
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

    // Shown when a CLI-requested RetroAchievements login (--ra-*) fails. Lets the
    // user retry with corrected credentials, continue unauthenticated, or quit.
    Dialog {
        id: raLoginDialog

        property string message: ""

        modal: true
        parent: Overlay.overlay
        anchors.centerIn: parent
        padding: 24
        closePolicy: Popup.NoAutoClose

        background: Rectangle {
            color: ColorPalette.neutral900
            radius: 8
            border.color: ColorPalette.neutral700
            border.width: 1
        }

        contentItem: ColumnLayout {
            spacing: 12

            Text {
                text: "RetroAchievements login failed"
                color: "white"
                font.family: Constants.regularFontFamily
                font.pixelSize: 20
            }

            Text {
                Layout.preferredWidth: 380
                text: raLoginDialog.message
                color: "white"
                font.family: Constants.regularFontFamily
                font.pixelSize: 15
                wrapMode: Text.WordWrap
            }

            TextField {
                id: raUsernameField
                Layout.fillWidth: true
                placeholderText: "Username"
                text: StartupOptions.raUsername
            }

            TextField {
                id: raPasswordField
                Layout.fillWidth: true
                placeholderText: "Password"
                echoMode: TextInput.Password
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                FirelightButton {
                    label: "Abort"
                    onClicked: {
                        raLoginDialog.close()
                        Qt.quit()
                    }
                }

                Item {
                    Layout.fillWidth: true
                }

                FirelightButton {
                    label: "Continue without login"
                    onClicked: {
                        raLoginDialog.close()
                        window.maybeAutoLaunch()
                    }
                }

                FirelightButton {
                    label: "Retry"
                    onClicked: {
                        raLoginDialog.close()
                        window.raLoginInProgress = true
                        achievement_manager.logInUserWithPassword(raUsernameField.text, raPasswordField.text)
                    }
                }
            }
        }
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
        z: 1000
    }

    Component {
        id: quickMenuPage
        QuickMenu {
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
        id: helpScreen

        HelpScreen {}
    }

    Component {
        id: galleryPage

        GalleryPage {}
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
        id: activityPage
        ActivityPage {}
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
                } else if (route === "/help") {
                    content.goToContent("Help", helpScreen, {}, StackView.ReplaceTransition)
                } else if (route === "/gallery") {
                    content.goToContent("Media", galleryPage, {}, StackView.ReplaceTransition)
                } else if (route.startsWith("/gallery/games/")) {
                    let things = route.split("/")
                    if (things.length === 4) {
                        content.goToContent("Media", galleryPage, {gameContentHash: things[3]}, StackView.PushTransition)
                    } else {
                        content.goToContent("Not found", lol, {}, StackView.PushTransition)
                    }
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
                    content.goToContent("Quick Menu", quickMenuPage, {}, StackView.ReplaceTransition)
                } else if (route === "/activity") {
                    content.goToContent("Activity", activityPage, {}, StackView.ReplaceTransition)
                } else {
                        content.goToContent("Not found", lol, {}, StackView.ReplaceTransition)
                }
            }
        }
}
