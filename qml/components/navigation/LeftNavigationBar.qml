import QtQuick
import QtQuick.Controls
import QtQuick.Layouts 1.0
import Firelight 1.0

Pane {
    id: root

    // implicitWidth: 320
    padding: 16

    property bool expanded: false
    property bool showQuickMenuButton: false

    states: [
        State {
            name: "collapsed"
            when: !root.expanded
            PropertyChanges {
                target: root
                implicitWidth: 50 + (padding * 2)
            }
        },
        State {
            name: "expanded"
            when: root.expanded
            PropertyChanges {
                target: root
                implicitWidth: 280
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

    ButtonGroup {
        id: buttonGroup
        exclusive: true
    }

    background: Rectangle {
        color: "#19181d"
    }

    contentItem: ColumnLayout {
        spacing: 10

        Item {
            implicitWidth: 50
            implicitHeight: 50

            // Image {
            //     source: "qrc:/images/firelight-logo"
            //     anchors.centerIn: parent
            //     height: 32
            //     sourceSize.height: 32
            //     fillMode: Image.PreserveAspectFit
            //     mipmap: true
            // }
        }

        LeftNavigationItem {
            id: quickMenuButton
            label: "Now Playing"
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

            KeyNavigation.down: settingsButton

            Layout.fillWidth: true

            checked: Router.currentRoute.startsWith("/controllers")
            onToggled: {
                if (toggled) {
                    Router.navigateTo("/controllers")
                }
            }
        }

        // LeftNavigationItem {
        //     id: galleryButton
        //     label: "Gallery"
        //     iconName: "photo-library"
        //
        //     KeyNavigation.down: settingsButton
        //
        //     Layout.fillWidth: true
        //
        //     checked: Router.currentRoute.startsWith("/gallery")
        //     onToggled: {
        //         if (toggled) {
        //             Router.navigateTo("/gallery")
        //         }
        //     }
        // }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        LeftNavigationItem {
            id: settingsButton
            label: "Settings"
            iconName: "settings"

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
    }

    // contentItem: RowLayout {
    //     spacing: 12
    //
    //     FLIcon {
    //         Layout.preferredWidth: parent.height
    //         Layout.preferredHeight: parent.height
    //         size: parent.height
    //         icon: root.iconName
    //         filled: root.checked
    //         color: root.checked ? "#FFFFFF" : "#a8a8a8"
    //     }
    //
    //     Text {
    //         id: labelText
    //         opacity: parent.width > 140 ? 1 : 0
    //         text: root.label
    //         font.pixelSize: 17
    //         font.weight: Font.DemiBold
    //         font.family: Constants.regularFontFamily
    //         color: root.checked ? "#FFFFFF" : "#a8a8a8"
    //         Layout.fillHeight: true
    //         Layout.fillWidth: true
    //         horizontalAlignment: Text.AlignLeft
    //         verticalAlignment: Text.AlignVCenter
    //
    //         Behavior on opacity {
    //             NumberAnimation {
    //                 duration: 100
    //                 easing.type: Easing.InOutQuad
    //             }
    //         }
    //     }
    // }



    TapHandler {
        id: handler
        onTapped: function(event, button) {
            root.expanded = !root.expanded
        }
    }


}
