// TODO: NEEDS REVIEW
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts 1.0
import QtQuick.Window
import Firelight 1.0

MainWindow {
    id: window

    TapHandler {
        onTapped: window.contentItem.forceActiveFocus(Qt.MouseFocusReason)
    }

    background: FLUserBackground {
        mode: AppearanceSettings.backgroundMode
        // color1: AppearanceSettings.backgroundColor
        // color2: AppearanceSettings.backgroundColor2
        backgroundFile: AppearanceSettings.backgroundFile
        blurAmount: 0
        dimAmount: 0
        defaultColor: "#1A1A1C"
    }

    // Hosts overlay routes (e.g. /settings) as a popup over the current view
    // RouteOverlay {
    //     id: routeOverlay
    // }

    // A hotkey has no other feedback: without this, a save state and a binding
    // that silently doesn't work look identical. The wording is decided in C++
    // Toast {
    //     id: shortcutToast
    //     z: 999
    //
    //     Connections {
    //         target: ShortcutDispatcher
    //         function onNotified(message) {
    //             shortcutToast.show(message);
    //         }
    //     }
    // }

    Action {
        id: searchAction
        text: qsTr("&Copy")
        icon.name: "edit-copy"
        shortcut: StandardKey.Find
        onTriggered: actualTitleBar.activateSearch()
    }

    // TODO
    // Dev-only: toggle the component gallery
    Shortcut {
        sequence: "F11"
        context: Qt.ApplicationShortcut
        onActivated: Router.matchedPattern === "/dev/gallery" ? Router.back() : Router.navigate("/dev/gallery")
    }

    // TODO
    // Dev-only: the disc set manager, in a window of its own until it has a home in the UI
    Shortcut {
        sequence: "F10"
        context: Qt.ApplicationShortcut
        onActivated: {
            if (discSetWindow.visible) {
                discSetWindow.close()
            } else {
                DiscSetModel.refresh()
                discSetWindow.show()
            }
        }
    }

    Window {
        id: discSetWindow

        objectName: "DiscSetManagerWindow"
        width: 1000
        height: 640
        title: qsTr("Disc sets")
        color: Theme.surface

        DiscSetManager {
            anchors.fill: parent
        }
    }

    // TODO
    // Toggles the performance overlay, on the key RetroArch uses for the same thing so the two can
    // be brought up side by side without relearning it
    Shortcut {
        sequence: "F3"
        context: Qt.ApplicationShortcut
        onActivated: PerformanceStats.toggle()
    }

    PerformanceOverlay {
        id: performanceOverlay

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: AppStyle.spacingLg
        z: 9999
    }

    onActiveFocusItemChanged: {
        Qt.callLater(guideBarActions.refresh);

        if (!activeFocusItem) {
            return;
        }

        // console.log("Active focus item changed to: " + activeFocusItem.FLFocus.collectActions(activeFocusItem).map(a => a.label).join(", "));
    }

    // Pane {
    //     id: navRail
    //     anchors.top: titleBar.bottom
    //     anchors.left: parent.left
    //     anchors.bottom: parent.bottom
    //     width: Math.round(48 * AppStyle.scale)
    //
    //     background: Item {}
    //
    //     ColumnLayout {
    //         anchors.horizontalCenter: parent.horizontalCenter
    //         anchors.top: parent.top
    //         spacing: AppStyle.spacingLg
    //
    //         Repeater {
    //             model: [
    //                 {
    //                     displayName: "Library",
    //                     iconName: "browse",
    //                     route: "/library"
    //                 },
    //                 {
    //                     displayName: "Mod Shop",
    //                     iconName: "shopping-bag",
    //                     route: "/shop"
    //                 },
    //                 {
    //                     displayName: "Controllers",
    //                     iconName: "controller",
    //                     route: "/controllers"
    //                 },
    //                 {
    //                     displayName: "Gallery",
    //                     iconName: "photo-library",
    //                     route: "/gallery"
    //                 },
    //                 {
    //                     displayName: "Activity",
    //                     iconName: "bar-chart",
    //                     route: "/activity"
    //                 },
    //                 {
    //                     displayName: "Online Lobby",
    //                     iconName: "online",
    //                     route: "/netplay"
    //                 }
    //             ]
    //
    //             delegate: FLIconButton {
    //                 id: menuItem
    //                 required property var model
    //
    //                 checkable: false
    //                 checked: Router.isActive(model.route)
    //                 checkedColor: Theme.textPrimary
    //                 iconName: model.iconName
    //                 tooltipText: model.displayName
    //
    //                 Layout.alignment: Qt.AlignVCenter
    //
    //                 onClicked: {
    //                     Router.navigate(model.route);
    //                 }
    //             }
    //         }
    //
    //         Item {
    //             Layout.fillHeight: true
    //             Layout.fillWidth: true
    //         }
    //
    //         FLIconButton {
    //             readonly property string route: "/settings"
    //
    //             checkable: false
    //             checked: Router.isActive(route)
    //             checkedColor: Theme.textPrimary
    //             iconName: "settings"
    //             tooltipText: "Settings"
    //
    //             Layout.alignment: Qt.AlignVCenter
    //
    //             onClicked: {
    //                 Router.navigate(route);
    //             }
    //         }
    //     }
    // }

    Item {
        id: contentContainer
        anchors.fill: parent

        // TODO
        // Arrow presses no container used end up here — the cursor's edge bump.
        // Fires once per continuous hold: the first dead press bumps, repeats
        // stay quiet, and the release (or focus moving) re-arms it
        property bool deadHeld: false

        // TODO
        // Runs an action and reports that it ran, so each lookup below reads as one line
        function runAction(action: FLAction): bool {
            if (action === null) {
                return false;
            }

            if (action.sound) {
                action.sound.play(false);
            }

            action.triggered();
            return true;
        }

        // TODO
        // Runs whatever the focused item declared for this key. Falls back to
        // click() so an ordinary button needs no declaration at all
        function activate(key: int, modifiers: int): bool {
            const focused = window.activeFocusItem;

            if (focused === null) {
                return false;
            }

            const info = contentContainer.FLFocus.find(focused);

            // TODO
            // Looks outward from the focused item, so an action a container declares answers for
            // everything inside it, which is what the guide bar already lists
            if (contentContainer.runAction(contentContainer.FLFocus.findActionFor(focused, key, modifiers))) {
                return true;
            }

            // TODO
            // This handler is the container's own, so what the container declares answers here the
            // way a button's own actions answer in its handler
            if (contentContainer.runAction(contentContainer.FLFocus.getActionFor(key, modifiers))) {
                return true;
            }

            if (info !== null && info.getActionCount() > 0) {
                return false;
            }

            // TODO
            // The fallback ignores modifiers on purpose: a modified press no declared action
            // answered still presses the thing, the way a modified click does
            if (key !== Qt.Key_Select && key !== Qt.Key_Return && key !== Qt.Key_Enter) {
                return false;
            }

            if (typeof focused.click !== "function") {
                return false;
            }

            focused.click();
            return true;
        }

        Keys.onPressed: event => {
            if (gameplay.foregrounded) {
                return;
            }

            // TODO: Add allowAutoRepeat
            // Holding a face button repeats ~33 times a second, so only the
            // press itself activates
            if (!event.isAutoRepeat && contentContainer.activate(event.key, event.modifiers)) {
                event.accepted = true;
                return;
            }

            // TODO
            // A direction no container claimed is answered against whatever
            // else is on screen, so the cursor crosses between them without
            // either knowing the other exists
            if (FocusNavigator.move(window.activeFocusItem, event.key, event.isAutoRepeat) !== FocusNavigator.NoTarget) {
                event.accepted = true;
                return;
            }

            if (contentContainer.deadHeld) {
                return;
            }

            contentContainer.deadHeld = FocusCursor.bump(event.key);
        }

        Keys.onReleased: event => {
            if (event.isAutoRepeat) {
                return;
            }

            if (event.key === Qt.Key_Up || event.key === Qt.Key_Down || event.key === Qt.Key_Left || event.key === Qt.Key_Right) {
                contentContainer.deadHeld = false;
            }
        }

        Connections {
            target: focusHighlight
            function onCursorItemChanged() {
                contentContainer.deadHeld = false;
            }
        }

        RouteView {
            id: contentStack
            clip: true

            objectName: "RouteView"

            anchors.bottomMargin: guideBar.height
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: titleBar.bottom

            Component.onCompleted: Router.navigate(StartupOptions.startupRoute !== "" ? StartupOptions.startupRoute : "/library")
        }

        // Shown while an uncached page is being built asynchronously
        BusyIndicator {
            anchors.centerIn: contentStack
            implicitWidth: 48
            implicitHeight: 48
            running: contentStack.loading
            visible: running
            z: 10
        }

        Pane {
            id: guideBar
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: AppStyle.windowPadding
            anchors.rightMargin: AppStyle.windowPadding
            verticalPadding: AppStyle.spacingXs
            horizontalPadding: 0
            height: 60
            z: focusHighlight.z + 1

            parent: Overlay.overlay

            background: Item {
                Rectangle {
                    color: "transparent"
                    anchors.fill: parent
                }
                Rectangle {
                    color: "#4c4c4c"
                    height: 1
                    width: parent.width
                }
            }

            RowLayout {
                anchors.fill: parent
                spacing: AppStyle.spacingXs

                FLGuideBarPill {
                    text: qsTr("Open menu")
                    bindings: [
                        {
                            text: qsTr("Open menu"),
                            key: Qt.Key_Home
                        }
                    ]
                    actionEnabled: true
                }

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }

                Repeater {
                    id: guideBarActions

                    Layout.alignment: Qt.AlignVCenter

                    property var groups: []

                    function refresh() {
                        const focused = window.activeFocusItem;
                        guideBarActions.groups = focused === null ? [] : focused.FLFocus.collectActionGroups(focused).reverse();
                    }

                    Component.onCompleted: guideBarActions.refresh()

                    model: guideBarActions.groups
                    delegate: FLGuideBarPill {
                        required property var modelData

                        text: modelData.label
                        bindings: modelData.bindings
                        actionEnabled: modelData.enabled
                    }
                }
            }
        }

        Pane {
            id: titleBar
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: AppStyle.titleBarHeight
            padding: AppStyle.titleBarPadding

            background: Item {
                Rectangle {
                    color: Theme.border
                    height: 1
                    anchors.left: parent.left
                    anchors.leftMargin: AppStyle.windowPadding
                    anchors.right: parent.right
                    anchors.rightMargin: AppStyle.windowPadding
                    anchors.bottom: parent.bottom
                }
            }
            TitleBar {
                id: actualTitleBar
                anchors.fill: parent

                page: contentStack.currentItem

                onMaximizeClicked: window.maximize();
                onMinimizeClicked: window.showMinimized()
                onCloseClicked: window.close()
            }
        }

        // The running game + quick menu, layered above the router. It grows to full
        // screen when foregrounded and shrinks into a bottom bar when backgrounded —
        // the game render itself becomes the "now playing" bar
        GameplayLayer {
            id: gameplay
            z: 90
        }

        Item {
            id: dimmer
            anchors.fill: parent
            Component.onCompleted: {
                FLDimmer.target = dimmer;
            }
        }

        FLFocusHighlight {
            id: focusHighlight
            parent: Overlay.overlay
            z: 100000
            target: window.activeFocusItem
            usingMouse: InputMethodManager.usingMouse

            Component.onCompleted: FocusCursor.register(focusHighlight)
        }
    }

    LaunchCinematic {
        id: launchCinematic
        parent: Overlay.overlay
        z: 200000
    }

    Connections {
        target: EmulationService

        function onGameLoadStarted() {
            launchCinematic.launchFrom(FocusCursor.highlight ? FocusCursor.highlight.cursorItem : null);
        }

        function onGameLoadFailed(message) {
            launchCinematic.reveal();
        }
    }

    Connections {
        target: launchCinematic
        function onBlackFull() {
            gameplay.markBlackFull();
        }
    }

    Connections {
        target: gameplay
        function onReadyToReveal() {
            launchCinematic.reveal();
        }
    }

    // Netplay status has to outlive the /netplay page: both of these sit above
    // the gameplay layer so they stay visible wherever the user has navigated
    // LobbyChip {
    //     anchors.left: parent.left
    //     anchors.bottom: parent.bottom
    //     anchors.margins: AppStyle.spacingLg
    //     z: 900
    // }
    //
    // ReadyCheckToast {
    //     anchors.horizontalCenter: parent.horizontalCenter
    //     anchors.bottom: parent.bottom
    //     anchors.bottomMargin: AppStyle.spacingXl
    //     z: 950
    //
    //     onHostLaunchRequested: {
    //         const entryId = NetworkService.selectedGameEntryId();
    //         if (entryId >= 0) {
    //             NetworkService.confirmLaunch();
    //             EmulationService.loadEntry(entryId);
    //         }
    //     }
    // }

    // A game named on the command line launches once the shell is up. When the
    // CLI also asked for a RetroAchievements login, that has to land first so
    // the session is credited to the right user
    function maybeAutoLaunch() {
        if (StartupOptions.launchEntryId >= 0) {
            Qt.callLater(function () {
                EmulationService.loadEntry(StartupOptions.launchEntryId);
            });
        }
    }

    // The launch still proceeds on a failed login, just without achievements
    function raLoginFailed(reason) {
        shortcutToast.show(qsTr("RetroAchievements login failed: ") + reason);
        window.maybeAutoLaunch();
    }

    function beginRaLogin() {
        if (StartupOptions.raToken.length > 0) {
            achievement_manager.logInUserWithToken(StartupOptions.raUsername, StartupOptions.raToken);
        } else {
            achievement_manager.logInUserWithPassword(StartupOptions.raUsername, StartupOptions.raPassword);
        }
    }

    Connections {
        target: achievement_manager
        enabled: StartupOptions.raPendingLogin

        function onLoginSucceeded() {
            window.maybeAutoLaunch();
        }

        function onLoginFailedWithInvalidCredentials() {
            window.raLoginFailed(qsTr("Invalid username or password"));
        }

        function onLoginFailedWithExpiredToken() {
            window.raLoginFailed(qsTr("Login token has expired"));
        }

        function onLoginFailedWithAccessDenied() {
            window.raLoginFailed(qsTr("Access denied"));
        }

        function onLoginFailedWithInternalError() {
            window.raLoginFailed(qsTr("RetroAchievements is unreachable"));
        }
    }

    // Null unless --single-instance was passed; a second process forwards its
    // launch here instead of opening another window
    Connections {
        target: SingleInstance
        enabled: SingleInstance !== null

        function onLaunchRequested(entryId) {
            if (entryId >= 0) {
                EmulationService.loadEntry(entryId);
            }
        }
    }

    Component.onCompleted: {
        if (StartupOptions.raPendingLogin) {
            window.beginRaLogin();
        } else {
            window.maybeAutoLaunch();
        }
    }

    component RoleData: QtObject {
        property string displayName
    }
}
