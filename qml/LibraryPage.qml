import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FirelightStyle 1.0

Item {
    id: root

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

        handle: FirelightSplitViewHandle {
        }

        Item {
            SplitView.minimumWidth: 260
            SplitView.maximumWidth: parent.width / 2

            ColumnLayout {
                id: menu
                anchors.fill: parent
                spacing: 4

                // Item {
                //     Layout.fillWidth: true
                //     Layout.fillHeight: true
                // }

                FirelightMenuItem {
                    labelText: "All Games"
                    labelIcon: "\uf53e"
                    Layout.preferredWidth: parent.width
                    Layout.preferredHeight: 40
                    Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                }
                FirelightMenuItem {
                    labelText: "Recently Played"
                    labelIcon: "\ue889"
                    Layout.preferredWidth: parent.width
                    Layout.preferredHeight: 40
                    Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                }
                FirelightMenuItem {
                    labelText: "Newly Added"
                    labelIcon: "\ue838"
                    Layout.preferredWidth: parent.width
                    Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                    Layout.preferredHeight: 40
                }
                RowLayout {
                    Layout.preferredWidth: parent.width
                    Layout.preferredHeight: 40
                    Text {
                        text: "Folders"
                        color: "#c6c6c6"
                        font.pointSize: 12
                        font.family: Constants.regularFontFamily
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        leftPadding: 8
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Text {
                        text: "\ue145"
                        font.pixelSize: 24
                        font.family: Constants.symbolFontFamily
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        color: "#dadada"
                    }
                }
                Rectangle {
                    Layout.preferredWidth: parent.width
                    Layout.preferredHeight: 1
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    opacity: 0.3
                    color: "#dadada"
                }
                FirelightMenuItem {
                    labelText: "Test Folder One"
                    Layout.preferredWidth: parent.width
                    Layout.preferredHeight: 40
                    Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                }
                FirelightMenuItem {
                    labelText: "Some Other Stuff"
                    Layout.preferredWidth: parent.width
                    Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                    Layout.preferredHeight: 40
                }

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
            }

        }

        Item {

        }
    }


}