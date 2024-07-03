import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

FocusScope {
    id: myDelegate
    required property var index
    required property var model

    required property real cellWidth
    required property real cellHeight
    required property real cellSpacing

    width: cellWidth
    height: cellHeight
    Button {
        id: button
        padding: 0
        anchors {
            fill: parent
            margins: myDelegate.cellSpacing / 2
        }
        focus: true

        TapHandler {
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            onTapped: function (event, button) {
                if (button === Qt.RightButton) {
                    rightClickMenu.popup()
                } else if (button === Qt.LeftButton) {
                    console.log("Tapped")
                }
            }
        }

        RightClickMenu {
            id: rightClickMenu
            objectName: "rightClickMenu"

            RightClickMenuItem {
                text: "Play " + myDelegate.model.display_name
                onTriggered: {
                    console.log("Edit clicked")
                }
            }

            RightClickMenuItem {
                // enabled: false
                text: "View details"
            }

            MenuSeparator {
                contentItem: Rectangle {
                    implicitWidth: rightClickMenu.width
                    implicitHeight: 1
                    color: "#606060"
                }
            }

            RightClickMenuItem {
                enabled: false
                text: "Manage save data"
                // onTriggered: {
                //     addPlaylistRightClickMenu.entryId = libraryEntryRightClickMenu.entryId
                //     addPlaylistRightClickMenu.popup()
                // }
            }
        }

        hoverEnabled: true

        background: Rectangle {
            objectName: "background"
            color: button.hovered ? "#3a3e45" : "#25282C"
        }

        contentItem: ColumnLayout {
            Rectangle {
                id: image
                Layout.preferredHeight: parent.width
                Layout.fillWidth: true
                Layout.leftMargin: -2
                Layout.rightMargin: -2

                color: "grey"
            }
            Text {
                text: myDelegate.model.display_name
                font.pointSize: 11
                font.weight: Font.Bold
                // font.family: Constants.regularFontFamily
                color: "white"
                Layout.fillWidth: true
                elide: Text.ElideRight
                maximumLineCount: 1
                // Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
            }

            Text {
                text: myDelegate.model.platform_name
                font.pointSize: 10
                font.weight: Font.Medium
                // font.family: Constants.regularFontFamily
                color: "#C2BBBB"
                Layout.fillWidth: true
                // Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
            }
            Item {
                Layout.fillHeight: true
                Layout.fillWidth: true
            }
        }
    }
}