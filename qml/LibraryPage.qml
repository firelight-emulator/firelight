import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQml.Models
import FirelightStyle 1.0

Item {
    id: root

    signal entryClicked(int entryId)

    Pane {
        id: header
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        background: Item {
        }

        horizontalPadding: 0
        verticalPadding: 6

        Column {
            anchors.fill: parent
            spacing: 8

            Text {
                text: "Library"
                color: "#353535"
                font.pointSize: 22
                font.family: Constants.semiboldFontFamily
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
            }
        }
    }


    // Pane {
    //     id: folders
    //     background: Rectangle {
    //         color: "white"
    //         radius: 12
    //     }
    //     anchors.topMargin: 4
    //     anchors.top: header.bottom
    //     anchors.left: parent.left
    //     anchors.bottom: parent.bottom
    //     width: 200
    //
    //     ColumnLayout {
    //         spacing: 8
    //
    //         Button {
    //             id: allButton
    //             background: Item {
    //             }
    //             Layout.alignment: Qt.AlignLeft | Qt.AlignTop
    //
    //             padding: 0
    //
    //             checkable: true
    //             autoExclusive: true
    //             checked: true
    //
    //             contentItem: Text {
    //                 text: "All games"
    //                 color: "#353535"
    //                 opacity: allButton.checked ? 1 : 0.5
    //                 font.pointSize: 12
    //                 font.family: Constants.semiboldFontFamily
    //                 horizontalAlignment: Text.AlignLeft
    //                 verticalAlignment: Text.AlignVCenter
    //             }
    //
    //             onClicked: function () {
    //                 library_short_model.sortByDisplayName()
    //             }
    //
    //             HoverHandler {
    //                 cursorShape: Qt.PointingHandCursor
    //             }
    //         }
    //
    //         Button {
    //             id: allButton2
    //             background: Item {
    //             }
    //             Layout.alignment: Qt.AlignLeft | Qt.AlignTop
    //
    //             padding: 0
    //
    //             checkable: true
    //             autoExclusive: true
    //
    //             contentItem: Text {
    //                 text: "Nintendo 64"
    //                 color: "#353535"
    //                 opacity: allButton2.checked ? 1 : 0.5
    //                 font.pointSize: 12
    //                 font.family: Constants.semiboldFontFamily
    //                 horizontalAlignment: Text.AlignLeft
    //                 verticalAlignment: Text.AlignVCenter
    //             }
    //
    //             onClicked: function () {
    //                 library_short_model.sortByDisplayName()
    //             }
    //
    //             HoverHandler {
    //                 cursorShape: Qt.PointingHandCursor
    //             }
    //         }
    //
    //         Button {
    //             id: recentButton2
    //             background: Item {
    //             }
    //             Layout.alignment: Qt.AlignLeft | Qt.AlignTop
    //
    //             padding: 0
    //
    //             checkable: true
    //             autoExclusive: true
    //
    //             contentItem: Text {
    //                 text: "Game Boy Advance"
    //                 color: "#353535"
    //                 opacity: recentButton2.checked ? 1 : 0.5
    //                 font.pointSize: 12
    //                 font.family: Constants.semiboldFontFamily
    //                 horizontalAlignment: Text.AlignLeft
    //                 verticalAlignment: Text.AlignVCenter
    //             }
    //
    //             onClicked: function () {
    //                 library_short_model.sortByLastPlayedAt()
    //             }
    //
    //             HoverHandler {
    //                 cursorShape: Qt.PointingHandCursor
    //             }
    //         }
    //
    //         Button {
    //             id: newButton2
    //             background: Item {
    //             }
    //             Layout.alignment: Qt.AlignLeft | Qt.AlignTop
    //
    //             padding: 0
    //
    //             checkable: true
    //             autoExclusive: true
    //
    //             contentItem: Text {
    //                 text: "Sega Genesis"
    //                 color: "#353535"
    //                 opacity: newButton2.checked ? 1 : 0.5
    //                 font.pointSize: 12
    //                 font.family: Constants.semiboldFontFamily
    //                 horizontalAlignment: Text.AlignLeft
    //                 verticalAlignment: Text.AlignVCenter
    //             }
    //
    //             onClicked: function () {
    //                 library_short_model.sortByCreatedAt()
    //             }
    //
    //             HoverHandler {
    //                 cursorShape: Qt.PointingHandCursor
    //             }
    //         }
    //
    //         Item {
    //             Layout.fillWidth: true
    //         }
    //     }
    // }

    Pane {
        background: Rectangle {
            color: "transparent"
            radius: 12
        }
        // anchors.leftMargin: 12
        anchors.topMargin: 4
        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        // Text {
        //     text: "no gams here"
        //     font.pointSize: 12
        //     font.family: Constants.regularFontFamily
        //     color: "#b3b3b3"
        //     anchors.centerIn: parent
        //     visible: libraryList.count === 0
        // }

        // RowLayout {
        //     id: sortByThing
        //     anchors.top: parent.top
        //     anchors.left: parent.left
        //     anchors.right: parent.right
        //     height: 48
        //     spacing: 8
        //
        //     Item {
        //         Layout.fillWidth: true
        //     }
        //
        //     Text {
        //         Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
        //         text: "sort:"
        //         color: "#dadada"
        //         font.pointSize: 10
        //         font.family: Constants.regularFontFamily
        //         horizontalAlignment: Text.AlignLeft
        //         verticalAlignment: Text.AlignVCenter
        //     }
        //
        //     ComboBox {
        //         background: Rectangle {
        //             color: "white"
        //             opacity: 0.9
        //             radius: 4
        //         }
        //         Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
        //         Layout.preferredWidth: 100
        //         model: ["A-Z", "Last Played", "Newest"]
        //     }
        //
        //     Item {
        //         Layout.preferredWidth: 10
        //     }
        //
        //     Text {
        //         Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
        //         text: "show:"
        //         color: "#dadada"
        //         font.pointSize: 10
        //         font.family: Constants.regularFontFamily
        //         horizontalAlignment: Text.AlignLeft
        //         verticalAlignment: Text.AlignVCenter
        //     }
        //
        //     ComboBox {
        //         background: Rectangle {
        //             color: "white"
        //             opacity: 0.9
        //             radius: 4
        //         }
        //         Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
        //         Layout.preferredWidth: 100
        //         model: ["List", "Details", "Grid"]
        //     }
        // }

        GridView {
            id: libraryGrid
            anchors.fill: parent
            cellWidth: cellContentWidth + cellSpacing
            cellHeight: cellContentHeight + cellSpacing

            readonly property int cellSpacing: 12
            readonly property int cellContentWidth: 180
            readonly property int cellContentHeight: 260

            currentIndex: 0

            model: library_short_model
            boundsBehavior: Flickable.StopAtBounds
            keyNavigationEnabled: true
            // delegate: gameListItem

            highlight: Item {
            }

            delegate: Item {
                id: rootItem
                width: libraryGrid.cellContentWidth
                height: libraryGrid.cellContentHeight

                Button {
                    id: libItemButton
                    anchors {
                        fill: parent
                        margins: libraryGrid.cellSpacing / 2
                    }
                    // bottomInset: 12
                    // rightInset: 12
                    // leftInset: 12
                    // topInset: 12
                    //
                    // padding: 12

                    // padding: 8
                    // verticalPadding: 0
                    // horizontalPadding: 0

                    background: Rectangle {
                        color: libItemButton.hovered ? "#3a3e45" : "#25282C"
                        radius: 6
                        border.color: rootItem.GridView.isCurrentItem ? "white" : "transparent"
                        border.width: 2
                    }

                    TapHandler {
                        acceptedButtons: Qt.RightButton
                        onTapped: {
                            rootItem.GridView.view.currentIndex = index
                            libraryEntryRightClickMenu.popup()
                        }
                    }

                    contentItem: ColumnLayout {
                        id: colLayout
                        spacing: 0
                        anchors {
                            fill: parent
                            margins: 3
                        }

                        // Image {
                        //     asynchronous: true
                        //     source: "file:system/_img/pmbox.jpg"
                        //     // Layout.fillWidth: true
                        //     // Layout.preferredHeight: 180
                        //
                        //     // implicitWidth: libItemMouse.hovered ? parent.width + 10 : parent.width
                        //     // implicitHeight: libItemMouse.hovered ? parent.height + 10 : parent.height
                        //
                        //     // scale: libItemMouse.hovered ? 1.02 : 1
                        //
                        //     // Behavior on scale {
                        //     //     NumberAnimation {
                        //     //         duration: 200
                        //     //         easing.type: Easing.InOutQuad
                        //     //     }
                        //     // }
                        //
                        //     width: 200
                        //     height: 200
                        //
                        //     sourceSize.width: 180
                        //     sourceSize.height: 180
                        //
                        //     horizontalAlignment: Image.AlignLeft
                        //
                        //     fillMode: Image.PreserveAspectCrop
                        // }

                        Rectangle {
                            color: "grey"
                            Layout.fillWidth: true
                            Layout.preferredHeight: colLayout.width

                            Text {
                                text: "Box art coming soon :)"
                                font.pointSize: 9
                                // font.weight: Font.Light
                                font.family: Constants.regularFontFamily
                                color: "#232323"
                                anchors.centerIn: parent
                                // Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
                            }
                        }


                        // Rectangle {
                        //     Layout.fillWidth: true
                        //     Layout.preferredHeight: 180
                        //
                        //     color: "grey"
                        //
                        //     topLeftRadius: 6
                        //     topRightRadius: 6
                        //
                        //     clip: true
                        //
                        //     // Image {
                        //     //     asynchronous: true
                        //     //     source: "file:system/_img/pmbox.jpg"
                        //     //     anchors.fill: parent
                        //     //
                        //     //     // implicitWidth: libItemMouse.hovered ? parent.width + 10 : parent.width
                        //     //     // implicitHeight: libItemMouse.hovered ? parent.height + 10 : parent.height
                        //     //
                        //     //     // scale: libItemMouse.hovered ? 1.02 : 1
                        //     //
                        //     //     // Behavior on scale {
                        //     //     //     NumberAnimation {
                        //     //     //         duration: 200
                        //     //     //         easing.type: Easing.InOutQuad
                        //     //     //     }
                        //     //     // }
                        //     //
                        //     //     width: 20
                        //     //     height: 20
                        //     //
                        //     //     horizontalAlignment: Image.AlignLeft
                        //     //
                        //     //     fillMode: Image.PreserveAspectCrop
                        //     // }
                        // }


                        Pane {
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            // clip: true
                            background: Item {
                            }
                            padding: 8
                            ColumnLayout {
                                anchors.fill: parent
                                spacing: 2
                                Text {
                                    text: model.display_name
                                    font.pointSize: 11
                                    font.weight: Font.Bold
                                    font.family: Constants.regularFontFamily
                                    color: "white"
                                    Layout.fillWidth: true
                                    elide: Text.ElideRight
                                    maximumLineCount: 1
                                    // Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
                                }
                                Text {
                                    text: model.platform_name
                                    font.pointSize: 10
                                    font.weight: Font.Medium
                                    font.family: Constants.regularFontFamily
                                    color: "#C2BBBB"
                                    Layout.fillWidth: true
                                    // Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
                                }
                                Item {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                }
                            }

                        }
                    }

                    // hoverEnabled: true
                    // autoExclusive: true

                    // width: ListView.view.width

                    Keys.onReturnPressed: {
                        doubleClicked()
                    }

                    // verticalPadding: 8
                    //
                    onClicked: function () {
                        rootItem.GridView.view.currentIndex = model.index
                        // libraryEntryRightClickMenu.popup()
                    }

                    onDoubleClicked: function () {
                        entryClicked(model.id)
                    }


                    RightClickMenu {
                        id: libraryEntryRightClickMenu

                        RightClickMenuItem {
                            text: "Play " + model.display_name
                            // onTriggered: {
                            //     addPlaylistRightClickMenu.entryId = libraryEntryRightClickMenu.entryId
                            //     addPlaylistRightClickMenu.popup()
                            // }
                        }

                        RightClickMenuItem {
                            text: "View details"
                            // onTriggered: {
                            //     addPlaylistRightClickMenu.entryId = libraryEntryRightClickMenu.entryId
                            //     addPlaylistRightClickMenu.popup()
                            // }
                        }

                        MenuSeparator {
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

                        MenuSeparator {
                        }

                        RightClickMenuItem {
                            enabled: false
                            text: "Manage save data"
                            // onTriggered: {
                            //     addPlaylistRightClickMenu.entryId = libraryEntryRightClickMenu.entryId
                            //     addPlaylistRightClickMenu.popup()
                            // }
                        }

                        // MenuSeparator {
                        // }

                        // RightClickMenuItem {
                        //     text: "Show in filesystem"
                        //     // onTriggered: {
                        //     //     addPlaylistRightClickMenu.entryId = libraryEntryRightClickMenu.entryId
                        //     //     addPlaylistRightClickMenu.popup()
                        //     // }
                        // }


                    }

                    // contentItem: Rectangle {
                    //     color: "#25282C"
                    // }
                }
            }
        }

        // ListView {
        //     id: libraryList
        //     // anchors.top: sortByThing.bottom
        //     anchors.fill: parent
        //     clip: true
        //
        //     property real titleColumnWidth: libraryList.width * 2 / 5
        //     property real platformColumnWidth: libraryList.width / 5
        //     property real filenameColumnWidth: libraryList.width * 2 / 5
        //
        //     // header: ColumnLayout {
        //     //     spacing: 4
        //     //
        //     //     RowLayout {
        //     //         id: headerRow
        //     //         // Layout.leftMargin: 8
        //     //         Layout.leftMargin: 8
        //     //         Layout.fillWidth: true
        //     //
        //     //         // anchors.fill: parent
        //     //         // Text {
        //     //         //     id: favoriteSection
        //     //         //     text: "\ue87d"
        //     //         //     font.family: Constants.symbolFontFamily
        //     //         //     font.pointSize: 12
        //     //         //     color: "#b3b3b3"
        //     //         //     Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
        //     //         //     Layout.fillHeight: true
        //     //         //     Layout.preferredWidth: 24
        //     //         //     topPadding: 2
        //     //         //     horizontalAlignment: Text.AlignLeft
        //     //         //     verticalAlignment: Text.AlignVCenter
        //     //         // }
        //     //         Text {
        //     //             id: titleSection
        //     //             text: "Title"
        //     //             font.pointSize: 10
        //     //             font.family: Constants.regularFontFamily
        //     //             color: "black"
        //     //             opacity: 0.7
        //     //             Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
        //     //             Layout.fillHeight: true
        //     //             Layout.fillWidth: true
        //     //             // Layout.preferredWidth: 300
        //     //             Layout.minimumWidth: libraryList.titleColumnWidth
        //     //             Layout.horizontalStretchFactor: 3
        //     //             horizontalAlignment: Text.AlignLeft
        //     //             verticalAlignment: Text.AlignVCenter
        //     //         }
        //     //
        //     //         Text {
        //     //             id: platformSection
        //     //             text: "Platform"
        //     //             font.pointSize: 10
        //     //             font.family: Constants.regularFontFamily
        //     //             color: "black"
        //     //             opacity: 0.7
        //     //             Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
        //     //             Layout.fillHeight: true
        //     //             Layout.minimumWidth: libraryList.platformColumnWidth
        //     //             horizontalAlignment: Text.AlignLeft
        //     //             verticalAlignment: Text.AlignVCenter
        //     //             // Layout.preferredWidth: 100
        //     //         }
        //     //
        //     //         Text {
        //     //             id: filenameSection
        //     //             text: "Filename"
        //     //             font.pointSize: 10
        //     //             font.family: Constants.regularFontFamily
        //     //             color: "black"
        //     //             opacity: 0.7
        //     //             Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
        //     //             Layout.fillHeight: true
        //     //             Layout.minimumWidth: libraryList.filenameColumnWidth
        //     //             horizontalAlignment: Text.AlignLeft
        //     //             verticalAlignment: Text.AlignVCenter
        //     //             // Layout.preferredWidth: 100
        //     //         }
        //     //     }
        //     //
        //     //     Rectangle {
        //     //         Layout.bottomMargin: 4
        //     //         Layout.fillWidth: true
        //     //         Layout.preferredHeight: 1
        //     //         color: "black"
        //     //         opacity: 0.3
        //     //     }
        //     // }
        //
        //     // header: Rectangle {
        //     //     width: libraryList.width
        //     //     height: 30
        //     //     color: "grey"
        //     //     Text {
        //     //         anchors.fill: parent
        //     //         anchors.leftMargin: 12
        //     //         text: "All games"
        //     //         color: "black"
        //     //         font.pointSize: 12
        //     //         font.family: Constants.regularFontFamily
        //     //     }
        //     // }
        //     //
        //     // headerPositioning: ListView.PullBackHeader
        //
        //     // section.criteria: ViewSection.FirstCharacter
        //     // section.property: "display_name"
        //     // section.delegate: Item {
        //     //     width: libraryList.width
        //     //     height: 42
        //     //
        //     //     Rectangle {
        //     //         width: libraryList.width
        //     //         height: 30
        //     //         anchors.centerIn: parent
        //     //         color: Constants.colorTestBackground
        //     //         Text {
        //     //             anchors.fill: parent
        //     //             anchors.leftMargin: 12
        //     //             text: section
        //     //             color: Constants.colorTestText
        //     //             font.pointSize: 12
        //     //             font.family: fontFamilyName
        //     //             verticalAlignment: Text.AlignVCenter
        //     //         }
        //     //     }
        //     // }
        //     ScrollBar.vertical
        //         :
        //         ScrollBar {
        //             width: 15
        //             interactive: true
        //         }
        //
        //     currentIndex: 0
        //
        //     model: library_short_model
        //     boundsBehavior: Flickable.StopAtBounds
        //     keyNavigationEnabled: true
        //     // delegate: gameListItem
        //
        //     highlight: Item {
        //     }
        //
        //     delegate: Button {
        //         id: libItemButton
        //         background: Rectangle {
        //             color: "black"
        //             opacity: libItemButton.ListView.isCurrentItem ? 0.9 : (libItemMouse.hovered ? 0.2 : 0)
        //             radius: 0
        //         }
        //
        //         HoverHandler {
        //             id: libItemMouse
        //         }
        //
        //         TapHandler {
        //             acceptedButtons: Qt.RightButton
        //             onTapped: {
        //                 libItemButton.ListView.view.currentIndex = index
        //                 libraryEntryRightClickMenu.popup()
        //             }
        //         }
        //
        //         // hoverEnabled: true
        //         // autoExclusive: true
        //
        //         // width: ListView.view.width
        //
        //         Keys.onReturnPressed: {
        //             doubleClicked()
        //         }
        //
        //         // verticalPadding: 8
        //         //
        //         onClicked: function () {
        //             ListView.view.currentIndex = index
        //             // libraryEntryRightClickMenu.popup()
        //         }
        //
        //         onDoubleClicked: function () {
        //             entryClicked(model.id)
        //         }
        //
        //
        //         RightClickMenu {
        //             id: libraryEntryRightClickMenu
        //
        //             RightClickMenuItem {
        //                 text: "Play"
        //                 // onTriggered: {
        //                 //     addPlaylistRightClickMenu.entryId = libraryEntryRightClickMenu.entryId
        //                 //     addPlaylistRightClickMenu.popup()
        //                 // }
        //             }
        //
        //             RightClickMenuItem {
        //                 text: "View details"
        //                 // onTriggered: {
        //                 //     addPlaylistRightClickMenu.entryId = libraryEntryRightClickMenu.entryId
        //                 //     addPlaylistRightClickMenu.popup()
        //                 // }
        //             }
        //
        //             MenuSeparator {
        //             }
        //
        //             RightClickMenu {
        //                 id: addPlaylistRightClickMenu
        //                 enabled: ins.count > 0
        //
        //                 title: "Add to folder"
        //
        //                 Instantiator {
        //                     id: ins
        //                     model: playlist_model
        //                     delegate: RightClickMenuItem {
        //                         text: model.display_name
        //                         onTriggered: {
        //                             playlist_model.addEntryToPlaylist(model.id, libraryEntryRightClickMenu.entryId)
        //                             library_model.updatePlaylistsForEntry(libraryEntryRightClickMenu.entryId)
        //                             // Add your action here
        //                         }
        //                     }
        //
        //                     onObjectAdded: function (index, object) {
        //                         addPlaylistRightClickMenu.insertItem(index, object)
        //                     }
        //                     onObjectRemoved: function (index, object) {
        //                         addPlaylistRightClickMenu.removeItem(object)
        //                     }
        //                 }
        //             }
        //
        //             MenuSeparator {
        //             }
        //
        //             RightClickMenuItem {
        //                 enabled: false
        //                 text: "Manage save data"
        //                 // onTriggered: {
        //                 //     addPlaylistRightClickMenu.entryId = libraryEntryRightClickMenu.entryId
        //                 //     addPlaylistRightClickMenu.popup()
        //                 // }
        //             }
        //
        //             // MenuSeparator {
        //             // }
        //
        //             // RightClickMenuItem {
        //             //     text: "Show in filesystem"
        //             //     // onTriggered: {
        //             //     //     addPlaylistRightClickMenu.entryId = libraryEntryRightClickMenu.entryId
        //             //     //     addPlaylistRightClickMenu.popup()
        //             //     // }
        //             // }
        //
        //
        //         }
        //
        //         contentItem: RowLayout {
        //
        //             Flow {
        //                 Layout.fillHeight: true
        //                 // Layout.fillWidth: true
        //                 Layout.preferredWidth: libraryList.titleColumnWidth
        //
        //                 Text {
        //                     text: model.display_name
        //                     font.pointSize: 11
        //                     font.family: Constants.regularFontFamily
        //                     color: libItemButton.ListView.isCurrentItem ? "white" : "black"
        //                     wrapMode: Text.WordWrap
        //                     width: parent.width
        //                 }
        //
        //                 Text {
        //                     visible: model.parent_game_name !== ""
        //                     text: model.parent_game_name
        //                     font.pointSize: 10
        //                     font.family: Constants.regularFontFamily
        //                     color: libItemButton.ListView.isCurrentItem ? "white" : "black"
        //                 }
        //             }
        //
        //             Text {
        //                 text: model.platform_name
        //                 font.pointSize: 10
        //                 font.family: Constants.regularFontFamily
        //                 color: libItemButton.ListView.isCurrentItem ? "white" : "black"
        //                 opacity: libItemButton.ListView.isCurrentItem ? 0.9 : (libItemMouse.hovered ? 0.9 : 0.6)
        //                 Layout.fillHeight: true
        //                 Layout.preferredWidth: libraryList.platformColumnWidth
        //                 // Layout.fillWidth: true
        //                 // Layout.horizontalStretchFactor: 1
        //                 // Layout.preferredWidth: 100
        //             }
        //
        //             Text {
        //                 text: model.content_path
        //                 font.pointSize: 10
        //                 font.family: Constants.regularFontFamily
        //                 color: libItemButton.ListView.isCurrentItem ? "white" : "black"
        //                 opacity: libItemButton.ListView.isCurrentItem ? 0.9 : (libItemMouse.hovered ? 0.9 : 0.6)
        //                 Layout.fillHeight: true
        //                 Layout.preferredWidth: libraryList.filenameColumnWidth
        //                 // Layout.fillWidth: true
        //                 // Layout.horizontalStretchFactor: 1
        //                 // Layout.preferredWidth: 100
        //             }
        //         }
        //     }
        // }
    }


}