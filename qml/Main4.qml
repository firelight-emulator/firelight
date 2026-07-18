import QtQuick
import QtQuick.Controls
import QtQuick.Window
import QtQuick.Layouts 1.0
import QtQuick.Effects
import Firelight 1.0

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

            onMenuButtonClicked: {
                navigationPane.open()
            }

            onMaximizeClicked: {
                window.maximize()
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
        // Scales with the UI so the icon buttons (which grow with scale) always
        // fit instead of overflowing a fixed rail
        width: Math.round(58 * AppStyle.scale)

        background: Item {}

        ColumnLayout {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            spacing: AppStyle.spacingLg

            Repeater {
                model: [
                    { displayName: "Library", iconName: "browse", route: "/library" },
                    { displayName: "Mod Shop", iconName: "shopping-bag", route: "/shop" },
                    { displayName: "Controllers", iconName: "controller", route: "/controllers" },
                    { displayName: "Gallery", iconName: "photo-library", route: "/gallery" },
                    { displayName: "Activity", iconName: "bar-chart", route: "/activity" },
                    { displayName: "Online Lobby", iconName: "online", route: "/netplay" }
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
                        Router.navigate(model.route)
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
                    Router.navigate(route)
                }
            }
        }
    }

    RouteView {
        id: contentStack

        anchors.bottom: gameplay.top
        anchors.left: navRail.right
        anchors.right: parent.right
        anchors.top: titleBar.bottom
        anchors.leftMargin: 2
        anchors.rightMargin: 8
        anchors.bottomMargin: 8

        Component.onCompleted: Router.navigate("/library")
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

    // TODO
    // The running game + quick menu, layered above the router. It grows to full
    // screen when foregrounded and shrinks into a bottom bar when backgrounded —
    // the game render itself becomes the "now playing" bar
    GameplayLayer {
        id: gameplay
        z: 90
    }

    component RoleData: QtObject {
        property string displayName
    }
}
