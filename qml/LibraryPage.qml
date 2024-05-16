import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQml.Models

Flickable {
    id: root

    contentHeight: theColumn.height
    boundsBehavior: Flickable.StopAtBounds

    ScrollBar.vertical: ScrollBar {
    }

    signal entryClicked(int entryId)

    RowLayout {
        id: contentRow
        anchors.fill: parent
        spacing: 0

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.horizontalStretchFactor: 1
        }

        Column {
            id: theColumn
            Layout.preferredWidth: Math.max(5, (Math.min(Math.floor(parent.width / libraryGrid.cellContentWidth), 8)) * libraryGrid.cellContentWidth)

            Pane {
                id: header

                width: parent.width
                height: 120
                background: Rectangle {
                    color: "transparent"
                    // border.color: "red"
                }

                horizontalPadding: 8

                Text {
                    anchors.fill: parent
                    text: "All Games"
                    color: "white"
                    font.pointSize: 26
                    font.family: Constants.regularFontFamily
                    font.weight: Font.DemiBold
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Pane {
                id: filters
                background: Rectangle {
                    color: "transparent"
                    // border.color: "blue"
                }
                width: parent.width

                horizontalPadding: 8
                verticalPadding: 0

                RowLayout {
                    anchors.fill: parent

                    spacing: 12
                    Text {
                        Layout.fillHeight: true
                        Layout.alignment: Qt.AlignBottom
                        text: "Showing " + library_model.count + " game" + (library_model.count === 1 ? "" : "s")
                        color: "#C2BBBB"
                        font.pointSize: 11
                        font.family: Constants.regularFontFamily
                        font.weight: Font.Normal
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignBottom
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    // Button {
                    //     id: melol
                    //     Layout.preferredHeight: 32
                    //     horizontalPadding: 12
                    //     verticalPadding: 8
                    //
                    //     hoverEnabled: true
                    //
                    //     Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
                    //     background: Rectangle {
                    //         color: melol.hovered ? "#4e535b" : "#3e434b"
                    //         radius: 12
                    //         border.color: "#7d848c"
                    //     }
                    //
                    //     contentItem: Text {
                    //         text: "Show filters"
                    //         color: "white"
                    //         font.pointSize: 11
                    //         font.family: Constants.regularFontFamily
                    //         horizontalAlignment: Text.AlignHCenter
                    //         verticalAlignment: Text.AlignVCenter
                    //     }
                    // }

                    // Item {
                    //     Layout.preferredWidth: 8
                    // }

                    Text {
                        Layout.fillHeight: true
                        Layout.alignment: Qt.AlignVCenter
                        text: "Sort by:"
                        color: "white"
                        font.pointSize: 12
                        font.family: Constants.regularFontFamily
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                    }

                    ComboBox {
                        textRole: "text"
                        valueRole: "value"

                        onActivated: library_short_model.sortType = currentValue
                        Component.onCompleted: currentIndex = indexOfValue(library_short_model.sortType)

                        background: Rectangle {
                            implicitWidth: 140
                            implicitHeight: 40
                            color: "#3e434b"
                            radius: 12
                            border.color: "#7d848c"
                        }

                        palette.buttonText: "white"

                        model: [
                            {text: "A-Z", value: "display_name"},
                            {text: "Newest first", value: "created_at"}
                        ]
                    }
                }
            }

            Pane {
                background: Rectangle {
                    color: "transparent"
                    // border.color: "green"
                }
                width: parent.width
                height: libraryGrid.contentHeight

                horizontalPadding: 0

                GridView {
                    id: libraryGrid
                    anchors.fill: parent
                    cellWidth: cellContentWidth
                    cellHeight: cellContentHeight

                    function itemsPerRow() {
                        return Math.floor(width / cellWidth);
                    }

                    function rowWidth() {
                        return itemsPerRow() * cellWidth;
                    }

                    // onWidthChanged: {
                    //     contentRow.middleSectionWidth = libraryGrid.rowWidth()
                    // }

                    cacheBuffer: 30

                    clip: true

                    interactive: false
                    readonly property int cellSpacing: 12
                    readonly property int cellContentWidth: 180 + cellSpacing
                    readonly property int cellContentHeight: 260 + cellSpacing

                    populate: Transition {
                    }

                    currentIndex: 0

                    model: library_short_model
                    boundsBehavior: Flickable.StopAtBounds
                    keyNavigationEnabled: true

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

                            scale: pressed ? 0.97 : 1
                            Behavior on scale {
                                NumberAnimation {
                                    duration: 64
                                    easing.type: Easing.InOutQuad
                                }
                            }

                            background: Rectangle {
                                color: libItemButton.hovered ? "#3a3e45" : "#25282C"
                                radius: 6
                                // border.color: rootItem.GridView.isCurrentItem ? "white" : "transparent"
                                // border.width: 2
                                // leftInset: -2
                                // rightInset: -2
                                // topInset: -2
                                // bottomInset: -2
                            }

                            contentItem: ColumnLayout {
                                id: colLayout
                                spacing: 0
                                anchors {
                                    fill: parent
                                    // margins: 2
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
                                    topLeftRadius: 6
                                    topRightRadius: 6

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

                                Item {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true

                                    ColumnLayout {
                                        spacing: 2
                                        anchors {
                                            fill: parent
                                            margins: 6
                                        }

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

                                // Pane {
                                //     Layout.fillWidth: true
                                //     Layout.fillHeight: true
                                //
                                //     // clip: true
                                //     background: Item {
                                //     }
                                //     padding: 8
                                //     ColumnLayout {
                                //         anchors.fill: parent
                                //         spacing: 2
                                //         Text {
                                //             text: model.display_name
                                //             font.pointSize: 11
                                //             font.weight: Font.Bold
                                //             font.family: Constants.regularFontFamily
                                //             color: "white"
                                //             Layout.fillWidth: true
                                //             elide: Text.ElideRight
                                //             maximumLineCount: 1
                                //             // Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
                                //         }
                                //         Text {
                                //             text: model.platform_name
                                //             font.pointSize: 10
                                //             font.weight: Font.Medium
                                //             font.family: Constants.regularFontFamily
                                //             color: "#C2BBBB"
                                //             Layout.fillWidth: true
                                //             // Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
                                //         }
                                //         Item {
                                //             Layout.fillWidth: true
                                //             Layout.fillHeight: true
                                //         }
                                //     }
                                //
                                // }
                            }

                            TapHandler {
                                acceptedButtons: Qt.RightButton
                                onTapped: {
                                    rootItem.GridView.view.currentIndex = index
                                    libraryEntryRightClickMenu.popup()
                                }
                            }

                            Keys.onReturnPressed: {
                                doubleClicked()
                            }

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
                                    onTriggered: {
                                        entryClicked(model.id)
                                    }
                                }

                                RightClickMenuItem {
                                    enabled: false
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
                            }
                        }
                    }
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.horizontalStretchFactor: 1
        }
    }


}