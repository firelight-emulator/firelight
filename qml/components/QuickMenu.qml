import QtQuick
import QtQuick.Controls
import QtQuick.Layouts 1.0
import Firelight 1.0

Pane {
    id: root

    signal resumeGame()
    signal resetGame()
    signal rewindPressed()
    signal closeGame()

    LibraryEntry {
        id: entry
        entryId: EmulationService.currentEntryId
    }

    padding: 32
    background: Item {}

    Row {
        id: titleRow
        height: 60
        spacing: 16

        Rectangle {
            color: "#292929"
            visible: entry.icon1x1SourceUrl === ""
            implicitWidth: 60
            implicitHeight: 60

            FLIcon {
                icon: entry.platformIconName
                color: "#595959"
                anchors.centerIn: parent
                height: parent.height - 16
                width: parent.width - 16
                size: height
            }
        }
        Image {
            source: entry.icon1x1SourceUrl
            visible: entry.icon1x1SourceUrl !== ""
            asynchronous: true
            sourceSize.width: 60
            sourceSize.height: 60
            smooth: true
            fillMode: Image.PreserveAspectFit
        }

        Text {
            text: entry.name
            font.family: Constants.regularFontFamily
            font.pixelSize: 22
            font.weight: Font.DemiBold
            height: 60
            color: "white"
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
        }
    }

    RowLayout {
        anchors.top: titleRow.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        spacing: 32
         ColumnLayout {
            id: menuColumn
            KeyNavigation.right: quickMenuStack
             Layout.maximumWidth: 400
             Layout.alignment: Qt.AlignCenter
             FirelightMenuItem {
                 id: resumeGameButton
                 labelText: "Resume Game"
                 focus: true
                 Layout.fillWidth: true
                 KeyNavigation.down: restartGameButton
                 // Layout.preferredWidth: parent.width / 2
                 Layout.preferredHeight: 40
                 Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                 checkable: false
                 alignRight: true
                 onClicked: function () {
                     root.resumeGame()
                 }
             }
             FirelightMenuItem {
                 id: restartGameButton
                 labelText: "Restart Game"
                 Layout.fillWidth: true
                 KeyNavigation.down: rewindButton
                 // Layout.preferredWidth: parent.width / 2
                 Layout.preferredHeight: 40
                 Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                 checkable: false
                 alignRight: true
                 onClicked: {
                     resetGameDialog.open()
                 }
             }
             FirelightMenuItem {
                 id: rewindButton
                 labelText: "Rewind"
                 KeyNavigation.down: suspendPointButton
                 Layout.fillWidth: true
                 // Layout.preferredWidth: parent.width / 2
                 Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                 Layout.preferredHeight: 40
                 checkable: false
                 enabled: EmulationService.rewindEnabled
                 alignRight: true
                 onClicked: {
                     root.rewindPressed()
                 }
             }
             // Rectangle {
             //     Layout.fillWidth: true
             //     // Layout.preferredWidth: parent.width / 2
             //     Layout.preferredHeight: 1
             //     Layout.alignment: Qt.AlignLeft | Qt.AlignTop
             //     opacity: 0.3
             //     color: "#dadada"
             // }
             FirelightMenuItem {
                 id: suspendPointButton
                 labelText: "Suspend Points"
                 Layout.fillWidth: true
                 KeyNavigation.down: gameSettingsButton
                 // Layout.preferredWidth: parent.width / 2
                 Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                 Layout.preferredHeight: 40
                 checkable: false
                 alignRight: true
                 // enabled: false
                 onClicked: {
                     quickMenuStack.replaceCurrentItem(suspendPointMenu, {}, StackView.Immediate)
                     quickMenuStack.forceActiveFocus()
                 }
             }
             FirelightMenuItem {
                 id: gameSettingsButton
                 labelText: "Game settings"
                 Layout.fillWidth: true
                 KeyNavigation.down: closeGameButton
                 // Layout.preferredWidth: parent.width / 2
                 Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                 Layout.preferredHeight: 40
                 checkable: false
                 alignRight: true
                 // enabled: false
                 onClicked: {
                     quickMenuStack.replaceCurrentItem(gameSettingsView, {}, StackView.Immediate)
                     quickMenuStack.forceActiveFocus()
                 }
             }
             // Rectangle {
             //     Layout.fillWidth: true
             //     // Layout.preferredWidth: parent.width / 2
             //     Layout.preferredHeight: 1
             //     Layout.alignment: Qt.AlignLeft | Qt.AlignTop
             //     opacity: 0.3
             //     color: "#dadada"
             // }
             // FirelightMenuItem {
             //     id: achievementsButton
             //     labelText: "Achievements"
             //     Layout.fillWidth: true
             //     KeyNavigation.down: closeGameButton
             //     // Layout.preferredWidth: parent.width / 2
             //     Layout.alignment: Qt.AlignLeft | Qt.AlignTop
             //     Layout.preferredHeight: 40
             //     checkable: false
             //     alignRight: true
             //     // enabled: false
             //     onClicked: {
             //         quickMenuStack.pushItem(achievementsView, {}, StackView.Immediate)
             //     }
             // }

             // Rectangle {
             //     Layout.fillWidth: true
             //     // Layout.preferredWidth: parent.width / 2
             //     Layout.alignment: Qt.AlignLeft | Qt.AlignTop
             //     Layout.preferredHeight: 1
             //     opacity: 0.3
             //     color: "#dadada"
             // }
             FirelightMenuItem {
                 id: closeGameButton
                 labelText: "Close Game"
                 Layout.fillWidth: true
                 // Layout.preferredWidth: parent.width / 2
                 Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                 Layout.preferredHeight: 40
                 checkable: false
                 alignRight: true

                 onClicked: {
                     closeGameDialog.open()
                 }
             }
         }

         StackView {
             id: quickMenuStack
             Layout.fillWidth: true
             Layout.fillHeight: true

             KeyNavigation.left: resumeGameButton

             Keys.onBackPressed: {
                if (quickMenuStack.depth >= 1) {
                    quickMenuStack.pop()
                    resumeGameButton.forceActiveFocus()
                }
             }
         }
     }

    FirelightDialog {
        id: closeGameDialog
        text: "Are you sure you want to close the game?"
        onAccepted: {
            root.closeGame()
        }
    }

    FirelightDialog {
        id: resetGameDialog
        text: "Are you sure you want to reset the game?"
        onAccepted: {
            root.resetGame()
        }
    }

    Component {
        id: gameSettingsView
        GameSettingsView {
            contentHash: EmulationService.currentContentHash
            platformId: EmulationService.currentPlatformId
            level: EmulationService.currentSettingsLevel
            platformName: EmulationService.currentPlatformName
        }
    }

    Component {
        id: suspendPointMenu
        Item {
            SuspendPoints {
                id: suspendData
                contentHash: emulatorLoader.item ? emulatorLoader.item.contentHash : ""
                saveSlotNumber: EmulationService.currentSaveSlotNumber
            }

            FirelightDialog {
                id: deleteSuspendPointDialog
                property int index: 0
                text: "Are you sure you want to delete the Suspend Point in slot " + (index + 1) + "?"
                onAccepted: {
                    suspendData.deleteSuspendPoint(index)
                    index = 0
                }
            }

            FirelightDialog {
                id: overwriteSuspendPointDialog
                property int index: 0
                text: "Are you sure you want to overwrite the data in slot " + (index + 1) + "?"
                onAccepted: {
                    emulatorLoader.item.writeSuspendPoint(index)
                    index = 0
                }
            }

            GridView {
                id: suspendPointGrid
                anchors.fill: parent
                anchors.leftMargin: (parent.width % cellWidth) / 2
                anchors.rightMargin: (parent.width % cellWidth) / 2
                model: suspendData.suspendPoints
                cellWidth: 300
                cellHeight: 360

                header: Pane {
                    width: GridView.view.width
                    verticalPadding: 0
                    horizontalPadding: 8
                    background: Item {}
                    contentItem: ColumnLayout {
                        spacing: 8
                        RowLayout {
                            spacing: 12
                            Layout.preferredHeight: 40
                            Layout.fillWidth: true
                            Text {
                                Layout.fillHeight: true
                                text: "Suspend Points"
                                color: "white"
                                font.family: Constants.regularFontFamily
                                font.pixelSize: 22
                                font.weight: Font.DemiBold
                                verticalAlignment: Text.AlignVCenter
                            }
                            Item {
                                Layout.fillHeight: true
                                Layout.fillWidth: true
                            }
                            Button {
                                id: undoButton
                                Layout.fillHeight: true
                                horizontalPadding: 12
                                background: Rectangle {
                                    color: "white"
                                    opacity: parent.pressed ? 0.16 : 0.1
                                    radius: 4
                                    visible: createHoverHandler.hovered && parent.enabled
                                }
                                HoverHandler {
                                    id: createHoverHandler
                                    cursorShape: parent.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                                }
                                contentItem: RowLayout {
                                    spacing: 8
                                    FLIcon {
                                        id: createIcon
                                        icon: "undo"
                                        Layout.fillHeight: true
                                        size: height
                                        color: undoButton.enabled ? "white" : "#737373"
                                    }
                                    Text {
                                        Layout.fillHeight: true
                                        Layout.fillWidth: true
                                         text: "Undo last load"
                                         color: undoButton.enabled ? "white" : "#737373"
                                         font.family: Constants.regularFontFamily
                                         font.pixelSize: 16
                                         font.weight: Font.Medium
                                         verticalAlignment: Text.AlignVCenter
                                            horizontalAlignment: Text.AlignHCenter
                                     }
                                 }

                                 enabled: emulatorLoader.item ? emulatorLoader.item.canUndoLoadSuspendPoint : false

                                 onClicked: {
                                    emulatorLoader.item.undoLoadSuspendPoint()
                                 }
                            }

                        }
                        // RowLayout {
                        //     spacing: 12
                        //     Layout.preferredHeight: 40
                        //     Layout.fillWidth: true
                        //     Button {
                        //         Layout.fillHeight: true
                        //         horizontalPadding: 12
                        //         background: Rectangle {
                        //             color: "white"
                        //             opacity: parent.pressed ? 0.16 : 0.1
                        //             radius: 4
                        //             visible: createHoverHandler.hovered && parent.enabled
                        //         }
                        //         HoverHandler {
                        //             id: createHoverHandler
                        //             cursorShape: Qt.PointingHandCursor
                        //         }
                        //         contentItem: RowLayout {
                        //             spacing: 8
                        //             FLIcon {
                        //                 id: createIcon
                        //                 icon: "undo"
                        //                 Layout.fillHeight: true
                        //                 size: height
                        //                 color: "white"
                        //             }
                        //             Text {
                        //                 Layout.fillHeight: true
                        //                 Layout.fillWidth: true
                        //                  text: "Undo last load"
                        //                  color: "white"
                        //                  font.family: Constants.regularFontFamily
                        //                  font.pixelSize: 16
                        //                  font.weight: Font.Medium
                        //                  verticalAlignment: Text.AlignVCenter
                        //                     horizontalAlignment: Text.AlignHCenter
                        //              }
                        //          }
                        //
                        //          // onClicked: {
                        //          //    emulatorLoader.item.writeSuspendPoint(suspendPointGrid.count)
                        //          // }
                        //     }
                        //     Item {
                        //         Layout.fillHeight: true
                        //         Layout.fillWidth: true
                        //     }
                        //
                        // }
                    }
                }

                populate: FLGridViewPopulateTransition {}
                add: FLGridViewAddTransition {}

                delegate: Pane {
                    id: theThing
                    required property var model
                    required property var index
                    width: GridView.view.cellWidth
                    height: GridView.view.cellHeight
                    padding: 8
                    background: Item {}

                    property bool showGlobalCursor: true
                    property var globalCursorProxy: contentItem

                    contentItem: FocusScope {
                        FLSuspendPointCard {
                            anchors.fill: parent
                            visible: model.has_data
                            imageUrl: theThing.model.image_url
                            dateTimeString: theThing.model.timestamp
                            index: theThing.index

                            onDeleteClicked: function() {
                                deleteSuspendPointDialog.index = index
                                deleteSuspendPointDialog.open()
                            }

                            onOverwriteClicked: function() {
                                overwriteSuspendPointDialog.index = index
                                overwriteSuspendPointDialog.open()
                            }
                        }
                        Button {
                            anchors.fill: parent
                            visible: !model.has_data
                            HoverHandler {
                                id: theHoverHandler
                                cursorShape: Qt.PointingHandCursor
                            }
                            background: Rectangle {
                                color: "white"
                                opacity: parent.pressed ? 0.08 : theHoverHandler.hovered ? 0.24 : 0.16
                                border.color: "#737373"
                                radius: 4
                            }
                            contentItem: Text {
                                color: "white"
                                text: "Create in slot " + (theThing.index + 1)
                                font.pixelSize: 15
                                font.family: Constants.regularFontFamily
                                font.weight: Font.DemiBold
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            onClicked: {
                                console.log("clicked create suspend point")
                                emulatorLoader.item.writeSuspendPoint(theThing.index)
                            }
                        }
                    }
                }
            }
        }
    }

    Component {
        id: rewindMenu
        RewindMenu {
            onRewindPointSelected: function(point) {
                emulatorLoader.item.loadRewindPoint(point)
                root.currentItem.close()
                // root.popCurrentItem(StackView.Immediate)
            }
        }
    }
}