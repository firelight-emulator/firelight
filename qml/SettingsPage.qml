import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    Rectangle {
        color: "#101114"
        height: parent.height
        width: leftSpacer.width + menu.width + contentPane.horizontalPadding + 6
    }

    RowLayout {
        id: contentRow
        anchors.fill: parent
        spacing: 0

        Item {
            id: leftSpacer
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.horizontalStretchFactor: 1
        }

        Pane {
            id: contentPane
            Layout.fillWidth: true
            Layout.horizontalStretchFactor: 6
            Layout.fillHeight: true

            verticalPadding: 80

            background: Item {
            }

            ColumnLayout {
                id: menu
                spacing: 4
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: 200

                FirelightMenuItem {
                    labelText: "Appearance"
                    // labelIcon: "\ue40a"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 40
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    enabled: false
                }
                FirelightMenuItem {
                    labelText: "Library"
                    // labelIcon: "\ue40a"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 40
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    onToggled: {
                        if (checked) {
                            rightHalf.replace(librarySettings)
                        }
                    }
                }
                FirelightMenuItem {
                    labelText: "Achievements"
                    // labelIcon: "\ue897"
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    Layout.preferredHeight: 40
                    onToggled: {
                        if (checked) {
                            rightHalf.replace(retroAchievementSettings)
                        }
                    }
                }

                FirelightMenuItem {
                    labelText: "Audio / Video"
                    // labelIcon: "\ue333"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 40
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    checked: true
                    onToggled: {
                        if (checked) {
                            rightHalf.replace(videoSettings)
                        }
                    }
                }
                FirelightMenuItem {
                    labelText: "System"
                    // labelIcon: "\uf522"
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    Layout.preferredHeight: 40
                    enabled: false
                }
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    opacity: 0.3
                    color: "#dadada"
                }
                FirelightMenuItem {
                    labelText: "Privacy"
                    // labelIcon: "\ue897"
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    Layout.preferredHeight: 40
                    enabled: false
                }
                FirelightMenuItem {
                    labelText: "About"
                    // labelIcon: "\ue88e"
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    Layout.preferredHeight: 40
                    enabled: false
                }
                FirelightMenuItem {
                    labelText: "Debug"
                    // labelIcon: "\ue88e"
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    Layout.preferredHeight: 40
                    onToggled: {
                        if (checked) {
                            rightHalf.replace(debugSettings)
                        }
                    }
                }

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
            }

            StackView {
                id: rightHalf

                anchors.top: parent.top
                anchors.left: menu.right
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.leftMargin: 36

                initialItem: videoSettings

                pushEnter: Transition {
                }

                pushExit: Transition {
                }

                popEnter: Transition {
                }

                popExit: Transition {
                }

                replaceEnter: Transition {
                }

                replaceExit: Transition {
                }
            }

            // Pane {
            //     id: header
            //
            //     Layout.fillWidth: true
            //     Layout.preferredHeight: 120
            //     background: Rectangle {
            //         color: "transparent"
            //         // border.color: "red"
            //     }
            //
            //     horizontalPadding: 8
            //
            //     Text {
            //         anchors.fill: parent
            //         text: "Settings"
            //         color: "white"
            //         font.pointSize: 26
            //         font.family: Constants.regularFontFamily
            //         font.weight: Font.DemiBold
            //         horizontalAlignment: Text.AlignLeft
            //         verticalAlignment: Text.AlignVCenter
            //     }
            // }
        }


        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.horizontalStretchFactor: 1

            Button {
                id: me
                background: Rectangle {
                    color: enabled ? (me.hovered ? "#404143" : "transparent") : "transparent"
                    radius: height / 2

                }
                anchors.topMargin: 24
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.leftMargin: 8

                width: 40
                height: 40

                hoverEnabled: true

                contentItem: Text {
                    text: "\ue5cd"
                    font.family: Constants.symbolFontFamily
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 26
                    color: "#c3c3c3"
                }

                checkable: false

                onClicked: {
                    root.StackView.view.pop()
                }
                // Layout.fillHeight: true
                // Layout.fillWidth: true
            }
        }
    }

    Component {
        id: librarySettings
        LibrarySettings {
        }
    }

    Component {
        id: videoSettings
        VideoSettings {
        }
    }

    Component {
        id: retroAchievementSettings
        RetroAchievementSettings {
        }
    }

    Component {
        id: debugSettings
        DebugPage {
        }

    }
}

// Item {
//     id: root
//
//     Pane {
//         id: header
//         anchors.top: parent.top
//         anchors.left: parent.left
//         anchors.right: parent.right
//         background: Item {
//         }
//
//         Column {
//             anchors.fill: parent
//             spacing: 8
//
//             Item {
//                 width: parent.width
//                 height: 24
//             }
//
//             Text {
//                 text: "Settings"
//                 color: "#dadada"
//                 font.pointSize: 24
//                 font.family: Constants.semiboldFontFamily
//                 horizontalAlignment: Text.AlignLeft
//                 verticalAlignment: Text.AlignVCenter
//             }
//
//             Rectangle {
//                 width: parent.width
//                 height: 1
//                 opacity: 0.3
//                 color: "#dadada"
//             }
//         }
//     }
//
//     Item {
//         id: leftHalf
//         anchors.left: parent.left
//         anchors.top: header.bottom
//         anchors.bottom: parent.bottom
//         width: parent.width / 3
//
//         ColumnLayout {
//             id: menu
//             anchors.fill: parent
//             spacing: 4
//
//             FirelightMenuItem {
//                 labelText: "Appearance"
//                 // labelIcon: "\ue40a"
//                 Layout.fillWidth: true
//                 Layout.preferredHeight: 40
//                 Layout.alignment: Qt.AlignLeft | Qt.AlignTop
//                 enabled: false
//             }
//             FirelightMenuItem {
//                 labelText: "Video"
//                 // labelIcon: "\ue333"
//                 Layout.fillWidth: true
//                 Layout.preferredHeight: 40
//                 Layout.alignment: Qt.AlignLeft | Qt.AlignTop
//                 checked: true
//                 onToggled: {
//                     if (checked) {
//                         rightHalf.replace(videoSettings)
//                     }
//                 }
//             }
//             FirelightMenuItem {
//                 labelText: "Sound"
//                 // labelIcon: "\ue050"
//                 Layout.fillWidth: true
//                 Layout.alignment: Qt.AlignLeft | Qt.AlignTop
//                 Layout.preferredHeight: 40
//                 enabled: false
//             }
//             FirelightMenuItem {
//                 labelText: "System"
//                 // labelIcon: "\uf522"
//                 Layout.fillWidth: true
//                 Layout.alignment: Qt.AlignLeft | Qt.AlignTop
//                 Layout.preferredHeight: 40
//                 enabled: false
//             }
//             Rectangle {
//                 Layout.fillWidth: true
//                 Layout.preferredHeight: 1
//                 Layout.alignment: Qt.AlignLeft | Qt.AlignTop
//                 opacity: 0.3
//                 color: "#dadada"
//             }
//             FirelightMenuItem {
//                 labelText: "RetroAchievements"
//                 // labelIcon: "\ue897"
//                 Layout.fillWidth: true
//                 Layout.alignment: Qt.AlignLeft | Qt.AlignTop
//                 Layout.preferredHeight: 40
//                 onToggled: {
//                     if (checked) {
//                         rightHalf.replace(retroAchievementSettings)
//                     }
//                 }
//             }
//             FirelightMenuItem {
//                 labelText: "Privacy"
//                 // labelIcon: "\ue897"
//                 Layout.fillWidth: true
//                 Layout.alignment: Qt.AlignLeft | Qt.AlignTop
//                 Layout.preferredHeight: 40
//                 enabled: false
//             }
//             FirelightMenuItem {
//                 labelText: "About"
//                 // labelIcon: "\ue88e"
//                 Layout.fillWidth: true
//                 Layout.alignment: Qt.AlignLeft | Qt.AlignTop
//                 Layout.preferredHeight: 40
//                 enabled: false
//             }
//
//             Item {
//                 Layout.fillWidth: true
//                 Layout.fillHeight: true
//             }
//         }
//     }
//
//     StackView {
//         id: rightHalf
//         anchors.right: parent.right
//         anchors.top: header.bottom
//         anchors.bottom: parent.bottom
//         anchors.left: leftHalf.right
//
//         initialItem: videoSettings
//
//         pushEnter: Transition {
//         }
//
//         pushExit: Transition {
//         }
//
//         popEnter: Transition {
//         }
//
//         popExit: Transition {
//         }
//
//         replaceEnter: Transition {
//         }
//
//         replaceExit: Transition {
//         }
//     }
//
//     Component {
//         id: videoSettings
//         VideoSettings {
//         }
//     }
//
//     Component {
//         id: retroAchievementSettings
//         RetroAchievementSettings {
//         }
//     }
//
//
// }