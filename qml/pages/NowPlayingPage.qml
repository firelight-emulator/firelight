import QtQuick
import QtQuick.Controls
import QtQuick.Layouts


FocusScope {
    id: root

    signal resumeGamePressed()

    signal restartGamePressed()

    signal backToMainMenuPressed()

    signal closeGamePressed()

    signal rewindPressed()

    Item {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 40
    }
    // NavigationTabBar {
    //     id: navBar
    //     anchors.horizontalCenter: parent.horizontalCenter
    //
    //     tabs: ["Stuff", "Controller", "Achievements", "Settings"]
    //     tabWidth: 150
    //     height: 40
    // }

    RowLayout {
        id: contentRow
        // anchors.top: navBar.bottom
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        spacing: 0

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.horizontalStretchFactor: 1
        }

        ButtonGroup {
            id: navButtonGroup
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
                alignRight: true

                onClicked: function () {
                    rightSide.clear(StackView.PopTransition)
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
                alignRight: true

                onClicked: function () {
                    rightSide.clear(StackView.PopTransition)
                    restartGamePressed()
                }
            }
            FirelightMenuItem {
                labelText: "Rewind"
                Layout.fillWidth: true
                // Layout.preferredWidth: parent.width / 2
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.preferredHeight: 40
                enabled: true
                checkable: false
                alignRight: true

                onClicked: function () {
                    rightSide.clear(StackView.PopTransition)
                    rewindPressed()
                }
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
                checkable: true
                enabled: true
                alignRight: true

                ButtonGroup.group: navButtonGroup

                onToggled: function (checked) {
                    if (rightSide.depth === 0) {
                        rightSide.replaceCurrentItem(suspendPoints, {}, StackView.PushTransition)
                    }
                    console.log("clicked create")
                }
            }
            FirelightMenuItem {
                labelText: "Load Suspend Point"
                Layout.fillWidth: true
                // Layout.preferredWidth: parent.width / 2
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.preferredHeight: 40
                checkable: true
                enabled: true
                alignRight: true

                ButtonGroup.group: navButtonGroup

                onToggled: function (checked) {
                    if (rightSide.depth === 0) {
                        rightSide.replaceCurrentItem(suspendPoints, {}, StackView.PushTransition)
                    }
                    console.log("clicked load")
                }
            }
            FirelightMenuItem {
                id: undo
                labelText: "Undo Last Load"
                Layout.fillWidth: true
                // Layout.preferredWidth: parent.width / 2
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.preferredHeight: 40
                checkable: false
                enabled: true
                alignRight: true

                onClicked: function () {
                    console.log(undo.ButtonGroup.group)
                }
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
                alignRight: true
                enabled: false

                onClicked: function () {
                    rightSide.clear(StackView.PopTransition)
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
                alignRight: true

                onClicked: function () {
                    navButtonGroup.checkedButton.checked = false
                    rightSide.clear(StackView.PopTransition)
                    closeGameConfirmationDialog.open()
                }
            }
        }

        StackView {
            id: rightSide
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.horizontalStretchFactor: 4

            pushEnter: Transition {
                NumberAnimation {
                    property: "opacity"
                    from: 0.0
                    to: 1.0
                    duration: 200
                }
                NumberAnimation {
                    property: "x"
                    from: 20
                    to: 0
                    duration: 200
                }
            }

            popExit: Transition {
                NumberAnimation {
                    property: "opacity"
                    from: 1.0
                    to: 0.0
                    duration: 200
                }
                NumberAnimation {
                    property: "x"
                    from: 0;
                    to: 20
                    duration: 200
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
        id: suspendPoints
        Item {
            ListView {
                width: Math.min(800, parent.width - 64)
                height: parent.height
                anchors.horizontalCenter: parent.horizontalCenter
                model: 40
                spacing: 12

                header: Rectangle {
                    height: 140
                    width: ListView.view.width
                    color: "transparent"
                }
                delegate: Button {
                    id: dele
                    padding: 6
                    required property var index
                    width: ListView.view.width
                    height: 120
                    hoverEnabled: true
                    background: Rectangle {
                        color: dele.hovered && !details.hovered ? ColorPalette.neutral700 : ColorPalette.neutral800
                        opacity: 0.8
                        radius: 8
                    }
                    contentItem: Item {
                        DetailsButton {
                            id: details
                            anchors.top: parent.top
                            anchors.right: parent.right
                            width: 36
                            height: 36
                        }
                        Rectangle {
                            id: pic
                            anchors.top: parent.top
                            anchors.bottom: parent.bottom
                            anchors.left: parent.left
                            width: parent.height
                        }
                        Text {
                            id: label
                            anchors.top: parent.top
                            anchors.right: parent.right
                            anchors.left: pic.right
                            height: parent.height / 2
                            topPadding: 8
                            bottomPadding: 8
                            leftPadding: 10
                            rightPadding: 10
                            font.pixelSize: 16
                            font.family: Constants.regularFontFamily
                            font.weight: Font.DemiBold
                            horizontalAlignment: Text.AlignLeft
                            lineHeight: 1.2
                            verticalAlignment: Text.AlignTop
                            color: ColorPalette.neutral100
                            wrapMode: Text.WordWrap

                            text: "Suspend point " + dele.index
                        }
                        Text {
                            id: date
                            anchors.top: label.bottom
                            anchors.right: parent.right
                            anchors.left: pic.right
                            anchors.bottom: parent.bottom
                            topPadding: 8
                            bottomPadding: 8
                            leftPadding: 10
                            rightPadding: 10
                            font.pixelSize: 15
                            font.family: Constants.regularFontFamily
                            font.weight: Font.DemiBold
                            horizontalAlignment: Text.AlignLeft
                            lineHeight: 1.2
                            verticalAlignment: Text.AlignBottom
                            color: ColorPalette.neutral400
                            wrapMode: Text.WordWrap

                            text: "Last updated 10/4/2024 5:12:25"
                        }

                    }
                    // contentItem: Rectangle {
                    //     color: "red"
                    // }
                }
            }
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