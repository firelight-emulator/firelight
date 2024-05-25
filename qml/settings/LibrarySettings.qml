import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.platform

Item {
    ColumnLayout {
        spacing: 8
        anchors.fill: parent
        // ComboBoxOption {
        //     Layout.fillWidth: true
        //     Layout.preferredHeight: 60
        //     label: "Window Mode?"
        // }

        Text {
            // Layout.topMargin: 30
            Layout.fillWidth: true
            text: qsTr("Game directories")
            color: "white"
            font.pointSize: 12
            // font.pointSize: 11
            font.family: Constants.regularFontFamily
            font.weight: Font.DemiBold
            // color: "#a6a6a6"
        }

        Text {
            Layout.fillWidth: true
            text: qsTr("Firelight will automatically watch these directories for game files and update your library accordingly.")
            color: "#c1c1c1"
            wrapMode: Text.WordWrap
            font.pointSize: 11
            font.family: Constants.regularFontFamily
            Layout.bottomMargin: 8
        }

        ListView {
            spacing: 8
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: ListModel {
                ListElement { name: "file:///C:/Users/alexs/AppData/Roaming/Firelight/roms" }
                // ListElement { name: "C:/Users/alexs/otherplace" }
                // ListElement { name: "C:/gamez" }
            }
            delegate: ColumnLayout {
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
                        text: folderDialog.folder
                        font.family: Constants.regularFontFamily
                        font.pointSize: 12
                        color: "white"
                        verticalAlignment: Text.AlignVCenter
                        readOnly: true
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Text {
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        // height: parent.height / 2
                        text: "Contains 0 games"
                        color: "#c1c1c1"
                        wrapMode: Text.WordWrap
                        font.pointSize: 10
                        font.family: Constants.regularFontFamily
                        verticalAlignment: Text.AlignTop
                        horizontalAlignment: Text.AlignLeft
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
                            font.family: Constants.regularFontFamily
                            font.pointSize: 10
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            color: "#020202"
                        }
                        background: Rectangle {
                            color: "#d6d6d6"
                            radius: 4
                        }

                        onClicked: function(){
                            folderDialog.open()
                        }

                        FolderDialog {
                            id: folderDialog
                            folder: model.name
                            // currentFolder: model.name
                            onAccepted: {
                                console.log("found it")
                            }
                        }
                    }

                    Button {
                        id: but
                        Layout.fillHeight: true
                        Layout.preferredWidth: 100

                        // onHeightChanged: function() {
                        //     width = height
                        // }

                        contentItem: Text {
                            text: "Remove"
                            font.family: Constants.regularFontFamily
                            font.pointSize: 10
                            font.weight: Font.DemiBold
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            color: "white"
                        }
                        background: Rectangle {
                            color: "#c60e0e"
                            radius: 4
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



        // Pane {
        //     Layout.fillWidth: true
        //     padding: 4
        //
        //     background: Rectangle {
        //         color: "transparent"
        //         radius: 8
        //     }
        //
        //     contentItem: TextInput {
        //         padding: 4
        //         text: "C:/Users/alexs/otherplace"
        //         font.family: Constants.regularFontFamily
        //         font.pointSize: 12
        //         color: "white"
        //         verticalAlignment: Text.AlignVCenter
        //         readOnly: true
        //     }
        // }
        //
        // Button {
        //     Layout.alignment: Qt.AlignRight
        //     contentItem: Text {
        //         text: "\ue872"
        //         font.family: Constants.symbolFontFamily
        //         font.pixelSize: 24
        //         horizontalAlignment: Text.AlignHCenter
        //         verticalAlignment: Text.AlignVCenter
        //         color: "white"
        //     }
        //     background: Rectangle {
        //         color: "#c60e0e"
        //         radius: 4
        //     }
        //     Layout.preferredHeight: 40
        //     Layout.preferredWidth: 40
        // }
        //
        // Rectangle {
        //     Layout.fillWidth: true
        //     Layout.preferredHeight: 1
        //     color: "#333333"
        // }
        //
        // Pane {
        //     Layout.fillWidth: true
        //     padding: 4
        //
        //     background: Rectangle {
        //         color: "transparent"
        //         radius: 8
        //     }
        //
        //     contentItem: TextInput {
        //         padding: 4
        //         text: "C:/gamez"
        //         font.family: Constants.regularFontFamily
        //         font.pointSize: 12
        //         color: "white"
        //         verticalAlignment: Text.AlignVCenter
        //         readOnly: true
        //     }
        // }
        //
        // Button {
        //     Layout.alignment: Qt.AlignRight
        //     contentItem: Text {
        //         text: "\ue872"
        //         font.family: Constants.symbolFontFamily
        //         font.pixelSize: 24
        //         horizontalAlignment: Text.AlignHCenter
        //         verticalAlignment: Text.AlignVCenter
        //         color: "white"
        //     }
        //     background: Rectangle {
        //         color: "#c60e0e"
        //         radius: 4
        //     }
        //     Layout.preferredHeight: 40
        //     Layout.preferredWidth: 40
        // }
        //
        // Rectangle {
        //     Layout.fillWidth: true
        //     Layout.preferredHeight: 1
        //     color: "#333333"
        // }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

        }
    }
}