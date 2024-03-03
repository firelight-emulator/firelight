import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FirelightStyle 1.0

Rectangle {
    property string fontFamilyName

    signal entryClicked(string entryId)

    color: "transparent"
    Pane {
        id: content
        background: Rectangle {
            color: Constants.colorTestSurface
            radius: 12
        }

        // padding: 12
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        width: 300

        ButtonGroup {
            id: buttonGroup
            exclusive: true
        }

        Text {
            id: libraryLabel
            text: "LIBRARY"
            color: Constants.colorTestText
            font.pointSize: 8
            font.family: fontFamilyName
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
        }

        ListView {
            id: mainCategoryList
            interactive: false
            spacing: 2
            anchors.left: parent.left
            anchors.top: libraryLabel.bottom
            anchors.topMargin: 4
            anchors.right: parent.right
            height: model.count * 40
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
                height: 40
                width: mainCategoryList.width

                onClicked: function () {
                    library_short_model.filterOnPlaylistId(model.playlistId)
                }

                ButtonGroup.group: buttonGroup
            }
        }

        Text {
            id: divider
            text: "PLAYLISTS"
            color: Constants.colorTestText
            font.pointSize: 8
            font.family: fontFamilyName
            anchors.topMargin: 18
            anchors.top: mainCategoryList.bottom
            anchors.left: parent.left
            anchors.right: parent.right
        }

        ListView {
            id: categoryList
            spacing: 2

            clip: true
            boundsBehavior: Flickable.StopAtBounds
            anchors.left: parent.left
            anchors.topMargin: 4
            anchors.top: divider.bottom
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            model: playlist_model

            delegate: FirelightMenuItem {
                labelText: model.display_name
                labelIcon: ""
                height: 40
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

        // minimumWidth: 200
        // maximumWidth: 400

        // Rectangle {
        //     id: testItem
        //     width: 200
        //     height: 160
        //     color: Constants.colorTestSurfaceContainerLowest
        //     radius: 12
        // }
    }

    Pane {
        id: contentRight
        background: Rectangle {
            color: Constants.colorTestSurface
            radius: 12
        }

        // padding: 12

        anchors.leftMargin: 12
        anchors.left: content.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        ListView {
            id: libraryList
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
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
                        entryClicked(model.id)
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

    FirelightCreatePlaylistDialog {
        id: createPlaylistDialog
        visible: false
        onAccepted: {
            playlist_model.addPlaylist(createPlaylistDialog.text)
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

    Menu {
        id: playlistRightClickMenu
        property string playlistName: ""
        property int playlistId: -1

        padding: 4

        background: Rectangle {
            implicitWidth: 200
            implicitHeight: playlistRightClickMenu.count * 40
            color: Constants.colorTestSurfaceContainerLowest
            radius: 6
        }

        MenuItem {
            background: Rectangle {
                implicitWidth: 200
                implicitHeight: 40
                radius: 6
                color: renameHover.hovered ? Constants.colorTestSurfaceVariant : "transparent"

                HoverHandler {
                    id: renameHover
                    acceptedDevices: PointerDevice.Mouse
                }
            }

            contentItem: Text {
                text: "Rename"
                color: Constants.colorTestTextActive
                font.pointSize: 12
                font.family: Constants.regularFontFamily
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
            }
        }

        MenuItem {
            onTriggered: {
                deletePlaylistDialog.playlistName = playlistRightClickMenu.playlistName
                deletePlaylistDialog.playlistId = playlistRightClickMenu.playlistId
                deletePlaylistDialog.open()
            }

            background: Rectangle {
                implicitWidth: 200
                implicitHeight: 40
                radius: 6
                color: deleteHover.hovered ? Constants.colorTestSurfaceVariant : "transparent"

                HoverHandler {
                    id: deleteHover
                    acceptedDevices: PointerDevice.Mouse
                }
            }

            contentItem: Text {
                text: "Delete Playlist"
                color: deleteHover.hovered ? Constants.color_errorContainer : Constants.colorTestTextActive
                font.pointSize: 12
                font.family: Constants.regularFontFamily
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
            }
        }
    }

}