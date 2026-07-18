import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import QtQuick.Layouts 1.0
import Firelight 1.0

FocusScope {
    id: root
    signal closeClicked()
    signal maximizeClicked()
    signal minimizeClicked()
    signal menuButtonClicked()

    function activateSearch() {
        searchBar.forceActiveFocus()
    }

    Item {
        objectName: "Top Bar Drag Area"
        anchors.fill: parent
        z: -1

        DragHandler {
            target: null
            grabPermissions: DragHandler.ApprovesTakeOverByAnything
            onActiveChanged: if (active) root.Window.window.startSystemMove()
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

            spacing: AppStyle.spacingLg

            FLIconButton {
                id: menuButton
                iconName: "menu"
                Layout.alignment: Qt.AlignVCenter

                // Layout.topMargin: 2
                Layout.leftMargin: 6
                // tooltipText: "Open menu"

                RightClickMenu {
                    id: contextMenu
                    x: menuButton.width * 1 / 4
                    y: menuButton.height + 8

                    RightClickMenu {
                        title: qsTr("File")
                        // iconSource: "qrc:/icons/bar-chart"
                        RightClickMenuItem {
                            text: qsTr("Add ROM or Folder")
                            // iconSource: "qrc:/icons/bar-chart"
                            minWidth: 260
                            onTriggered: { /* ... */ }
                        }
                        RightClickMenuItem {
                            text: qsTr("New Folder")
                            // iconSource: "qrc:/icons/bar-chart"
                            minWidth: 260
                            onTriggered: { /* ... */ }
                        }
                        RightClickMenuItem {
                            text: qsTr("New Smart Folder")
                            // iconSource: "qrc:/icons/bar-chart"
                            minWidth: 260
                            onTriggered: { /* ... */ }
                        }
                    }
                    RightClickMenuItem {
                        text: qsTr("Edit")
                        // iconSource: "qrc:/icons/bar-chart"
                        minWidth: 260
                        onTriggered: { /* ... */ }
                    }
                    RightClickMenuItem {
                        text: qsTr("View")
                        // iconSource: "qrc:/icons/bar-chart"
                        minWidth: 260
                        onTriggered: { /* ... */ }
                    }
                    RightClickMenuItem {
                        text: qsTr("RetroAchievements")
                        // iconSource: "qrc:/icons/bar-chart"
                        minWidth: 260
                        onTriggered: { /* ... */ }
                    }
                    RightClickMenu {
                        title: qsTr("Help")
                        // iconSource: "qrc:/icons/bar-chart"
                        RightClickMenuItem {
                            text: qsTr("Settings")
                            // iconSource: "qrc:/icons/bar-chart"
                            minWidth: 260
                            onTriggered: { /* ... */ }
                        }
                        RightClickMenuItem {
                            text: qsTr("Troubleshooting")
                            // iconSource: "qrc:/icons/bar-chart"
                            minWidth: 260
                            onTriggered: { /* ... */ }
                        }
                        RightClickMenuItem {
                            text: qsTr("User Guide")
                            // iconSource: "qrc:/icons/bar-chart"
                            minWidth: 260
                            onTriggered: { /* ... */ }
                        }
                        RightClickMenuItem {
                            text: qsTr("About Firelight")
                            // iconSource: "qrc:/icons/bar-chart"
                            minWidth: 260
                            onTriggered: { /* ... */ }
                        }
                    }
                }

                onClicked: {
                    contextMenu.open()
                }
            }

            FLIconButton {
                iconName: "chevron-back"
                Layout.alignment: Qt.AlignVCenter
                enabled: Router.canGoBack
                onClicked: Router.back()
            }

            FLIconButton {
                iconName: "chevron-forward"
                Layout.alignment: Qt.AlignVCenter
                enabled: Router.canGoForward
                onClicked: Router.forward()
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }

        TitleBarSearchBar {
            id: searchBar
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            // Shrinks as the window narrows. The wider panel sets the mirrored
            // margin on the shorter side so the bar stays exactly centred
            width: Math.min(600, Math.max(0,
                parent.width - Math.max(leftContentArea.width, rightContentArea.width) * 2 - 32))
        }

        RowLayout {
            id: rightContentArea
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom

            spacing: AppStyle.spacingXl

            TitleBarProfileButton {
                Layout.fillHeight: true
                Layout.preferredWidth: height
                avatarUrlSource: achievement_manager.avatarUrl
            }

            TitleBarUtilityButtons {
                Layout.fillHeight: true
                Layout.alignment: Qt.AlignVCenter
                Layout.leftMargin: -(parent.spacing / 2)

                onMinimizeClicked: root.minimizeClicked()
                onMaximizeClicked: root.maximizeClicked()
                onCloseClicked: root.closeClicked()
            }
        }

    }
}
