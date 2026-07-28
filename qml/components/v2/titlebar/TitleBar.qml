import QtQuick
import QtQuick.Layouts 1.0

FocusScope {
    id: root
    signal closeClicked
    signal maximizeClicked
    signal minimizeClicked

    function activateSearch() {
        searchBar.forceActiveFocus();
    }

    Item {
        objectName: "TitleBar|DragArea"
        anchors.fill: parent
        z: -1

        DragHandler {
            target: null
            grabPermissions: DragHandler.ApprovesTakeOverByAnything
            onActiveChanged: if (active)
                root.Window.window.startSystemMove()
            margin: 8
        }

        TapHandler {
            onDoubleTapped: root.maximizeClicked()
            margin: 8
        }
    }

    Item {
        id: contentRow
        anchors.fill: parent

        RowLayout {
            id: leftContentArea
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            visible: false

            spacing: AppStyle.spacingMd

            // FLIconButton {
            //     id: menuButton
            //     iconName: "menu"
            //     Layout.alignment: Qt.AlignVCenter
            //
            //     // Layout.topMargin: 2
            //     Layout.leftMargin: AppStyle.spacingXs
            //     // tooltipText: "Open menu"
            //
            //     RightClickMenu {
            //         id: contextMenu
            //         x: menuButton.width * 1 / 4
            //         y: menuButton.height + 8
            //
            //         RightClickMenu {
            //             title: qsTr("File")
            //             // iconSource: "qrc:/icons/bar-chart"
            //             RightClickMenuItem {
            //                 text: qsTr("Add ROM or Folder")
            //                 // iconSource: "qrc:/icons/bar-chart"
            //                 minWidth: 260
            //                 onTriggered: { /* ... */ }
            //             }
            //             RightClickMenuItem {
            //                 text: qsTr("New Folder")
            //                 // iconSource: "qrc:/icons/bar-chart"
            //                 minWidth: 260
            //                 onTriggered: { /* ... */ }
            //             }
            //             RightClickMenuItem {
            //                 text: qsTr("New Smart Folder")
            //                 // iconSource: "qrc:/icons/bar-chart"
            //                 minWidth: 260
            //                 onTriggered: { /* ... */ }
            //             }
            //         }
            //         RightClickMenuItem {
            //             text: qsTr("Edit")
            //             // iconSource: "qrc:/icons/bar-chart"
            //             minWidth: 260
            //             onTriggered: { /* ... */ }
            //         }
            //         RightClickMenuItem {
            //             text: qsTr("View")
            //             // iconSource: "qrc:/icons/bar-chart"
            //             minWidth: 260
            //             onTriggered: { /* ... */ }
            //         }
            //         RightClickMenuItem {
            //             text: qsTr("RetroAchievements")
            //             // iconSource: "qrc:/icons/bar-chart"
            //             minWidth: 260
            //             onTriggered: { /* ... */ }
            //         }
            //         RightClickMenu {
            //             title: qsTr("Help")
            //             // iconSource: "qrc:/icons/bar-chart"
            //             RightClickMenuItem {
            //                 text: qsTr("Settings")
            //                 // iconSource: "qrc:/icons/bar-chart"
            //                 minWidth: 260
            //                 onTriggered: { /* ... */ }
            //             }
            //             RightClickMenuItem {
            //                 text: qsTr("Troubleshooting")
            //                 // iconSource: "qrc:/icons/bar-chart"
            //                 minWidth: 260
            //                 onTriggered: { /* ... */ }
            //             }
            //             RightClickMenuItem {
            //                 text: qsTr("User Guide")
            //                 // iconSource: "qrc:/icons/bar-chart"
            //                 minWidth: 260
            //                 onTriggered: { /* ... */ }
            //             }
            //             RightClickMenuItem {
            //                 text: qsTr("About Firelight")
            //                 // iconSource: "qrc:/icons/bar-chart"
            //                 minWidth: 260
            //                 onTriggered: { /* ... */ }
            //             }
            //         }
            //     }
            //
            //     onClicked: {
            //         contextMenu.open();
            //     }
            // }

            FLIconButton {
                iconName: "chevron-back"
                Layout.alignment: Qt.AlignVCenter
                Layout.leftMargin: AppStyle.spacingSm
                enabled: Router.canGoBack
                onClicked: Router.back()
            }

            // FLButton {
            //     text: "Home"
            //     iconName: "home"
            //     Layout.alignment: Qt.AlignVCenter
            //     variant: "subtle"
            //     checkable: false
            //     enabled: false
            //     checked: Router.isActive("/home")
            //     onClicked: Router.navigate("/home")
            // }

            FLButton {
                text: "Library"
                iconName: "browse"
                Layout.alignment: Qt.AlignVCenter
                variant: "subtle"
                checkable: false
                checked: Router.isActive("/library")
                onClicked: Router.navigate("/library")
            }

            // FLButton {
            //     text: "Mod Shop"
            //     iconName: "shopping-bag"
            //     Layout.alignment: Qt.AlignVCenter
            //     variant: "subtle"
            //     checkable: false
            //     checked: Router.isActive("/shop")
            //     onClicked: Router.navigate("/shop")
            // }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }

        // RowLayout {
        //     id: centerContentArea
        //     anchors.top: parent.top
        //     anchors.bottom: parent.bottom
        //     anchors.horizontalCenter: parent.horizontalCenter
        //     // width: Math.min(600, Math.max(0, parent.width - Math.max(leftContentArea.width, rightContentArea.width) * 2 - 32))
        //
        //     spacing: AppStyle.spacingMd
        //
        //     FLButton {
        //         text: "Home"
        //         iconName: "home"
        //         Layout.alignment: Qt.AlignVCenter
        //         variant: "subtle"
        //         checkable: false
        //         checked: Router.isActive("/home")
        //         onClicked: Router.navigate("/home")
        //     }
        //
        //     FLButton {
        //         text: "Library"
        //         iconName: "browse"
        //         Layout.alignment: Qt.AlignVCenter
        //         variant: "subtle"
        //         checkable: false
        //         checked: Router.isActive("/library")
        //         onClicked: Router.navigate("/library")
        //     }
        //
        //     FLButton {
        //         text: "Mod Shop"
        //         iconName: "shopping-bag"
        //         Layout.alignment: Qt.AlignVCenter
        //         variant: "subtle"
        //         checkable: false
        //         checked: Router.isActive("/shop")
        //         onClicked: Router.navigate("/shop")
        //     }
        // }

        // TitleBarSearchBar {
        //     id: searchBar
        //     anchors.top: parent.top
        //     anchors.bottom: parent.bottom
        //     anchors.horizontalCenter: parent.horizontalCenter
        //     // Shrinks as the window narrows. The wider panel sets the mirrored
        //     // margin on the shorter side so the bar stays exactly centred
        //     width: Math.min(600, Math.max(0, parent.width - Math.max(leftContentArea.width, rightContentArea.width) * 2 - 32))
        // }

        RowLayout {
            id: rightContentArea
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom

            spacing: AppStyle.spacingMd
            //
            // FLIconButton {
            //     iconName: "search"
            //     Layout.alignment: Qt.AlignVCenter
            // }

            // FLIconButton {
            //     iconName: "controller"
            //     Layout.alignment: Qt.AlignVCenter
            // }

            // FLIconButton {
            //     iconName: "bell"
            //     Layout.alignment: Qt.AlignVCenter
            // }

            FLIconButton {
                iconName: "settings"
                Layout.alignment: Qt.AlignVCenter
                onClicked: Router.navigate("/settings")
            visible: false
            }

            TitleBarProfileButton {
                Layout.fillHeight: true
                Layout.topMargin: AppStyle.spacingXs
                Layout.bottomMargin: AppStyle.spacingXs
                Layout.leftMargin: AppStyle.spacingXs
                Layout.preferredWidth: height
                avatarUrlSource: achievement_manager.avatarUrl
            visible: false
            }

            TitleBarUtilityButtons {
                Layout.fillHeight: true
                Layout.alignment: Qt.AlignVCenter

                onMinimizeClicked: root.minimizeClicked()
                onMaximizeClicked: root.maximizeClicked()
                onCloseClicked: root.closeClicked()
            }
        }
    }
}
