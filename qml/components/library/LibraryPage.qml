import QtQuick
import QtQuick.Controls
import QtQuick.Layouts 1.0
import QtQml.Models
import Firelight 1.0

FocusScope {
    id: root

    signal startGame(int entryId)
    required property int currentEntryId

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
    ManageSaveDataDialog {
        id: manageSaveDataDialog
    }
    ColumnLayout {
            id: libraryNavList
            focus: true
        anchors.left: parent.left
        anchors.topMargin: 64
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 240
        FirelightMenuItem {
          id: allGamesButton
          focus: true

          property string sectionTitle: "All games"

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

            interactive: contentHeight > height

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

    ListView {
        id: theList
        anchors.left: libraryNavList.right
        anchors.leftMargin: 16
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        highlightMoveDuration: 80
        highlightMoveVelocity: -1
        highlightRangeMode: InputMethodManager.usingMouse ? ListView.NoHighlightRange : ListView.ApplyRange
        preferredHighlightBegin: 64
        preferredHighlightEnd: height - 64
        model: FilteredLibraryEntryModel
        ScrollBar.vertical: ScrollBar { }

        // headerPositioning: ListView.PullBackHeader

        header: ColumnLayout {
            height: 64
            width: ListView.view.width
            spacing: 8
            Text {
                text: libraryButtonGroup.checkedButton.sectionTitle
                font.pixelSize: 24
                font.family: Constants.regularFontFamily
                font.weight: Font.DemiBold
                color: "white"
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                // height: 64
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: "#4B4B4B"
            }

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
            }
        }

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