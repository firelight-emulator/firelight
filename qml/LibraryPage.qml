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
                            name: "All games"
                            icon: "\uf53e"
                            playlistId: -1
                        }
                        ListElement {
                            name: "Recently played"
                            icon: "\ue889"
                            playlistId: -1
                        }
                        ListElement {
                            name: "Newly added"
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

                RowLayout {
                    id: playlistHeader
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.right: parent.right
                    height: 40

                    Text {
                        text: "Playlists"
                        Layout.leftMargin: 8
                        font.pointSize: 12
                        font.family: Constants.strongFontFamily
                        color: "#b3b3b3"
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                    }

                    Button {
                        id: createPlaylistButton
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        hoverEnabled: true

                        background: Rectangle {
                            color: createPlaylistButton.pressed ? "black" : (createPlaylistButton.hovered ? "#232323" : "transparent")
                            radius: 50
                        }

                        implicitWidth: 32
                        implicitHeight: 32

                        contentItem: Text {
                            text: "\ue145"
                            font.pixelSize: 24
                            font.family: Constants.symbolFontFamily
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            color: createPlaylistButton.pressed ? "#3e3e3e" : "#b3b3b3"
                        }

                        onClicked: {
                            createPlaylistDialog.open()
                        }

                        ToolTip.visible: createPlaylistButton.hovered
                        ToolTip.text: "Create new playlist"
                        ToolTip.delay: 400
                        ToolTip.timeout: 5000
                    }
                }

                ListView {
                    id: categoryList

                    clip: true
                    boundsBehavior: Flickable.StopAtBounds
                    anchors.left: parent.left
                    anchors.top: playlistHeader.bottom
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    model: playlist_model

                    delegate: Button {
                        id: playlistItemButton
                        background: Rectangle {
                            color: playlistItemButton.checked ?
                                (playlistItemMouse.pressed ?
                                    "#2b2b2b"
                                    : ((playlistItemMouse.containsMouse ?
                                        "#393939"
                                        : "#232323")))
                                : (playlistItemMouse.pressed ?
                                    "#020202"
                                    : (playlistItemMouse.containsMouse ?
                                        "#1a1a1a"
                                        : "transparent"))
                            radius: 4
                        }

                        ButtonGroup.group: buttonGroup

                        width: ListView.view.width

                        padding: 6

                        onClicked: function () {
                            library_short_model.filterOnPlaylistId(model.id)
                            library_short_model.sort(0)
                        }

                        contentItem: Text {
                            text: model.display_name
                            font.pointSize: 12
                            font.family: Constants.regularFontFamily
                            color: "#ffffff"
                        }

                        MouseArea {
                            id: playlistItemMouse
                            anchors.fill: parent
                            acceptedButtons: Qt.RightButton
                            hoverEnabled: true
                            onClicked: function (event) {
                                playlistRightClickMenu.playlistName = model.display_name
                                playlistRightClickMenu.playlistId = model.id
                                playlistRightClickMenu.popup()
                            }
                            cursorShape: Qt.PointingHandCursor
                        }
                    }

                    // delegate: FirelightMenuItem {
                    //     labelText: model.display_name
                    //     labelIcon: ""
                    //     height: 64
                    //     width: categoryList.width
                    //
                    //     onClicked: function () {
                    //         library_short_model.filterOnPlaylistId(model.id)
                    //         library_short_model.sort(0)
                    //     }
                    //
                    //     onRightClicked: function () {
                    //         playlistRightClickMenu.playlistName = model.display_name
                    //         playlistRightClickMenu.playlistId = model.id
                    //         playlistRightClickMenu.popup()
                    //     }
                    //
                    //     ButtonGroup.group: buttonGroup
                    // }
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

                // section.criteria: ViewSection.FirstCharacter
                // section.property: "display_name"
                // section.delegate: Item {
                //     width: libraryList.width
                //     height: 42
                //
                //     Rectangle {
                //         width: libraryList.width
                //         height: 30
                //         anchors.centerIn: parent
                //         color: Constants.colorTestBackground
                //         Text {
                //             anchors.fill: parent
                //             anchors.leftMargin: 12
                //             text: section
                //             color: Constants.colorTestText
                //             font.pointSize: 12
                //             font.family: fontFamilyName
                //             verticalAlignment: Text.AlignVCenter
                //         }
                //     }
                // }
                ScrollBar.vertical: ScrollBar {
                    width: 15
                    interactive: true
                }

                currentIndex: 0

                model: library_short_model
                boundsBehavior: Flickable.StopAtBounds
                // delegate: gameListItem
                delegate: Button {
                    id: libItemButton
                    background: Rectangle {
                        // color: libItemButton.checked ?
                        //     (libItemMouse.pressed ?
                        //         "#2b2b2b"
                        //         : ((libItemMouse.containsMouse ?
                        //             "#393939"
                        //             : "#232323")))
                        //     : (libItemMouse.pressed ?
                        //         "#020202"
                        //         : (libItemMouse.containsMouse ?
                        //             "#1a1a1a"
                        //             : "transparent"))
                        color: "transparent"
                        radius: 4
                    }

                    autoExclusive: true

                    // ButtonGroup.group: buttonGroup

                    width: ListView.view.width

                    padding: 6

                    onClicked: function () {
                        entryClicked(model.id)
                    }

                    contentItem: Text {
                        text: model.display_name
                        font.pointSize: 12
                        font.family: Constants.regularFontFamily
                        color: "#ffffff"
                    }

                    // MouseArea {
                    //     id: libItemMouse
                    //     anchors.fill: parent
                    //     acceptedButtons: Qt.LeftButton | Qt.RightButton
                    //     hoverEnabled: true
                    //     onClicked: function (event) {
                    //         if (event.button === Qt.LeftButton) {
                    //             libItemButton.toggle()
                    //         } else if (event.button === Qt.RightButton) {
                    //             libraryEntryRightClickMenu.popup()
                    //         }
                    //     }
                    //     cursorShape: Qt.PointingHandCursor
                    // }
                }
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