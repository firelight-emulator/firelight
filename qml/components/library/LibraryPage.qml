import QtQuick
import QtQuick.Controls
import QtQuick.Layouts 1.0
import QtQml.Models
import Firelight 1.0

Pane {
    id: root

    signal startGame(int entryId)
    required property int currentEntryId

    padding: 16
    background: Item {}
    contentItem: FocusScope {
        focus: true

        Component.onCompleted: {
            FilteredLibraryEntryModel.folderId = -1
            theList.currentIndex = 0
        }

        ButtonGroup {
            id: libraryButtonGroup
            exclusive: true
        }
        CreateFolderDialog {
            id: createFolderDialog
            onAccepted: {
                LibraryFolderModel.addFolder(createFolderDialog.folderName)
            }
        }
        UpdateFolderDialog {
            id: updateFolderDialog
        }
        EditEntryDialog {
            id: editEntryDialog
        }
        ManageSaveDataDialog {
            id: manageSaveDataDialog
        }
        FocusScope {
            id: libraryNavFocusScope
            anchors.left: parent.left
            anchors.topMargin: 64
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 240
            ColumnLayout {
                id: libraryNavList
                anchors.fill: parent
                FirelightMenuItem {
                    id: allGamesButton
                    focus: true

                    property string sectionTitle: "All games"

                    KeyNavigation.down: createFolderButton

                    labelText: "All games"
                    property bool showGlobalCursor: true
                    // labelIcon: "\ue40a"
                    Layout.preferredHeight: 48
                    Layout.fillWidth: true
                    checked: true
                    ButtonGroup.group: libraryButtonGroup

                    onToggled: {
                        if (checked) {
                          FilteredLibraryEntryModel.folderId = -1
                            theList.currentIndex = 0
                        }
                    }
                }
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                    color: "#4B4B4B"
                }
                RowLayout {
                    Layout.fillWidth: true
                    Text {
                        text: "Folders"
                        font.pixelSize: 18
                        font.family: Constants.regularFontFamily
                        // font.weight: Font.DemiBold
                        color: ColorPalette.neutral200
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                        Layout.fillWidth: true
                    }
                    FirelightButton {
                        id: createFolderButton
                        KeyNavigation.down: folderList.count > 0 ? folderList : null

                        iconCode: "\ue2cc"
                        tooltipLabel: "New folder"
                        flat: true
                        circle: true
                        onClicked: {
                            createFolderDialog.open()
                        }
                    }
                }
                Text {
                    Layout.topMargin: 8
                    text: "No folders"
                    visible: folderList.count === 0
                    font.pixelSize: 15
                    font.weight: Font.DemiBold
                    font.family: Constants.regularFontFamily
                    color: ColorPalette.neutral400
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
                }
                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    visible: folderList.count === 0
                }
                ListView {
                    id: folderList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 4

                    // interactive: contentHeight > height

                    boundsBehavior: Flickable.StopAtBounds
                    currentIndex: 0

                    onCurrentIndexChanged: {
                        theList.currentIndex = 0
                    }

                    model: LibraryFolderModel
                    delegate: LibraryFolderListDelegate {
                        id: folderDelegate
                        ButtonGroup.group: libraryButtonGroup

                        ContextMenu.menu: RightClickMenu {
                            RightClickMenuItem {
                                text: "Rename"

                                onTriggered: {
                                  editClicked(folderDelegate.model.playlist_id, folderDelegate.model.display_name)
                                }
                            }

                            RightClickMenuItem {
                                text: "Delete"
                                dangerous: true

                                onTriggered: {
                                      LibraryFolderModel.deleteFolder(folderDelegate.model.playlist_id)
                                      if (folderDelegate.ListView.isCurrentItem) {
                                          FilteredLibraryEntryModel.folderId = -1
                                          allGamesButton.toggle()
                                      }
                                }
                            }
                        }

                        onEditClicked: function(folderId, folderName) {
                            updateFolderDialog.folderName = folderName
                            updateFolderDialog.openAndDoOnAccepted(function() {
                                folderDelegate.model.display_name = updateFolderDialog.folderName
                            });
                        }
                    }
                }
            }
        }

        ColumnLayout {
            id: listHeader
            anchors.left: libraryNavFocusScope.right
            anchors.leftMargin: 16
            anchors.top: parent.top
            anchors.right: parent.right
            height: 64
            spacing: 8
            RowLayout {
                Layout.fillWidth: true
                Text {
                    text: libraryButtonGroup.checkedButton.sectionTitle
                    font.pixelSize: 24
                    font.family: Constants.regularFontFamily
                    font.weight: Font.DemiBold
                    color: "white"
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    // height: 64
                }
                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }

                Text {
                    text: "Sort by:"
                    font.pixelSize: 16
                    font.family: Constants.regularFontFamily
                    font.weight: Font.DemiBold
                    color: "white"
                    verticalAlignment: Text.AlignVCenter
                    Layout.rightMargin: 8
                 }

                MyComboBox {
                   id: comboBox
                   model: [
                       { text: "A-Z", sortRole: "alphabetical" },
                       { text: "Recently played", sortRole: "recent" },
                       { text: "Newest first", sortRole: "newest"}
                   ]

                   property bool initialized: false

                   currentIndex: {
                      for (var i = 0; i < comboBox.count; i++) {
                        if (comboBox.valueAt(i) === GeneralSettings.librarySortMethod) {
                             return i
                        }
                      }
                      return 0
                   }

                   onCurrentIndexChanged: {
                        let val = comboBox.valueAt(currentIndex)
                        FilteredLibraryEntryModel.sortMethod = val
                        GeneralSettings.librarySortMethod = val
                   }

                    // Component.onCompleted: {
                    //     // Set the initial sort method
                    //     for (var i = 0; i < comboBox.count; i++) {
                    //         if (comboBox.valueAt(i) === GeneralSettings.librarySortMethod) {
                    //             comboBox.currentIndex = i
                    //             break
                    //         }
                    //     }
                    //     initialized = true
                    //     FilteredLibraryEntryModel.sortMethod = GeneralSettings.librarySortMethod
                    //     console.log("initialized")
                    //
                    // }
                   //
                   //
                   // onCurrentValueChanged: {
                   //     if (!initialized) {
                   //          console.log("not initialized")
                   //         return
                   //     };
                   //     FilteredLibraryEntryModel.sortMethod = currentValue
                   //     GeneralSettings.librarySortMethod = currentValue
                   // }

                   textRole: "text"
                   valueRole: "sortRole"
               }
                // BusyIndicator {
                //     Layout.preferredHeight: 40
                //     running: LibraryScanner.scanning
                //     palette.dark: "white"
                // }
                // Text {
                //     text: LibraryScanner.scanning ? "Scanning..." : ""
                //     font.pixelSize: 18
                //     font.family: Constants.regularFontFamily
                //     font.weight: Font.Medium
                //     color: "white"
                //     Layout.rightMargin: 16
                //     horizontalAlignment: Text.AlignRight
                //     verticalAlignment: Text.AlignVCenter
                // }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: "#4B4B4B"
            }
        }


        ListView {
            id: theList
            anchors.left: libraryNavFocusScope.right
            anchors.leftMargin: 16
            anchors.top: listHeader.bottom
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            highlightMoveDuration: 80
            highlightMoveVelocity: -1
            highlightRangeMode: InputMethodManager.usingMouse ? ListView.NoHighlightRange : ListView.ApplyRange
            preferredHighlightBegin: 64
            preferredHighlightEnd: height - 64
            ScrollBar.vertical: ScrollBar { }
            model: FilteredLibraryEntryModel
            focus: true

            KeyNavigation.left: libraryNavFocusScope

            clip: true

            delegate: LibraryEntryListDelegate {
                onStartGameClicked: function(entryId) {
                    root.startGame(entryId)
                }

                ContextMenu.menu: LibraryEntryRightClickMenu {
                    entryId: model.entryId

                    enableAddToFolder: folderList.count > 0
                    showRemoveFromFolder: FilteredLibraryEntryModel.folderId !== -1
                    showManageSaveData: root.currentEntryId !== entryId

                    onStartGameClicked: function(entryId) {
                        root.startGame(entryId)
                    }

                    onEditEntryClicked: function(entryId) {
                        editEntryDialog.text = model.displayName
                        editEntryDialog.openAndDoOnAccepted(function() {
                            model.displayName = editEntryDialog.text
                        });
                    }

                    onRemoveFromFolderClicked: function(entryId) {
                        LibraryEntryModel.removeEntryFromFolder(entryId, FilteredLibraryEntryModel.folderId)
                    }

                    onManageSaveDataClicked: function(entryId) {
                        manageSaveDataDialog.entryId = entryId
                        manageSaveDataDialog.open()
                    }
                }
            }
        }
    }
}