import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.platform

FocusScope {
    ColumnLayout {
        spacing: 8
        anchors.fill: parent

        SettingsSectionHeader {
            sectionName: "Content directories"
            showTopPadding: false
        }

        ListView {
            Layout.fillWidth: true
            Layout.preferredHeight: (count * 60) + (spacing * (count - 1))
            model: ContentDirectoryModel
            interactive: false

            FolderDialog {
                id: folderDialog
                property var doOnAccepted: null
                onAccepted: {
                    if (folderDialog.doOnAccepted) {
                        folderDialog.doOnAccepted(folderDialog.folder)
                    }
                    // console.log("found it")
                }
            }

            delegate: Button {
                id: folderDelegate
                required property var model
                implicitWidth: ListView.view.width
                implicitHeight: 60
                ContextMenu.menu: RightClickMenu {
                    RightClickMenuItem {
                        text: "Edit"

                        onTriggered: {
                          folderDialog.currentFolder = FilesystemUtils.prependFileURI(model.path)
                          folderDialog.doOnAccepted = function (folder) {
                              folderDelegate.model.path = FilesystemUtils.removeFileURI(folder.toString())
                          }
                          folderDialog.open()
                        }
                    }

                    RightClickMenuItem {
                        text: "Delete"
                        dangerous: true

                        onTriggered: {
                              console.log("Deleting directory: " + folderDelegate.model.directory_id)
                              ContentDirectoryModel.deleteItem(folderDelegate.model.directory_id)
                        }
                    }
                }
                background: Rectangle {
                    color: "white"
                    opacity: parent.hovered || (!InputMethodManager.usingMouse && parent.activeFocus) ? 0.1 : 0
                    radius: 4

                    Behavior on opacity {
                        NumberAnimation {
                            duration: 100
                            easing.type: Easing.InOutQuad
                        }
                    }
                }
                contentItem: RowLayout {
                    spacing: 12
                    FLIcon {
                        icon: "folder"
                        filled: true
                        Layout.fillHeight: true
                        size: height * 2 / 3
                        color: ColorPalette.neutral100
                    }
                    Text {
                        text: model.path
                        font.pixelSize: 17
                        font.family: Constants.regularFontFamily
                        font.weight: Font.DemiBold
                        color: ColorPalette.neutral100
                        verticalAlignment: Text.AlignVCenter
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                    }
                }
            }
        }

        FirelightButton {
            label: "Add directory"
            onClicked: {
              folderDialog.currentFolder = ""
              folderDialog.doOnAccepted = function (folder) {
                  ContentDirectoryModel.addItem(FilesystemUtils.removeFileURI(folder.toString()))
              }
              folderDialog.open()
            }
            // Layout.alignment: Qt.AlignRight
        }

        // DirectoryOption {
        //     id: gameDirectoryOption
        //     Layout.fillWidth: true
        //     label: "Game directory"
        //     focus: true
        //     value: UserLibrary.mainGameDirectory
        //
        //     background: Rectangle {
        //         color: ColorPalette.neutral300
        //         radius: 6
        //         // border.color: ColorPalette.neutral700
        //         opacity: parent.hovered || (!InputMethodManager.usingMouse && parent.activeFocus) ? 0.2 : 0.1
        //
        //         Behavior on opacity {
        //             NumberAnimation {
        //                 duration: 100
        //                 easing.type: Easing.InOutQuad
        //             }
        //         }
        //
        //     }
        //
        //     onValueChanged: function () {
        //         UserLibrary.mainGameDirectory = value
        //     }
        // }
        //
        // Text {
        //     font.pixelSize: 15
        //     font.family: Constants.regularFontFamily
        //     font.weight: Font.Medium
        //     color: ColorPalette.neutral100
        //     text: "This is where you’ll put your game files. Firelight will automatically detect files in this directory and add them to your library."
        //     leftPadding: 12
        //     Layout.bottomMargin: 20
        // }

        SettingsSectionHeader {
            sectionName: "Saves directory"
            showTopPadding: true
        }

        DirectoryOption {
            id: saveDirectoryOption
            KeyNavigation.up: gameDirectoryOption
            Layout.fillWidth: true
            label: "Saves directory"
            value: SaveManager.saveDirectory

            background: Rectangle {
                color: ColorPalette.neutral300
                radius: 6
                // border.color: ColorPalette.neutral700
                opacity: parent.hovered || (!InputMethodManager.usingMouse && parent.activeFocus) ? 0.2 : 0.1

                Behavior on opacity {
                    NumberAnimation {
                        duration: 100
                        easing.type: Easing.InOutQuad
                    }
                }

            }

            onValueChanged: function () {
                SaveManager.saveDirectory = value
            }
        }

        Text {
            font.pixelSize: 15
            font.family: Constants.regularFontFamily
            font.weight: Font.Medium
            color: ColorPalette.neutral100
            text: "This is where Firelight will save your save files and Suspend Point data."
            leftPadding: 12
            Layout.bottomMargin: 20
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

        }
    }
}
