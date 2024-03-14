import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQml.Models
import FirelightStyle 1.0

Pane {
    property string fontFamilyName

    signal entryClicked(string entryId)

    background: Rectangle {
        color: "transparent"
    }

    ButtonGroup {
        id: buttonGroup
        exclusive: true
    }

    SplitView {
        id: navigation
        orientation: Qt.Horizontal
        anchors.fill: parent

        handle: FirelightSplitViewHandle {
        }

        Item {
            SplitView.minimumWidth: 260
            SplitView.maximumWidth: parent.width / 2
            ContentPane {
                id: content

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
                        // ListElement {
                        //     name: "Recently played"
                        //     icon: "\ue889"
                        //     playlistId: -1
                        // }
                        // ListElement {
                        //     name: "Newly added"
                        //     icon: "\ue838"
                        //     playlistId: -1
                        // }
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
                            // if (model.name === "All games") {
                            //     library_short_model.sortByDisplayName()
                            //     // } else if (model.name === "Recently played") {
                            //     //     library_short_model.sortByLastPlayed()
                            // } else if (model.name === "Newly added") {
                            //     library_short_model.sortByCreatedAt()
                            // }
                        }

                        ButtonGroup.group: buttonGroup
                    }
                }
            }

            ContentPane {
                id: playlistPane

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
                        text: "Folders"
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
                        ToolTip.text: "Create folder"
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
                            opacity: playlistItemButton.checked ?
                                (playlistItemMouse.pressed ?
                                    0.3
                                    : ((playlistItemMouse.containsMouse ?
                                        0.2
                                        : 0.2)))
                                : (playlistItemMouse.pressed ?
                                    0.2
                                    : (playlistItemMouse.containsMouse ?
                                        0.1
                                        : "transparent"))
                            color: "white"
                            radius: 4
                        }

                        checkable: true
                        ButtonGroup.group: buttonGroup

                        width: ListView.view.width

                        padding: 6

                        onClicked: function () {
                            library_short_model.filterOnPlaylistId(model.id)
                            library_short_model.sort(0)
                        }

                        contentItem: CarouselText {
                            hovered: playlistItemMouse.containsMouse
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
                }

            }
        }
        ContentPane {
            id: contentRight

            Text {
                text: "no gams here"
                font.pointSize: 12
                font.family: Constants.regularFontFamily
                color: "#b3b3b3"
                anchors.centerIn: parent
                visible: libraryList.count === 0
            }

            ListView {
                id: libraryList
                anchors.fill: parent
                clip: true
                focus: true

                // header: Rectangle {
                //     width: libraryList.width
                //     height: 30
                //     color: "grey"
                //     Text {
                //         anchors.fill: parent
                //         anchors.leftMargin: 12
                //         text: "All games"
                //         color: "black"
                //         font.pointSize: 12
                //         font.family: Constants.regularFontFamily
                //     }
                // }
                //
                // headerPositioning: ListView.PullBackHeader

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
                ScrollBar.vertical
                    :
                    ScrollBar {
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
                        color: libItemMouse.containsMouse ? "white" : "transparent"
                        opacity: 0.2
                        radius: 4
                    }
                    autoExclusive: true

                    width: ListView.view.width

                    padding: 6

                    onClicked: function () {
                        entryClicked(model.id)
                    }

                    contentItem: Text {
                        text: model.display_name
                        font.pointSize: 12
                        font.family: Constants.strongFontFamily
                        color: "#ffffff"
                    }

                    MouseArea {
                        id: libItemMouse
                        anchors.fill: parent
                        acceptedButtons: Qt.LeftButton | Qt.RightButton
                        hoverEnabled: true
                        onClicked: function (event) {
                            if (event.button === Qt.LeftButton) {
                                libItemButton.onClicked()
                            } else if (event.button === Qt.RightButton) {
                                libraryEntryRightClickMenu.entryId = model.id
                                libraryEntryRightClickMenu.popup()
                            }
                        }
                        cursorShape: Qt.PointingHandCursor
                    }
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

        text: "Are you sure you want to delete the folder \"" + playlistName + "\"?"
        visible: false

        onAccepted: {
            playlist_model.removePlaylist(playlistId)
        }
    }

    // Instantiator {
    //     model: playlist_model
    //
    //     delegate: FirelightRightClickMenuItem {
    //         parent: addPlaylistRightClickMenu
    //         labelText: model.display_name
    //         onTriggered: {
    //             // playlist_model.addEntryToPlaylist(model.id, libraryList.currentItem.id)
    //         }
    //     }
    // }

    RightClickMenu {
        id: libraryEntryRightClickMenu

        property int entryId: -1

        RightClickMenuItem {
            text: "Manage save files"
            onTriggered: {
                manageSavefilesDialog.open()
            }
        }

        RightClickMenu {
            id: addPlaylistRightClickMenu
            enabled: ins.count > 0

            title: "Add to folder"

            Instantiator {
                id: ins
                model: playlist_model
                delegate: RightClickMenuItem {
                    text: model.display_name
                    onTriggered: {
                        playlist_model.addEntryToPlaylist(model.id, libraryEntryRightClickMenu.entryId)
                        library_model.updatePlaylistsForEntry(libraryEntryRightClickMenu.entryId)
                        // Add your action here
                    }
                }

                onObjectAdded: function (index, object) {
                    addPlaylistRightClickMenu.insertItem(index, object)
                }
                onObjectRemoved: function (index, object) {
                    addPlaylistRightClickMenu.removeItem(object)
                }
            }
        }
    }

    RightClickMenu {
        id: playlistRightClickMenu
        property string playlistName: ""
        property int playlistId: -1

        RightClickMenuItem {
            onTriggered: {
                renamePlaylistDialog.text = playlistRightClickMenu.playlistName
                renamePlaylistDialog.playlistId = playlistRightClickMenu.playlistId
                renamePlaylistDialog.open()
            }

            text: "Rename"
        }

        RightClickMenuItem {
            onTriggered: {
                deletePlaylistDialog.playlistName = playlistRightClickMenu.playlistName
                deletePlaylistDialog.playlistId = playlistRightClickMenu.playlistId
                deletePlaylistDialog.open()
            }

            text: "Delete folder"
        }
    }

    ManageSavefilesDialog {
        id: manageSavefilesDialog
        visible: false
    }

}