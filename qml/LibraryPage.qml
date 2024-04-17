import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FirelightStyle 1.0

FocusScope {
    id: root

    signal entryClicked(int entryId)

    Pane {
        id: header
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        background: Item {
        }

        Column {
            anchors.fill: parent
            spacing: 8

            Text {
                text: "Library"
                color: "#dadada"
                font.pointSize: 24
                font.family: Constants.semiboldFontFamily
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
            }

            Rectangle {
                width: parent.width
                height: 1
                opacity: 0.3
                color: "#dadada"
            }
        }
    }

    SplitView {
        orientation: Qt.Horizontal
        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        // handle: FirelightSplitViewHandle {
        // }
        //
        // Item {
        //     SplitView.minimumWidth: 260
        //     SplitView.maximumWidth: parent.width / 2
        //
        //     ColumnLayout {
        //         id: menu
        //         anchors.fill: parent
        //         spacing: 4
        //
        //         // Item {
        //         //     Layout.fillWidth: true
        //         //     Layout.fillHeight: true
        //         // }
        //
        //         FirelightMenuItem {
        //             labelText: "All Games"
        //             labelIcon: "\uf53e"
        //             Layout.preferredWidth: parent.width
        //             Layout.preferredHeight: 40
        //             Layout.alignment: Qt.AlignLeft | Qt.AlignTop
        //         }
        //         FirelightMenuItem {
        //             labelText: "Recently Played"
        //             labelIcon: "\ue889"
        //             Layout.preferredWidth: parent.width
        //             Layout.preferredHeight: 40
        //             Layout.alignment: Qt.AlignLeft | Qt.AlignTop
        //         }
        //         FirelightMenuItem {
        //             labelText: "Newly Added"
        //             labelIcon: "\ue838"
        //             Layout.preferredWidth: parent.width
        //             Layout.alignment: Qt.AlignLeft | Qt.AlignTop
        //             Layout.preferredHeight: 40
        //         }
        //         RowLayout {
        //             Layout.preferredWidth: parent.width
        //             Layout.preferredHeight: 40
        //             Text {
        //                 text: "Folders"
        //                 color: "#c6c6c6"
        //                 font.pointSize: 12
        //                 font.family: Constants.regularFontFamily
        //                 horizontalAlignment: Text.AlignLeft
        //                 verticalAlignment: Text.AlignVCenter
        //                 Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
        //                 leftPadding: 8
        //             }
        //
        //             Item {
        //                 Layout.fillWidth: true
        //             }
        //
        //             Text {
        //                 text: "\ue145"
        //                 font.pixelSize: 24
        //                 font.family: Constants.symbolFontFamily
        //                 horizontalAlignment: Text.AlignHCenter
        //                 verticalAlignment: Text.AlignVCenter
        //                 Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
        //                 color: "#dadada"
        //             }
        //         }
        //         Rectangle {
        //             Layout.preferredWidth: parent.width
        //             Layout.preferredHeight: 1
        //             Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        //             opacity: 0.3
        //             color: "#dadada"
        //         }
        //         FirelightMenuItem {
        //             labelText: "Test Folder One"
        //             Layout.preferredWidth: parent.width
        //             Layout.preferredHeight: 40
        //             Layout.alignment: Qt.AlignLeft | Qt.AlignTop
        //         }
        //         FirelightMenuItem {
        //             labelText: "Some Other Stuff"
        //             Layout.preferredWidth: parent.width
        //             Layout.alignment: Qt.AlignLeft | Qt.AlignTop
        //             Layout.preferredHeight: 40
        //         }
        //
        //         Item {
        //             Layout.fillWidth: true
        //             Layout.fillHeight: true
        //         }
        //     }
        //
        // }

        Pane {
            background: Item {
            }
            Text {
                text: "no gams here"
                font.pointSize: 12
                font.family: Constants.regularFontFamily
                color: "#b3b3b3"
                anchors.centerIn: parent
                visible: libraryList.count === 0
            }

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

            ListView {
                id: libraryList
                // anchors.top: sortByThing.bottom
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                clip: true
                focus: true

                property real titleColumnWidth: libraryList.width * 2 / 5
                property real platformColumnWidth: libraryList.width / 5
                property real filenameColumnWidth: libraryList.width * 2 / 5

                // header: Pane {
                //     background: Rectangle {
                //         color: "black"
                //         opacity: 0.3
                //         radius: 4
                //     }
                //     verticalPadding: 4
                //     horizontalPadding: 8
                //     RowLayout {
                //         id: headerRow
                //
                //         // anchors.fill: parent
                //         // Text {
                //         //     id: favoriteSection
                //         //     text: "\ue87d"
                //         //     font.family: Constants.symbolFontFamily
                //         //     font.pointSize: 12
                //         //     color: "#b3b3b3"
                //         //     Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                //         //     Layout.fillHeight: true
                //         //     Layout.preferredWidth: 24
                //         //     topPadding: 2
                //         //     horizontalAlignment: Text.AlignLeft
                //         //     verticalAlignment: Text.AlignVCenter
                //         // }
                //         Text {
                //             id: titleSection
                //             text: "Title"
                //             font.pointSize: 10
                //             font.family: Constants.regularFontFamily
                //             color: "#b3b3b3"
                //             Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                //             Layout.fillHeight: true
                //             Layout.fillWidth: true
                //             // Layout.preferredWidth: 300
                //             Layout.minimumWidth: libraryList.titleColumnWidth
                //             Layout.horizontalStretchFactor: 3
                //             horizontalAlignment: Text.AlignLeft
                //             verticalAlignment: Text.AlignVCenter
                //         }
                //
                //         Text {
                //             id: platformSection
                //             text: "Platform"
                //             font.pointSize: 10
                //             font.family: Constants.regularFontFamily
                //             color: "#b3b3b3"
                //             Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                //             Layout.fillHeight: true
                //             Layout.minimumWidth: libraryList.platformColumnWidth
                //             horizontalAlignment: Text.AlignLeft
                //             verticalAlignment: Text.AlignVCenter
                //             // Layout.preferredWidth: 100
                //         }
                //
                //         Text {
                //             id: filenameSection
                //             text: "Filename"
                //             font.pointSize: 10
                //             font.family: Constants.regularFontFamily
                //             color: "#b3b3b3"
                //             Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                //             Layout.fillHeight: true
                //             Layout.minimumWidth: libraryList.filenameColumnWidth
                //             horizontalAlignment: Text.AlignLeft
                //             verticalAlignment: Text.AlignVCenter
                //             // Layout.preferredWidth: 100
                //         }
                //     }
                //
                //     // Rectangle {
                //     //     width: libraryList.width
                //     //     height: 1
                //     //     color: "#dadada"
                //     //     anchors.bottom: headerRow.bottom
                //     // }
                // }

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
                keyNavigationEnabled: true
                // delegate: gameListItem

                highlight: Item {
                }

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
                        color: libItemButton.ListView.isCurrentItem ? "white" : (libItemMouse.hovered ? "white" : "transparent")
                        opacity: libItemButton.ListView.isCurrentItem ? 0.8 : 0.2
                        radius: 4
                    }

                    HoverHandler {
                        id: libItemMouse
                    }

                    // hoverEnabled: true
                    autoExclusive: true

                    // width: ListView.view.width

                    Keys.onReturnPressed: {
                        doubleClicked()
                    }

                    // verticalPadding: 8
                    //
                    onClicked: function () {
                        ListView.view.currentIndex = index
                    }

                    onDoubleClicked: function () {
                        entryClicked(model.id)
                    }

                    contentItem: RowLayout {
                        // Text {
                        //     text: "\ue87d"
                        //     font.family: Constants.symbolFontFamily
                        //     font.pointSize: 12
                        //     opacity: libItemMouse.containsMouse ? 1 : 0
                        //     color: "#b3b3b3"
                        //     Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        //     Layout.fillHeight: true
                        //     Layout.preferredWidth: favoriteSection.width
                        // }

                        Flow {
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                            Layout.preferredWidth: libraryList.titleColumnWidth

                            Text {
                                text: model.display_name
                                font.pointSize: 11
                                font.family: Constants.regularFontFamily
                                color: libItemButton.ListView.isCurrentItem ? "black" : "#ffffff"
                                wrapMode: Text.WordWrap
                                width: parent.width
                            }

                            Text {
                                visible: model.parent_game_name !== ""
                                text: model.parent_game_name
                                font.pointSize: 10
                                font.family: Constants.regularFontFamily
                                color: libItemButton.ListView.isCurrentItem ? "black" : "#b3b3b3"
                            }
                        }

                        Text {
                            text: model.platform_name
                            font.pointSize: 10
                            font.family: Constants.regularFontFamily
                            color: libItemButton.ListView.isCurrentItem ? "black" : (libItemMouse.hovered ? "white" : "#b3b3b3")
                            opacity: libItemButton.ListView.isCurrentItem ? 0.8 : 1
                            Layout.fillHeight: true
                            Layout.preferredWidth: libraryList.platformColumnWidth
                            // Layout.fillWidth: true
                            // Layout.horizontalStretchFactor: 1
                            // Layout.preferredWidth: 100
                        }

                        Text {
                            text: model.content_path
                            font.pointSize: 10
                            font.family: Constants.regularFontFamily
                            color: libItemButton.ListView.isCurrentItem ? "black" : (libItemMouse.hovered ? "white" : "#b3b3b3")
                            opacity: libItemButton.ListView.isCurrentItem ? 0.8 : 1
                            Layout.fillHeight: true
                            Layout.preferredWidth: libraryList.filenameColumnWidth
                            // Layout.fillWidth: true
                            // Layout.horizontalStretchFactor: 1
                            // Layout.preferredWidth: 100
                        }
                    }

                    // TapHandler {
                    //     id: entryLeftHandler
                    //     onTapped: {
                    //         libItemButton.onClicked()
                    //     }
                    // }
                    //
                    // TapHandler {
                    //     id: entryRightHandler
                    //     acceptedButtons: Qt.RightButton
                    //     onTapped: {
                    //         libraryEntryRightClickMenu.entryId = model.id
                    //         libraryEntryRightClickMenu.popup()
                    //     }
                    // }
                }

                // delegate: Button {
                //     id: libItemButton
                //     background: Rectangle {
                //         // color: libItemButton.checked ?
                //         //     (libItemMouse.pressed ?
                //         //         "#2b2b2b"
                //         //         : ((libItemMouse.containsMouse ?
                //         //             "#393939"
                //         //             : "#232323")))
                //         //     : (libItemMouse.pressed ?
                //         //         "#020202"
                //         //         : (libItemMouse.containsMouse ?
                //         //             "#1a1a1a"
                //         //             : "transparent"))
                //         color: entryHoverHandler.hovered ? "white" : "transparent"
                //         opacity: 0.2
                //         radius: 4
                //     }
                //     autoExclusive: true
                //
                //     // width: ListView.view.width
                //
                //     verticalPadding: 8
                //
                //     onClicked: function () {
                //         entryClicked(model.id)
                //     }
                //
                //     contentItem: RowLayout {
                //         // Text {
                //         //     text: "\ue87d"
                //         //     font.family: Constants.symbolFontFamily
                //         //     font.pointSize: 12
                //         //     opacity: libItemMouse.containsMouse ? 1 : 0
                //         //     color: "#b3b3b3"
                //         //     Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                //         //     Layout.fillHeight: true
                //         //     Layout.preferredWidth: favoriteSection.width
                //         // }
                //
                //         Flow {
                //             Layout.fillHeight: true
                //             Layout.fillWidth: true
                //             Layout.preferredWidth: titleSection.width
                //
                //             Text {
                //                 text: model.display_name
                //                 font.pointSize: 11
                //                 font.family: Constants.semiboldFontFamily
                //                 color: "#ffffff"
                //                 wrapMode: Text.WordWrap
                //                 width: parent.width
                //             }
                //
                //             Text {
                //                 visible: model.parent_game_name !== ""
                //                 text: model.parent_game_name
                //                 font.pointSize: 10
                //                 font.family: Constants.regularFontFamily
                //                 color: "#b3b3b3"
                //             }
                //         }
                //
                //         Text {
                //             text: model.platform_name
                //             font.pointSize: 10
                //             font.family: Constants.regularFontFamily
                //             color: entryHoverHandler.hovered ? "white" : "#b3b3b3"
                //             Layout.fillHeight: true
                //             Layout.fillWidth: true
                //             Layout.preferredWidth: platformSection.width
                //             // Layout.fillWidth: true
                //             // Layout.horizontalStretchFactor: 1
                //             // Layout.preferredWidth: 100
                //         }
                //
                //         Text {
                //             text: model.content_path
                //             font.pointSize: 10
                //             font.family: Constants.regularFontFamily
                //             color: entryHoverHandler.hovered ? "white" : "#b3b3b3"
                //             Layout.fillHeight: true
                //             Layout.fillWidth: true
                //             Layout.preferredWidth: filenameSection.width
                //             // Layout.fillWidth: true
                //             // Layout.horizontalStretchFactor: 1
                //             // Layout.preferredWidth: 100
                //         }
                //     }
                //
                //     HoverHandler {
                //         id: entryHoverHandler
                //         cursorShape: Qt.PointingHandCursor
                //     }
                //
                //     TapHandler {
                //         id: entryLeftHandler
                //         onTapped: {
                //             libItemButton.onClicked()
                //         }
                //     }
                //
                //     TapHandler {
                //         id: entryRightHandler
                //         acceptedButtons: Qt.RightButton
                //         onTapped: {
                //             libraryEntryRightClickMenu.entryId = model.id
                //             libraryEntryRightClickMenu.popup()
                //         }
                //     }
                // }
                // preferredHighlightBegin: height / 3
                // preferredHighlightEnd: 2 * (height / 3) + currentItem.height
            }
        }
    }


}