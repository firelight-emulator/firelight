import QtQuick
import QtQuick.Controls
// import QtQuick.Controls.Material
import QtQuick.Dialogs
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Effects
import Firelight 1.0

FirelightDialog {
    id: control
    headerText: "Manage save data"
    showCancel: false
    acceptText: "Close"
    centerButtons: false

    property alias entryId: saveDataInfo.entryId

    SaveDataInformation {
        id: saveDataInfo
    }

    // width: Math.min(parent.width, 800)

    contentItem: ListView {
        implicitHeight: 540
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        currentIndex: 0

        ScrollBar.vertical: ScrollBar {
        }

        model: saveDataInfo.saveFiles
        delegate: Button {
            id: delegateButton
            required property var model
            height: 80
            width: ListView.view.width

            TapHandler {
                acceptedButtons: Qt.RightButton
                onSingleTapped: function(event, button) {
                    if (button === 2) {
                        saveFileRightClickMenu.popupNormal()
                    }
                }
            }

            RightClickMenu {
                id: saveFileRightClickMenu
                RightClickMenuItem {
                    text: "Make active"
                    enabled: delegateButton.model.slotNumber !== saveDataInfo.activeSaveSlotNumber
                    onClicked: {
                        saveDataInfo.activeSaveSlotNumber = delegateButton.model.slotNumber
                    }
                }
                RightClickMenuItem {
                    text: "Show in File Explorer"
                    enabled:  delegateButton.model.hasData
                    onClicked: {
                        FilesystemUtils.showInFilesystem(delegateButton.model.filePath)
                    }
                }
            }

            HoverHandler {
                id: hoverHandler
                // cursorShape: Qt.PointingHandCursor
            }
            background: Rectangle {
                  color: "white"
                  radius: 2
                  opacity: hoverHandler.hovered ? 0.1 : 0
              }
            contentItem: RowLayout {
                spacing: 16
                // Text {
                //     text: model.name
                //     font.pixelSize: 16
                //     color: ColorPalette.neutral400
                //     Layout.alignment: Qt.AlignVCenter
                // }
                // Rectangle {
                //     implicitWidth: height * 16 / 9
                //     Layout.fillHeight: true
                //     Layout.alignment: Qt.AlignVCenter
                //     color: "#272727"
                //
                //     Text {
                //         text: "No save data"
                //         color: ColorPalette.neutral400
                //         font.pixelSize: 14
                //         anchors.centerIn: parent
                //         visible: !model.hasData
                //     }
                // }

                ColumnLayout {
                    Layout.alignment: Qt.AlignVCenter
                    Text {
                        text: "Slot " + model.slotNumber
                        font.pixelSize: 16
                        color: ColorPalette.neutral100
                        Layout.alignment: Qt.AlignLeft | Qt.AlignBottom
                    }

                    Text {
                        text: model.lastModified
                        font.pixelSize: 16
                        color: ColorPalette.neutral400
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        visible: model.hasData
                    }

                    Text {
                        text: "No data"
                        font.pixelSize: 16
                        color: ColorPalette.neutral400
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        visible: !model.hasData
                    }
                }
                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }

                Pane {
                    Layout.alignment: Qt.AlignVCenter
                    visible: delegateButton.model.slotNumber === saveDataInfo.activeSaveSlotNumber
                    verticalPadding: 8
                    horizontalPadding: 30
                    background: Rectangle {
                        color: "#155f9c"
                        radius: 8
                    }
                    contentItem: Text {
                        text: "Active"
                        font.pixelSize: 14
                        font.family: Constants.regularFontFamily
                        font.weight: Font.DemiBold
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                    }
                }

                // FLIcon {
                //     Layout.alignment: Qt.AlignVCenter
                //     size: 28
                //     icon: "arrow-forward"
                // }
            }
        }
    }
}