import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.platform

Item {
    ColumnLayout {
        spacing: 8
        anchors.fill: parent

        Text {
            // Layout.topMargin: 30
            Layout.fillWidth: true
            text: qsTr("Game directory")
            color: Theme.textPrimary
            font.pixelSize: AppStyle.fontSizeMedium
            // font.pointSize: 11
            font.family: AppStyle.fontFamily
            font.weight: Font.DemiBold
            // color: "#a6a6a6"
        }

        Text {
            Layout.fillWidth: true
            text: qsTr("Firelight will automatically watch this directory for game files and update your library accordingly. Later you'll be able to add more!")
            color: Theme.textMuted
            wrapMode: Text.WordWrap
            font.pixelSize: AppStyle.fontSizeMedium
            font.family: AppStyle.fontFamily
            Layout.bottomMargin: 8
        }

        ListView {
            spacing: 8
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: library_scan_path_model
            delegate: ColumnLayout {
                required property var model
                width: parent.width
                height: 100

                spacing: 8

                Pane {
                    id: texxxxt
                    Layout.fillWidth: true
                    padding: 4

                    background: Rectangle {
                        color: "black"
                        radius: 8
                    }

                    contentItem: TextInput {
                        padding: 4
                        text: model.path
                        font.family: AppStyle.fontFamily
                        font.pixelSize: AppStyle.fontSizeMedium
                        color: Theme.textPrimary
                        verticalAlignment: Text.AlignVCenter
                        readOnly: true
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    Item {
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                    }

                    Button {
                        id: but1
                        Layout.fillHeight: true
                        Layout.preferredWidth: 100

                        // onHeightChanged: function() {
                        //     width = height
                        // }

                        contentItem: Text {
                            text: "Change folder"
                            font.family: AppStyle.fontFamily
                            font.pixelSize: AppStyle.fontSizeSmall
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            color: "#020202"
                        }
                        background: Rectangle {
                            color: "#d6d6d6"
                            radius: 4
                        }

                        onClicked: function () {
                            folderDialog.open();
                        }

                        FolderDialog {
                            id: folderDialog
                            currentFolder: model.local_filename
                            onAccepted: {
                                model.path = folder;
                                // console.log("found it")
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.bottomMargin: 8
                    Layout.preferredHeight: 1
                    color: "#333333"
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
