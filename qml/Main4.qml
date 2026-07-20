import QtQuick
import QtQuick.Controls
import QtQuick.Layouts 1.0

MainWindow {
    id: window

    TapHandler {
        onTapped: window.contentItem.forceActiveFocus(Qt.MouseFocusReason)
    }

    background: FLUserBackground {
        mode: AppearanceSettings.backgroundMode
        color1: AppearanceSettings.backgroundColor
        color2: AppearanceSettings.backgroundColor2
        backgroundFile: AppearanceSettings.backgroundFile
        blurAmount: AppearanceSettings.backgroundBlur
        dimAmount: AppearanceSettings.backgroundDim
        defaultColor: "#12131A"
    }

    // Hosts overlay routes (e.g. /settings) as a popup over the current view
    RouteOverlay {
        id: routeOverlay
    }

    // A hotkey has no other feedback: without this, a save state and a binding
    // that silently doesn't work look identical. The wording is decided in C++
    Toast {
        id: shortcutToast
        z: 999

        Connections {
            target: ShortcutDispatcher
            function onNotified(message) {
                shortcutToast.show(message);
            }
        }
    }

    Action {
        id: searchAction
        text: qsTr("&Copy")
        icon.name: "edit-copy"
        shortcut: StandardKey.Find
        onTriggered: actualTitleBar.activateSearch()
    }

    Pane {
        id: titleBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: AppStyle.titleBarHeight
        padding: AppStyle.titleBarPadding

        background: Rectangle {
            color: "transparent"
            anchors.fill: parent
        }

        TitleBar {
            id: actualTitleBar
            anchors.fill: parent

            onMaximizeClicked: {
                window.maximize();
                // emulatorLoader.setSource("NewEmulatorPage.qml", {stackView: contentStack})
            }
            onMinimizeClicked: window.showMinimized()
            onCloseClicked: window.close()
        }
    }

    Pane {
        id: navRail
        anchors.top: titleBar.bottom
        anchors.left: parent.left
        anchors.leftMargin: 2
        anchors.bottom: parent.bottom
        width: Math.round(48 * AppStyle.scale)

        background: Item {}

        ColumnLayout {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            spacing: AppStyle.spacingLg

            Repeater {
                model: [
                    {
                        displayName: "Library",
                        iconName: "browse",
                        route: "/library"
                    },
                    {
                        displayName: "Mod Shop",
                        iconName: "shopping-bag",
                        route: "/shop"
                    },
                    {
                        displayName: "Controllers",
                        iconName: "controller",
                        route: "/controllers"
                    },
                    {
                        displayName: "Gallery",
                        iconName: "photo-library",
                        route: "/gallery"
                    },
                    {
                        displayName: "Activity",
                        iconName: "bar-chart",
                        route: "/activity"
                    },
                    {
                        displayName: "Online Lobby",
                        iconName: "online",
                        route: "/netplay"
                    }
                ]

                delegate: FLIconButton {
                    id: menuItem
                    required property var model

                    checkable: false
                    checked: Router.isActive(model.route)
                    checkedColor: Theme.textPrimary
                    size: "md"
                    iconName: model.iconName
                    tooltipText: model.displayName

                    Layout.alignment: Qt.AlignVCenter

                    onClicked: {
                        Router.navigate(model.route);
                    }
                }
            }

            Item {
                Layout.fillHeight: true
                Layout.fillWidth: true
            }

            FLIconButton {
                readonly property string route: "/settings"

                checkable: false
                checked: Router.isActive(route)
                checkedColor: Theme.textPrimary
                size: "md"
                iconName: "settings"
                tooltipText: "Settings"

                Layout.alignment: Qt.AlignVCenter

                onClicked: {
                    Router.navigate(route);
                }
            }
        }
    }

    RouteView {
        id: contentStack

        objectName: "RouteView"

        anchors.bottom: gameplay.top
        anchors.left: navRail.right
        anchors.right: parent.right
        anchors.top: titleBar.bottom
        anchors.leftMargin: 2
        anchors.rightMargin: 8
        anchors.bottomMargin: 8

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

    // The running game + quick menu, layered above the router. It grows to full
    // screen when foregrounded and shrinks into a bottom bar when backgrounded —
    // the game render itself becomes the "now playing" bar
    GameplayLayer {
        id: gameplay
        z: 90
    }

    // Netplay status has to outlive the /netplay page: both of these sit above
    // the gameplay layer so they stay visible wherever the user has navigated
    LobbyChip {
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: AppStyle.spacingLg
        z: 900
    }

    ReadyCheckToast {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: AppStyle.spacingXl
        z: 950

        onHostLaunchRequested: {
            const entryId = NetworkService.selectedGameEntryId();
            if (entryId >= 0) {
                NetworkService.confirmLaunch();
                EmulationService.loadEntry(entryId);
            }
        }
    }

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
