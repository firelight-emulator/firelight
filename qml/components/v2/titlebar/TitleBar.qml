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

            spacing: 16

            IconButton {
                icon.width: 24
                icon.height: 24
                icon.source: "qrc:/icons/menu"
                Layout.alignment: Qt.AlignVCenter
                Layout.topMargin: 2
                Layout.leftMargin: 6
                // tooltipText: "Open menu"
                opacity: 0.7

                onClicked: {
                    root.menuButtonClicked()
                }
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
            // margin on the shorter side so the bar stays exactly centred.
            width: Math.min(600, Math.max(0,
                parent.width - Math.max(leftContentArea.width, rightContentArea.width) * 2 - 32))
        }

        RowLayout {
            id: rightContentArea
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom

            spacing: 24

            IconButton {
                icon.width: 24
                icon.height: 24
                icon.source: "qrc:/icons/settings"
                Layout.alignment: Qt.AlignVCenter
                Layout.topMargin: 2
                // tooltipText: "Open menu"
                opacity: 0.7
            }

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