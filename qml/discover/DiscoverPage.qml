import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQml.Models
import FirelightStyle 1.0

Flickable {
    id: root

    contentHeight: theColumn.height
    boundsBehavior: Flickable.StopAtBounds

    ScrollBar.vertical: ScrollBar {
    }

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
                    text: "Market"
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
                    // Text {
                    //     Layout.fillHeight: true
                    //     Layout.alignment: Qt.AlignBottom
                    //     text: "Showing " + mod_model.count + " mods" + (mod_model.count === 1 ? "" : "s")
                    //     color: "#C2BBBB"
                    //     font.pointSize: 11
                    //     font.family: Constants.regularFontFamily
                    //     font.weight: Font.Normal
                    //     horizontalAlignment: Text.AlignLeft
                    //     verticalAlignment: Text.AlignBottom
                    // }

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
                    //
                    // Item {
                    //     Layout.preferredWidth: 8
                    // }
                    //
                    // Text {
                    //     Layout.fillHeight: true
                    //     Layout.alignment: Qt.AlignVCenter
                    //     text: "Sort by:"
                    //     color: "white"
                    //     font.pointSize: 12
                    //     font.family: Constants.regularFontFamily
                    //     horizontalAlignment: Text.AlignLeft
                    //     verticalAlignment: Text.AlignVCenter
                    // }
                    //
                    // ComboBox {
                    //     model: ["A-Z", "Recently played", "Date added (newest first)", "Date added (oldest first)"]
                    // }
                }
            }

            Pane {
                background: Rectangle {
                    color: "transparent"
                    // border.color: "green"
                }
                width: parent.width
                height: libraryGrid.contentHeight + 20

                horizontalPadding: 0

                GridView {
                    id: libraryGrid
                    anchors.fill: parent
                    cellWidth: cellContentWidth
                    cellHeight: cellContentHeight

                    function itemsPerRow() {
                        return Math.floor(width / cellWidth);
                    }

                    function itemsPerColumn() {
                        return Math.floor(height / cellHeight);
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

                    model: mod_model
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
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: colLayout.width

                                    radius: 6
                                    clip: true

                                    Image {
                                        id: img
                                        anchors.fill: parent
                                        source: model.image_source
                                        fillMode: Image.Stretch
                                    }
                                }

                                // Rectangle {
                                //     color: "grey"
                                //     Layout.fillWidth: true
                                //     Layout.preferredHeight: colLayout.width
                                //     topLeftRadius: 6
                                //     topRightRadius: 6
                                //
                                //     Text {
                                //         text: "Box art coming soon :)"
                                //         font.pointSize: 9
                                //         // font.weight: Font.Light
                                //         font.family: Constants.regularFontFamily
                                //         color: "#232323"
                                //         anchors.centerIn: parent
                                //         // Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
                                //     }
                                // }

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
                                            text: model.name
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

                            // TapHandler {
                            //     acceptedButtons: Qt.RightButton
                            //     onTapped: {
                            //         rootItem.GridView.view.currentIndex = index
                            //         libraryEntryRightClickMenu.popup()
                            //     }
                            // }

                            // Keys.onReturnPressed: {
                            //     doubleClicked()
                            // }

                            onClicked: function () {

                                //                         contentPopup.modId = model.id
                                //                         contentPopup.name = model.name
                                //                         contentPopup.author = model.primary_author
                                //                         contentPopup.description = model.description
                                //                         contentPopup.targetGameName = model.target_game_name
                                //                         contentPopup.modInLibrary = library_model.isModInLibrary(model.id)
                                //
                                //                         contentPopup.targetInLibrary = false
                                //                         for (let i = 0; i < model.rom_ids.length; i++) {
                                //                             if (library_model.isRomInLibrary(model.rom_ids[i])) {
                                //                                 contentPopup.targetInLibrary = true
                                //                                 break
                                //                             }
                                //                         }
                                root.StackView.view.push(contentThing, {
                                    modId: model.id,
                                    name: model.name,
                                    author: model.primary_author,
                                    description: model.description,
                                    targetGameName: model.target_game_name,
                                    modInLibrary: library_model.isModInLibrary(model.id),
                                    targetInLibrary: false,
                                    romId: -1,
                                    gameReleaseId: -1
                                })
                                // rootItem.GridView.view.currentIndex = model.index
                                // libraryEntryRightClickMenu.popup()
                            }

                            // onDoubleClicked: function () {
                            //     entryClicked(model.id)
                            // }
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

    Component {
        id: contentThing

        StoreContent {
        }
    }


}
//
//
// import QtQuick
// import QtQuick.Controls
// import QtQuick.Layouts
// import QtQuick.Effects
// import QtQml.Models
// import FirelightStyle 1.0
//
// Item {
//     id: root
//
//     Pane {
//         id: header
//         anchors.top: parent.top
//         anchors.left: parent.left
//         anchors.right: parent.right
//         background: Item {
//         }
//
//         horizontalPadding: 0
//         verticalPadding: 6
//
//         Column {
//             anchors.fill: parent
//             spacing: 8
//
//             Text {
//                 text: "Mods"
//                 color: "#353535"
//                 font.pointSize: 24
//                 font.family: Constants.semiboldFontFamily
//                 horizontalAlignment: Text.AlignLeft
//                 verticalAlignment: Text.AlignVCenter
//             }
//         }
//     }
//
//     Item {
//         id: content
//         anchors.top: header.bottom
//         anchors.left: parent.left
//         anchors.right: parent.right
//         anchors.bottom: parent.bottom
//
//         ListView {
//             id: list
//             anchors.fill: parent
//             boundsBehavior: Flickable.StopAtBounds
//             clip: true
//             spacing: 8
//             model: mod_model
//
//             delegate: Button {
//                 id: button
//                 height: 100
//                 width: parent.width / 2
//                 padding: 4
//
//                 scale: pressed ? 0.99 : 1
//
//                 Behavior on scale {
//                     NumberAnimation {
//                         duration: 50
//                         easing.type: Easing.InOutBounce
//                     }
//                 }
//
//                 // layer.enabled: true
//                 // layer.effect: MultiEffect {
//                 //     id: shadowEffect
//                 //     source: button
//                 //     // anchors.fill: button
//                 //     shadowEnabled: true
//                 //     shadowBlur: 0.4
//                 //     shadowVerticalOffset: button.pressed ? 2 : 6
//                 //     shadowScale: 1
//                 //     shadowOpacity: 0.3
//                 //
//                 //     Behavior on shadowVerticalOffset {
//                 //         NumberAnimation {
//                 //             duration: 50
//                 //             easing.type: Easing.InOutBounce
//                 //         }
//                 //     }
//                 // }
//
//                 background: Rectangle {
//                     id: bg
//                     color: "white"
//                     radius: 8
//
//                     // Behavior on opacity {
//                     //     NumberAnimation {
//                     //         duration: 100
//                     //         easing.type: Easing.InOutQuad
//                     //     }
//                     // }
//                 }
//
//                 // MultiEffect {
//                 //     source: bg
//                 //     anchors.fill: bg
//                 //     shadowEnabled: true
//                 // }
//
//                 HoverHandler {
//                     id: hov
//                     cursorShape: Qt.PointingHandCursor
//                 }
//
//                 TapHandler {
//                     onTapped: {
//                         contentPopup.modId = model.id
//                         contentPopup.name = model.name
//                         contentPopup.author = model.primary_author
//                         contentPopup.description = model.description
//                         contentPopup.targetGameName = model.target_game_name
//                         contentPopup.modInLibrary = library_model.isModInLibrary(model.id)
//
//                         contentPopup.targetInLibrary = false
//                         for (let i = 0; i < model.rom_ids.length; i++) {
//                             if (library_model.isRomInLibrary(model.rom_ids[i])) {
//                                 contentPopup.targetInLibrary = true
//                                 break
//                             }
//                         }
//
//                         contentPopup.open()
//                     }
//                 }
//
//                 contentItem: RowLayout {
//                     spacing: 8
//
//                     Image {
//                         id: img
//                         Layout.fillHeight: true
//                         Layout.preferredWidth: height * 2
//                         // visible: false
//                         source: model.image_source
//                         fillMode: Image.Stretch
//                     }
//
//                     ColumnLayout {
//                         Layout.topMargin: 4
//                         Layout.bottomMargin: 4
//                         Layout.fillHeight: true
//                         Layout.fillWidth: true
//                         spacing: 4
//
//                         Text {
//                             Layout.fillWidth: true
//                             text: model.name
//                             font.family: Constants.semiboldFontFamily
//                             font.pointSize: 14
//                             verticalAlignment: Text.AlignVCenter
//                             color: "black"
//                             opacity: 0.9
//                         }
//
//                         Text {
//                             Layout.fillWidth: true
//                             text: "by " + model.primary_author
//                             font.family: Constants.regularFontFamily
//                             font.pointSize: 10
//                             verticalAlignment: Text.AlignVCenter
//                             color: "black"
//                             opacity: 0.7
//                         }
//                         Item {
//                             Layout.fillWidth: true
//                             Layout.fillHeight: true
//                         }
//
//                         Text {
//                             Layout.fillWidth: true
//                             text: "Mod for " + model.target_game_name + " (" + model.platform_name + ")"
//                             font.family: Constants.regularFontFamily
//                             font.pointSize: 10
//                             verticalAlignment: Text.AlignVCenter
//                             color: "black"
//                             opacity: 0.7
//                         }
//                     }
//                 }
//             }
//         }
//     }
//
//
//     FirelightDialog {
//         id: contentPopup
//
//         property int modId;
//         property int patchId;
//         property string name;
//         property string author;
//         property string description;
//         property bool targetInLibrary;
//         property bool modInLibrary;
//         property string targetGameName;
//         property int romId;
//         property int gameReleaseId;
//
//         width: parent.width * 0.75
//         height: parent.height * 0.9
//         parent: Overlay.overlay
//         x: (parent.width / 2) - (width / 2)
//         y: (parent.height / 2) - (height / 2)
//         contentItem: StoreContent {
//             id: contentThing
//             anchors.centerIn: parent
//             modId: contentPopup.modId
//             name: contentPopup.name
//             author: contentPopup.author
//             description: contentPopup.description
//             targetInLibrary: contentPopup.targetInLibrary
//             targetGameName: contentPopup.targetGameName
//             romId: contentPopup.romId
//             gameReleaseId: contentPopup.gameReleaseId
//             modInLibrary: contentPopup.modInLibrary
//         }
//
//         onAboutToShow: {
//             contentThing.modId = contentPopup.modId
//             contentThing.name = contentPopup.name
//             contentThing.author = contentPopup.author
//             contentThing.description = contentPopup.description
//             contentThing.targetInLibrary = contentPopup.targetInLibrary
//             contentThing.targetGameName = contentPopup.targetGameName
//             contentThing.romId = contentPopup.romId
//             contentThing.gameReleaseId = contentPopup.gameReleaseId
//             contentThing.modInLibrary = contentPopup.modInLibrary
//         }
//
//         onClosed: {
//             contentThing.modId = -1
//             contentThing.name = ""
//             contentThing.author = ""
//             contentThing.description = ""
//             contentThing.targetInLibrary = false
//             contentThing.targetGameName = ""
//             contentThing.modInLibrary = false
//             contentThing.romId = -1
//             contentThing.gameReleaseId = -1
//         }
//
//         header: Item {
//             height: 0
//             width: 0
//         }
//         footer: Item {
//             height: 0
//             width: 0
//         }
//     }
//
//
// }