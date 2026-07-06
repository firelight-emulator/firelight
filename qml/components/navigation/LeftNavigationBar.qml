import QtQuick
import QtQuick.Controls
import QtQuick.Layouts 1.0
import Firelight 1.0

Pane {
    id: root

    signal powerButtonClicked()

    // implicitWidth: 320
    padding: 12

    property bool expanded: false
    property bool showQuickMenuButton: false

    states: [
        State {
            name: "collapsed"
            when: !root.expanded
            PropertyChanges {
                target: root
                implicitWidth: 38 + (padding * 2)
            }
        },
        State {
            name: "expanded"
            when: root.expanded
            PropertyChanges {
                target: root
                implicitWidth: 200
                // y: parent.height
            }
        }
    ]

    Behavior on implicitWidth {
        NumberAnimation {
            duration: 160
            easing.type: Easing.InOutQuad
        }
    }

    background: Rectangle {
        color: "transparent"

        // Rectangle {
        //     color: "white"
        //     opacity: 0.14
        //     width: 1
        //     anchors.top: parent.top
        //     anchors.bottom: parent.bottom
        //     anchors.right: parent.right
        //
        //     anchors.topMargin: 16
        //     anchors.bottomMargin: 16
        // }
    }

    contentItem: ColumnLayout {
        spacing: 8

        Image {
            source: "qrc:/images/firelight-logo"
            Layout.alignment: Qt.AlignHCenter
            sourceSize.width: 30
            sourceSize.height: 30
            fillMode: Image.PreserveAspectFit
        }

        // LeftNavigationItem {
        //     id: expandButton
        //     label: root.expanded ? "Collapse menu" : "Expand menu"
        //     iconName: root.expanded ? "left-panel-close" : "left-panel-open"
        //     checkable: false
        //
        //     KeyNavigation.down: quickMenuButton
        //
        //     Layout.fillWidth: true
        //
        //     onClicked: {
        //         root.expanded = !root.expanded
        //     }
        // }

        Item {
            implicitWidth: 6
            implicitHeight: 6
        }

        LeftNavigationItem {
            id: quickMenuButton
            label: EmulationService.currentGameName
            iconName: "play-circle"
            visible: root.showQuickMenuButton

            KeyNavigation.down: libraryButton

            Layout.fillWidth: true

            checked: Router.currentRoute.startsWith("/quick-menu")
            onToggled: {
                if (toggled) {
                    Router.navigateTo("/quick-menu")
                }
            }
        }

        LeftNavigationItem {
            id: libraryButton
            label: "Library"
            iconName: "book"

            focus: true

            KeyNavigation.down: shopButton

            Layout.fillWidth: true

            checked: Router.currentRoute.startsWith("/library")
            onToggled: {
                if (toggled) {
                    Router.navigateTo("/library")
                }
            }
        }

        LeftNavigationItem {
            id: shopButton
            label: "Mod Shop"
            iconName: "shopping-bag"

            KeyNavigation.down: controllersButton

            Layout.fillWidth: true

            checked: Router.currentRoute.startsWith("/shop")
            onToggled: {
                if (toggled) {
                    Router.navigateTo("/shop")
                }
            }
        }

        LeftNavigationItem {
            id: controllersButton
            label: "Controllers"
            iconName: "controller"

            KeyNavigation.down: activityButton

            Layout.fillWidth: true

            checked: Router.currentRoute.startsWith("/controllers")
            onToggled: {
                if (toggled) {
                    Router.navigateTo("/controllers")
                }
            }
        }

        LeftNavigationItem {
            id: activityButton
            label: "Activity"
            iconName: "bar-chart"

            KeyNavigation.down: settingsButton

            Layout.fillWidth: true

            checked: Router.currentRoute.startsWith("/activity")
            onToggled: {
                if (toggled) {
                    Router.navigateTo("/activity")
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        LeftNavigationItem {
            id: settingsButton
            label: "Settings"
            iconName: "settings"

            KeyNavigation.down: helpButton

            Layout.fillWidth: true

            checked: Router.currentRoute.startsWith("/settings")
            onClicked: {
                if (toggled) {
                    return
                }
            }
            onToggled: {
                if (toggled) {
                    Router.navigateTo("/settings")
                }
            }
        }

        LeftNavigationItem {
            id: helpButton
            label: "Help"
            iconName: "info"

            KeyNavigation.down: powerButton

            Layout.fillWidth: true

            checked: Router.currentRoute.startsWith("/help")
            onToggled: {
                if (toggled) {
                    Router.navigateTo("/help")
                }
            }
        }

        LeftNavigationItem {
            id: powerButton
            label: "Power"
            iconName: "power"

            Layout.fillWidth: true

            checkable: false

            onClicked: {
                root.powerButtonClicked()
            }
        }
    }
}
