import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FirelightStyle 1.0

Item {
    id: root

    signal resumeGamePressed()

    signal restartGamePressed()

    signal backToMainMenuPressed()

    signal closeGamePressed()

    RowLayout {
        id: contentRow
        anchors.fill: parent
        spacing: 0

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.horizontalStretchFactor: 1
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.horizontalStretchFactor: 2

            FirelightMenuItem {
                labelText: "Resume Game"
                Layout.fillWidth: true
                // Layout.preferredWidth: parent.width / 2
                Layout.preferredHeight: 40
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                checkable: false

                onClicked: function () {
                    resumeGamePressed()
                }
            }
            FirelightMenuItem {
                labelText: "Restart Game"
                Layout.fillWidth: true
                // Layout.preferredWidth: parent.width / 2
                Layout.preferredHeight: 40
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                checkable: false

                onClicked: function () {
                    restartGamePressed()
                }
            }
            FirelightMenuItem {
                labelText: "Rewind"
                Layout.fillWidth: true
                // Layout.preferredWidth: parent.width / 2
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.preferredHeight: 40
                enabled: false
            }
            Rectangle {
                Layout.fillWidth: true
                // Layout.preferredWidth: parent.width / 2
                Layout.preferredHeight: 1
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                opacity: 0.3
                color: "#dadada"
            }
            FirelightMenuItem {
                labelText: "Create Suspend Point"
                Layout.fillWidth: true
                // Layout.preferredWidth: parent.width / 2
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.preferredHeight: 40
                enabled: false
            }
            FirelightMenuItem {
                labelText: "Load Suspend Point"
                Layout.fillWidth: true
                // Layout.preferredWidth: parent.width / 2
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.preferredHeight: 40
                enabled: false
            }
            FirelightMenuItem {
                labelText: "Undo Last Load"
                Layout.fillWidth: true
                // Layout.preferredWidth: parent.width / 2
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.preferredHeight: 40
                enabled: false
            }
            Rectangle {
                Layout.fillWidth: true
                // Layout.preferredWidth: parent.width / 2
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.preferredHeight: 1
                opacity: 0.3
                color: "#dadada"
            }
            FirelightMenuItem {
                labelText: "Back to Main Menu"
                Layout.fillWidth: true
                // Layout.preferredWidth: parent.width / 2
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.preferredHeight: 40
                checkable: false

                onClicked: function () {
                    backToMainMenuPressed()
                }
            }
            FirelightMenuItem {
                labelText: "Close Game"
                Layout.fillWidth: true
                // Layout.preferredWidth: parent.width / 2
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.preferredHeight: 40
                checkable: false

                onClicked: function () {
                    closeGameConfirmationDialog.open()
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.horizontalStretchFactor: 3
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.horizontalStretchFactor: 1
        }
    }

    FirelightDialog {
        id: closeGameConfirmationDialog
        text: "Are you sure you want to close the game?"

        onAccepted: {
            closeGamePressed()
        }
    }

    // Pane {
    //     id: header
    //     anchors.top: parent.top
    //     anchors.left: parent.left
    //     anchors.right: parent.right
    //     background: Item {
    //     }
    //
    //     Column {
    //         anchors.fill: parent
    //         spacing: 8
    //
    //         Text {
    //             text: "Now Playing"
    //             color: "#dadada"
    //             font.pointSize: 24
    //             font.family: Constants.semiboldFontFamily
    //             horizontalAlignment: Text.AlignLeft
    //             verticalAlignment: Text.AlignVCenter
    //         }
    //
    //         Rectangle {
    //             width: parent.width
    //             height: 1
    //             opacity: 0.3
    //             color: "#dadada"
    //         }
    //     }
    // }
    //
    // Item {
    //     id: leftHalf
    //     anchors.left: parent.left
    //     anchors.top: header.bottom
    //     anchors.bottom: parent.bottom
    //     width: parent.width / 4
    //
    //     ColumnLayout {
    //         id: menu
    //         anchors.fill: parent
    //         spacing: 4
    //
    //         FirelightMenuItem {
    //             labelText: "Resume Game"
    //             Layout.preferredWidth: parent.width
    //             Layout.preferredHeight: 40
    //             Layout.alignment: Qt.AlignLeft | Qt.AlignTop
    //             checkable: false
    //         }
    //         FirelightMenuItem {
    //             labelText: "Restart Game"
    //             Layout.preferredWidth: parent.width
    //             Layout.preferredHeight: 40
    //             Layout.alignment: Qt.AlignLeft | Qt.AlignTop
    //             checkable: false
    //         }
    //         FirelightMenuItem {
    //             labelText: "Rewind"
    //             Layout.preferredWidth: parent.width
    //             Layout.alignment: Qt.AlignLeft | Qt.AlignTop
    //             Layout.preferredHeight: 40
    //             enabled: false
    //         }
    //         Rectangle {
    //             Layout.preferredWidth: parent.width
    //             Layout.preferredHeight: 1
    //             Layout.alignment: Qt.AlignLeft | Qt.AlignTop
    //             opacity: 0.3
    //             color: "#dadada"
    //         }
    //         FirelightMenuItem {
    //             labelText: "Create Suspend Point"
    //             Layout.preferredWidth: parent.width
    //             Layout.alignment: Qt.AlignLeft | Qt.AlignTop
    //             Layout.preferredHeight: 40
    //             enabled: false
    //         }
    //         FirelightMenuItem {
    //             labelText: "Load Suspend Point"
    //             Layout.preferredWidth: parent.width
    //             Layout.alignment: Qt.AlignLeft | Qt.AlignTop
    //             Layout.preferredHeight: 40
    //             enabled: false
    //         }
    //         FirelightMenuItem {
    //             labelText: "Undo Last Load"
    //             Layout.preferredWidth: parent.width
    //             Layout.alignment: Qt.AlignLeft | Qt.AlignTop
    //             Layout.preferredHeight: 40
    //             enabled: false
    //         }
    //         Rectangle {
    //             Layout.preferredWidth: parent.width
    //             Layout.alignment: Qt.AlignLeft | Qt.AlignTop
    //             Layout.preferredHeight: 1
    //             opacity: 0.3
    //             color: "#dadada"
    //         }
    //         FirelightMenuItem {
    //             labelText: "Quit Game"
    //             Layout.preferredWidth: parent.width
    //             Layout.alignment: Qt.AlignLeft | Qt.AlignTop
    //             Layout.preferredHeight: 40
    //             checkable: false
    //         }
    //
    //         Item {
    //             Layout.fillWidth: true
    //             Layout.fillHeight: true
    //         }
    //     }
    // }

    // Item {
    //     id: rightHalf
    //     anchors.right: parent.right
    //     anchors.top: header.bottom
    //     anchors.bottom: parent.bottom
    //     anchors.left: leftHalf.right
    //
    //
    //     Image {
    //         id: preview
    //         anchors.centerIn: parent
    //         smooth: false
    //
    //         source: "file:pmscreenshot.jpg"
    //         // fillMode: Image.PreserveAspectFit
    //     }
    // }


}