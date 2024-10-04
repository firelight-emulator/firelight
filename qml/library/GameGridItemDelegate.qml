import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

FocusScope {
    id: myDelegate

    signal startGame(entryId: int, hash: string)

    signal openDetails(entryId: int)

    signal manageSaveData(entryId: int)

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

        Keys.onPressed: function (event) {
            if (event.key === Qt.Key_Menu) {
                rightClickMenu.popup(400, 400)
            }
        }

        TapHandler {
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            onTapped: function (event, button) {
                if (button === Qt.RightButton) {
                    rightClickMenu.popup()
                } else if (button === Qt.LeftButton) {
                    // Router.navigateTo("library/" + myDelegate.model.id + "/details")
                    // myDelegate.openDetails(myDelegate.model.id)
                    myDelegate.startGame(myDelegate.model.id, myDelegate.model.contentHash)
                }
            }
        }

        RightClickMenu {
            id: rightClickMenu
            objectName: "rightClickMenu"

            RightClickMenuItem {
                text: "Play " + myDelegate.model.displayName
                onTriggered: {
                    myDelegate.startGame(myDelegate.model.id, myDelegate.model.contentHash)
                }
            }

            RightClickMenuItem {
                enabled: false
                text: "View details"
                onTriggered: {
                    myDelegate.openDetails(myDelegate.model.id)
                }
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
                onTriggered: {
                    myDelegate.manageSaveData(myDelegate.model.id)
                }
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
                radius: 8

                color: "grey"

                Image {
                    id: img
                    source: myDelegate.model.icon1x1SourceUrl
                    fillMode: Image.PreserveAspectFit
                    anchors.fill: parent
                    visible: false
                }

                MultiEffect {
                    source: img
                    anchors.fill: img
                    maskEnabled: true
                    maskSource: mask
                }

                Item {
                    id: mask
                    width: img.width
                    height: img.height
                    layer.enabled: true
                    smooth: true
                    visible: false

                    Rectangle {
                        width: img.width
                        height: img.height
                        topLeftRadius: 8
                        topRightRadius: 8
                        bottomLeftRadius: 8
                        bottomRightRadius: 8
                        color: "black"
                    }
                }
            }
            Text {
                text: myDelegate.model.displayName
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
                text: myDelegate.model.platformId
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