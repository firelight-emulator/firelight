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
                    text: "Discover"
                    color: "white"
                    font.pointSize: 26
                    font.family: Constants.regularFontFamily
                    font.weight: Font.DemiBold
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
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
                            }

                            contentItem: ColumnLayout {
                                id: colLayout
                                spacing: 0
                                anchors {
                                    fill: parent
                                    // margins: 2
                                }

                                Image {
                                    id: img
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: colLayout.width
                                    source: model.image_source
                                    fillMode: Image.Stretch
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

                            onClicked: function () {
                                let targetInLibrary = false
                                for (let i = 0; i < model.rom_ids.length; i++) {
                                    if (library_model.isRomInLibrary(model.rom_ids[i])) {
                                        targetInLibrary = true
                                        break
                                    }
                                }

                                root.StackView.view.push(contentThing, {
                                    modId: model.id,
                                    name: model.name,
                                    author: model.primary_author,
                                    description: model.description,
                                    targetGameName: model.target_game_name,
                                    modInLibrary: library_model.isModInLibrary(model.id),
                                    targetInLibrary: targetInLibrary
                                })

                                // rootItem.GridView.view.currentIndex = model.index
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

    Component {
        id: contentThing

        StoreContent {
        }
    }
}