import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

FocusScope {
    id: root

    required property string section

    Component.onCompleted: {
        sectionChanged()
    }

    Pane {
        id: headerBar
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.left: parent.left
        height: 72
        padding: 16

        background: Item {
        }

        contentItem: RowLayout {
            spacing: 24

            Text {
                text: "Settings"
                color: ColorPalette.neutral100
                font.pixelSize: 24
                font.weight: Font.Normal
                font.family: Constants.regularFontFamily
                Layout.fillHeight: true
                Layout.leftMargin: 24
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }


            Item {
                Layout.fillHeight: true
                Layout.fillWidth: true
            }

            FirelightButton {
                tooltipLabel: "Close"
                flat: true

                Layout.fillHeight: true
                Layout.preferredWidth: height
                Layout.rightMargin: 24

                iconCode: "\ue5cd"

                onClicked: {
                    root.StackView.view.pop()
                }
            }
        }
    }

    RowLayout {
        anchors.top: headerBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: 40
        anchors.rightMargin: 40

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.horizontalStretchFactor: 1
        }

        ColumnLayout {
            id: menu
            spacing: 4
            Layout.maximumWidth: 300
            Layout.preferredWidth: 300
            Layout.minimumWidth: 200
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignRight

            // FirelightMenuItem {
            //     labelText: "Appearance"
            //     // labelIcon: "\ue40a"
            //     Layout.fillWidth: true
            //     Layout.preferredHeight: 40
            //     Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            //     enabled: false
            // }
            FirelightMenuItem {
                id: libraryButton
                labelText: "Library"
                property bool showGlobalCursor: true
                KeyNavigation.down: notificationButton
                focus: true
                // labelIcon: "\ue40a"
                Layout.fillWidth: true
                Layout.preferredHeight: 50
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                checked: root.section === "library"
                onToggled: {
                    if (checked) {
                        root.section = "library"
                    }
                }
            }

            FirelightMenuItem {
                id: notificationButton
                labelText: "Notifications"
                property bool showGlobalCursor: true
                KeyNavigation.down: soundButton
                // labelIcon: "\ue333"
                Layout.fillWidth: true
                Layout.preferredHeight: 50
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                checked: root.section === "notifications"
                onToggled: {
                    if (checked) {
                        root.section = "notifications"
                    }
                }
            }

            FirelightMenuItem {
                id: soundButton
                labelText: "Sound"
                property bool showGlobalCursor: true
                KeyNavigation.down: achievementsButton
                // labelIcon: "\ue333"
                Layout.fillWidth: true
                Layout.preferredHeight: 50
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                checked: root.section === "sound"
                onToggled: {
                    if (checked) {
                        root.section = "sound"
                    }
                }
            }
            FirelightMenuItem {
                id: achievementsButton
                labelText: "Achievements"
                property bool showGlobalCursor: true
                KeyNavigation.down: avButton
                // labelIcon: "\ue897"
                Layout.fillWidth: true
                Layout.preferredHeight: 50
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                checked: root.section === "achievements"
                onToggled: {
                    if (checked) {
                        root.section = "achievements"
                    }
                }
            }

            FirelightMenuItem {
                id: avButton
                labelText: "Audio / Video"
                property bool showGlobalCursor: true
                KeyNavigation.down: platformsButton
                // labelIcon: "\ue333"
                Layout.fillWidth: true
                Layout.preferredHeight: 50
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                checked: root.section === "audiovideo"
                onToggled: {
                    if (checked) {
                        root.section = "audiovideo"
                    }
                }
            }
            FirelightMenuItem {
                id: platformsButton
                labelText: "Platforms"
                property bool showGlobalCursor: true
                // labelIcon: "\ue88e"
                Layout.fillWidth: true
                Layout.preferredHeight: 50
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                checked: root.section === "platforms"
                onToggled: {
                    if (checked) {
                        root.section = "platforms"
                    }
                }
            }
            // FirelightMenuItem {
            //     labelText: "System"
            //     // labelIcon: "\uf522"
            //     Layout.fillWidth: true
            //     Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            //     Layout.preferredHeight: 40
            //     enabled: false
            // }
            // Rectangle {
            //     Layout.fillWidth: true
            //     Layout.preferredHeight: 1
            //     Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            //     opacity: 0.3
            //     color: "#dadada"
            // }
            // FirelightMenuItem {
            //     labelText: "Privacy"
            //     // labelIcon: "\ue897"
            //     Layout.fillWidth: true
            //     Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            //     Layout.preferredHeight: 40
            //     enabled: false
            // }
            // FirelightMenuItem {
            //     labelText: "About"
            //     // labelIcon: "\ue88e"
            //     Layout.fillWidth: true
            //     Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            //     Layout.preferredHeight: 40
            //     enabled: false
            // }
            // FirelightMenuItem {
            //     labelText: "Debug"
            //     // labelIcon: "\ue88e"
            //     Layout.fillWidth: true
            //     Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            //     Layout.preferredHeight: 40
            //     onToggled: {
            //         if (checked) {
            //             rightHalf.replace(debugSettings)
            //         }
            //     }
            // }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }

        StackView {
            id: rightHalf

            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.maximumWidth: 1000
            Layout.preferredWidth: 1000
            Layout.minimumWidth: 500
            Layout.alignment: Qt.AlignLeft
            Layout.leftMargin: 12

            Connections {
                target: root

                function onSectionChanged() {
                    if (root.section === "library") {
                        rightHalf.replace(librarySettings)
                    } else if (root.section === "notifications") {
                        rightHalf.replace(notificationSettings)
                    } else if (root.section === "achievements") {
                        rightHalf.replace(retroAchievementSettings)
                    } else if (root.section === "sound") {
                        rightHalf.replace(soundSettings)
                    } else if (root.section === "audiovideo") {
                        rightHalf.replace(videoSettings)
                    } else if (root.section === "platforms/gbc") {
                        rightHalf.replace(gbcSettings)
                    } else if (root.section === "platforms/snes") {
                        rightHalf.replace(snesSettings)
                    } else if (root.section === "platforms") {
                        rightHalf.replace(platformSettings)
                    } else if (root.section === "platforms/gba") {
                        rightHalf.replace(gbaSettings)
                    }
                }
            }

            // initialItem: librarySettings

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

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.horizontalStretchFactor: 1
        }
    }


    // Rectangle {
    //     color: "#101114"
    //     height: parent.height
    //     width: leftSpacer.width + menu.width + contentPane.horizontalPadding + 6
    //
    //
    // }

    Component {
        id: platformSettings
        PlatformSettingsPage {
        }
    }

    Component {
        id: soundSettings
        SoundSettings {
        }
    }


    Component {
        id: librarySettings
        LibrarySettings {
        }
    }

    Component {
        id: notificationSettings
        NotificationSettings {
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
        id: gbcSettings
        GameBoyColorSettings {
        }
    }

    Component {
        id: gbaSettings
        GbaSettings {
        }
    }

    Component {
        id: snesSettings
        SnesSettings {
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