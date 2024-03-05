import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FirelightStyle 1.0

Rectangle {
    property string fontFamilyName

    signal entryClicked(string entryId)

    color: "transparent"

    ButtonGroup {
        id: buttonGroup
        exclusive: true
    }

    SplitView {
        id: navigation
        orientation: Qt.Horizontal
        anchors.fill: parent

        handle: Item {
            id: handle
            implicitWidth: 9
            implicitHeight: parent.height

            Rectangle {
                anchors.centerIn: parent
                implicitWidth: 1
                implicitHeight: parent.height - 10
                color: handle.SplitHandle.pressed ? "#f1f1f1"
                    : (handle.SplitHandle.hovered ? "#727272" : "transparent")

                Behavior on color {
                    ColorAnimation {
                        duration: 150
                        easing.type: Easing.InOutQuad
                    }

                }
            }
        }

        Item {
            SplitView.minimumWidth: 260
            SplitView.maximumWidth: parent.width / 2
            Pane {
                id: content
                background: Rectangle {
                    color: Constants.surface_color
                    radius: 8
                }

                padding: 8

                anchors.left: parent.left
                anchors.top: parent.top
                anchors.right: parent.right

                ListView {
                    id: mainCategoryList
                    interactive: false

                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.right: parent.right

                    implicitHeight: model.count * 50
                    model: ListModel {
                        ListElement {
                            name: "All Games"
                            icon: "\uf53e"
                            playlistId: -1
                        }
                        ListElement {
                            name: "Recently Played"
                            icon: "\ue889"
                            playlistId: -1
                        }
                        ListElement {
                            name: "Newly Added"
                            icon: "\ue838"
                            playlistId: -1
                        }
                    }

                    delegate: FirelightMenuItem {
                        labelText: model.name
                        labelIcon: model.icon
                        height: 50
                        width: mainCategoryList.width
                        leftPadding: 12
                        rightPadding: 12

                        onClicked: function () {
                            library_short_model.filterOnPlaylistId(model.playlistId)
                        }

                        ButtonGroup.group: buttonGroup
                    }
                }
            }

            Pane {
                id: playlistPane
                background: Rectangle {
                    color: Constants.surface_color
                    radius: 8
                }

                padding: 8

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.topMargin: 8
                anchors.top: content.bottom
                anchors.bottom: parent.bottom

                ListView {
                    id: categoryList

                    clip: true
                    boundsBehavior: Flickable.StopAtBounds
                    anchors.fill: parent
                    model: playlist_model

                    delegate: FirelightMenuItem {
                        labelText: model.display_name
                        labelIcon: ""
                        height: 64
                        width: categoryList.width

                        onClicked: function () {
                            library_short_model.filterOnPlaylistId(model.id)
                            library_short_model.sort(0)
                        }

                        onRightClicked: function () {
                            playlistRightClickMenu.playlistName = model.display_name
                            playlistRightClickMenu.playlistId = model.id
                            playlistRightClickMenu.popup()
                        }

                        ButtonGroup.group: buttonGroup
                    }
                }

                Button {
                    id: testItem
                    width: 160
                    height: 48

                    background: Rectangle {
                        color: Constants.colorTestCardActive
                        radius: 50
                    }

                    anchors.right: parent.right
                    anchors.bottom: parent.bottom

                    Behavior on scale {
                        NumberAnimation {
                            duration: 100
                            easing.type: Easing.OutBounce
                        }
                    }

                    onClicked: {
                        createPlaylistDialog.open()
                    }

                    scale: pressed ? 0.93 : 1

                    Text {
                        id: buttonIcon
                        text: "\ue03b"
                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        leftPadding: 12
                        rightPadding: 12
                        // width: 24

                        font.family: Constants.symbolFontFamily
                        font.pixelSize: 24
                        color: Constants.colorTestTextActive
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    Text {
                        id: buttonText
                        text: "New Playlist"
                        anchors.left: buttonIcon.right
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        anchors.right: parent.right
                        font.pointSize: 12
                        font.family: Constants.regularFontFamily
                        color: Constants.colorTestTextActive
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                    }

                    HoverHandler {
                        id: mouse
                        acceptedDevices: PointerDevice.Mouse
                        cursorShape: Qt.PointingHandCursor
                    }

                }

            }
        }
        Pane {
            id: contentRight
            background: Rectangle {
                color: Constants.surface_color
                radius: 8
            }

            padding: 8

            ListView {
                id: libraryList
                anchors.fill: parent
                clip: true

                section.criteria: ViewSection.FirstCharacter
                section.property: "display_name"
                section.delegate: Item {
                    width: libraryList.width
                    height: 42

                    Rectangle {
                        width: libraryList.width
                        height: 30
                        anchors.centerIn: parent
                        color: Constants.colorTestBackground
                        Text {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            text: section
                            color: Constants.colorTestText
                            font.pointSize: 12
                            font.family: fontFamilyName
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }
                ScrollBar.vertical: ScrollBar {
                    width: 15
                    interactive: true
                }

                currentIndex: 0

                model: library_short_model
                boundsBehavior: Flickable.StopAtBounds
                delegate: gameListItem
                // preferredHighlightBegin: height / 3
                // preferredHighlightEnd: 2 * (height / 3) + currentItem.height
            }

            Component {
                id: gameListItem

                Rectangle {
                    id: wrapper

                    width: ListView.view.width
                    height: 40
                    radius: 12

                    color: mouseArea.containsMouse ? Constants.colorTestCard : "transparent"

                    MouseArea {
                        id: mouseArea
                        hoverEnabled: true
                        anchors.fill: parent

                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            libraryEntryRightClickMenu.popup()
                            // entryClicked(model.id)
                            // gameLoader.loadGame(model.id)
                        }
                    }

                    Text {
                        id: label
                        anchors.left: parent.left
                        anchors.leftMargin: 12
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        width: (parent.width / 3) * 2

                        font.pointSize: 12
                        font.family: fontFamilyName
                        text: model.display_name
                        color: Constants.colorTestText
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                    }

                    Text {
                        id: platformLabel
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        anchors.left: label.right

                        // font.family: lexendLight.name
                        font.pointSize: 10
                        text: model.platform_name
                        color: "#989898"
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }
    }


    // Pane {
    //     id: playlistPane
    //     background: Rectangle {
    //         color: Constants.surface_color
    //         radius: 8
    //     }
    // }


    FirelightCreatePlaylistDialog {
        id: createPlaylistDialog
        visible: false
        onAccepted: {
            playlist_model.addPlaylist(createPlaylistDialog.text)
        }
    }

    FirelightRenamePlaylistDialog {
        id: renamePlaylistDialog
        visible: false
        onAccepted: {
            playlist_model.renamePlaylist(renamePlaylistDialog.playlistId, renamePlaylistDialog.text)
        }
    }

    FirelightDialog {
        id: deletePlaylistDialog

        property string playlistName: ""
        property int playlistId: -1

        text: "Are you sure you want to delete the playlist \"" + playlistName + "\"?"
        visible: false

        onAccepted: {
            playlist_model.removePlaylist(playlistId)
        }
    }

    FirelightRightClickMenu {
        id: libraryEntryRightClickMenu

        FirelightRightClickMenuItem {
            labelText: "Rename"
        }

        FirelightRightClickMenu {
            id: addPlaylistRightClickMenu

            title: "Add to playlist"
            FirelightRightClickMenuItem {
                labelText: "Playlist One"
            }

            FirelightRightClickMenuItem {
                labelText: "Numbah Two"
            }

        }

        // FirelightRightClickMenuItem {
        //     labelText: "Add to playlist"
        //     cascade: true
        //
        //
        // }
    }

    FirelightRightClickMenu {
        id: playlistRightClickMenu
        property string playlistName: ""
        property int playlistId: -1

        FirelightRightClickMenuItem {
            onTriggered: {
                renamePlaylistDialog.text = playlistRightClickMenu.playlistName
                renamePlaylistDialog.playlistId = playlistRightClickMenu.playlistId
                renamePlaylistDialog.open()
            }

            labelText: "Rename"
        }

        FirelightRightClickMenuItem {
            onTriggered: {
                deletePlaylistDialog.playlistName = playlistRightClickMenu.playlistName
                deletePlaylistDialog.playlistId = playlistRightClickMenu.playlistId
                deletePlaylistDialog.open()
            }

            labelText: "Delete playlist"
        }
    }

}