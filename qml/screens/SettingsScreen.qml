import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

FocusScope {
    id: root

    required property string section
    property bool movingDown: true

    Component.onCompleted: {
        sectionChanged()
    }

    Keys.onEscapePressed: {
        root.StackView.view.pop()
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
                id: closeButton
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

        ListView {
            id: settingsCategoryList
            spacing: 0
            Layout.maximumWidth: 300
            Layout.preferredWidth: 300
            Layout.minimumWidth: 200
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignRight
            focus: true
            keyNavigationEnabled: true

            KeyNavigation.right: rightHalf
            KeyNavigation.up: closeButton

            interactive: false

            property int lastIndex: 0
            currentIndex: 0

            onCurrentIndexChanged: function () {
                root.movingDown = currentIndex > lastIndex;
                lastIndex = currentIndex
                root.section = model.get(currentIndex).section
            }

            model: ListModel {

                ListElement {
                    name: "Directories"
                    section: "directories"
                }
                // ListElement {
                //     name: "Notifications"
                //     section: "notifications"
                // }
                // ListElement {
                //     name: "Sound"
                //     section: "sound"
                // }
                ListElement {
                    name: "Controllers"
                    section: "controllers"
                }
                ListElement {
                    name: "Achievements"
                    section: "achievements"
                }
                ListElement {
                    name: "Audio / Video"
                    section: "audiovideo"
                }
                ListElement {
                    name: "Platforms"
                    section: "platforms"
                }
                // ListElement {
                //     name: "Game Boy"
                //     section: "platforms/gb"
                // }
                // ListElement {
                //     name: "Game Boy Color"
                //     section: "platforms/gbc"
                // }
                // ListElement {
                //     name: "Game Boy Advance"
                //     section: "platforms/gba"
                // }
                // ListElement {
                //     name: "NES"
                //     section: "platforms/nes"
                // }
                // ListElement {
                //     name: "SNES"
                //     section: "platforms/snes"
                // }
                // ListElement {
                //     name: "Nintendo 64"
                //     section: "platforms/n64"
                // }
                // ListElement {
                //     name: "Nintendo DS"
                //     section: "platforms/nds"
                // }
                // ListElement {
                //     name: "Master System"
                //     section: "platforms/mastersystem"
                // }
                // ListElement {
                //     name: "Genesis"
                //     section: "platforms/genesis"
                // }
                // ListElement {
                //     name: "Game Gear"
                //     section: "platforms/gamegear"
                // }
            }

            delegate: FirelightMenuItem {
                required property var model
                required property var index

                labelText: model.name
                property bool showGlobalCursor: true
                // labelIcon: "\ue40a"
                height: 50
                width: ListView.view.width
                checked: ListView.isCurrentItem

                onClicked: function () {
                    rightHalf.forceActiveFocus()
                }

                onToggled: {
                    if (checked) {
                        ListView.view.currentIndex = index
                    }
                }
            }
        }

        // ColumnLayout {
        //     id: menu
        //     spacing: 4
        //     Layout.maximumWidth: 300
        //     Layout.preferredWidth: 300
        //     Layout.minimumWidth: 200
        //     Layout.fillHeight: true
        //     Layout.alignment: Qt.AlignRight
        //
        //     // FirelightMenuItem {
        //     //     labelText: "Appearance"
        //     //     // labelIcon: "\ue40a"
        //     //     Layout.fillWidth: true
        //     //     Layout.preferredHeight: 40
        //     //     Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        //     //     enabled: false
        //     // }
        //     FirelightMenuItem {
        //         id: libraryButton
        //         labelText: "Library"
        //         property bool showGlobalCursor: true
        //         KeyNavigation.down: notificationButton
        //         focus: true
        //         // labelIcon: "\ue40a"
        //         Layout.fillWidth: true
        //         Layout.preferredHeight: 50
        //         Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        //         checked: root.section === "library"
        //         onToggled: {
        //             if (checked) {
        //                 root.section = "library"
        //             }
        //         }
        //     }
        //
        //     FirelightMenuItem {
        //         id: notificationButton
        //         labelText: "Notifications"
        //         property bool showGlobalCursor: true
        //         KeyNavigation.down: soundButton
        //         // labelIcon: "\ue333"
        //         Layout.fillWidth: true
        //         Layout.preferredHeight: 50
        //         Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        //         checked: root.section === "notifications"
        //         onToggled: {
        //             if (checked) {
        //                 root.section = "notifications"
        //             }
        //         }
        //     }
        //
        //     FirelightMenuItem {
        //         id: soundButton
        //         labelText: "Sound"
        //         property bool showGlobalCursor: true
        //         KeyNavigation.down: achievementsButton
        //         // labelIcon: "\ue333"
        //         Layout.fillWidth: true
        //         Layout.preferredHeight: 50
        //         Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        //         checked: root.section === "sound"
        //         onToggled: {
        //             if (checked) {
        //                 root.section = "sound"
        //             }
        //         }
        //     }
        //     FirelightMenuItem {
        //         id: achievementsButton
        //         labelText: "Achievements"
        //         property bool showGlobalCursor: true
        //         KeyNavigation.down: avButton
        //         // labelIcon: "\ue897"
        //         Layout.fillWidth: true
        //         Layout.preferredHeight: 50
        //         Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        //         checked: root.section === "achievements"
        //         onToggled: {
        //             if (checked) {
        //                 root.section = "achievements"
        //             }
        //         }
        //     }
        //
        //     FirelightMenuItem {
        //         id: avButton
        //         labelText: "Audio / Video"
        //         property bool showGlobalCursor: true
        //         KeyNavigation.down: platformsButton
        //         // labelIcon: "\ue333"
        //         Layout.fillWidth: true
        //         Layout.preferredHeight: 50
        //         Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        //         checked: root.section === "audiovideo"
        //         onToggled: {
        //             if (checked) {
        //                 root.section = "audiovideo"
        //             }
        //         }
        //     }
        //     FirelightMenuItem {
        //         id: platformsButton
        //         labelText: "Platforms"
        //         property bool showGlobalCursor: true
        //         // labelIcon: "\ue88e"
        //         Layout.fillWidth: true
        //         Layout.preferredHeight: 50
        //         Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        //         checked: root.section === "platforms"
        //         onToggled: {
        //             if (checked) {
        //                 root.section = "platforms"
        //             }
        //         }
        //     }
        //     // FirelightMenuItem {
        //     //     labelText: "System"
        //     //     // labelIcon: "\uf522"
        //     //     Layout.fillWidth: true
        //     //     Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        //     //     Layout.preferredHeight: 40
        //     //     enabled: false
        //     // }
        //     // Rectangle {
        //     //     Layout.fillWidth: true
        //     //     Layout.preferredHeight: 1
        //     //     Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        //     //     opacity: 0.3
        //     //     color: "#dadada"
        //     // }
        //     // FirelightMenuItem {
        //     //     labelText: "Privacy"
        //     //     // labelIcon: "\ue897"
        //     //     Layout.fillWidth: true
        //     //     Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        //     //     Layout.preferredHeight: 40
        //     //     enabled: false
        //     // }
        //     // FirelightMenuItem {
        //     //     labelText: "About"
        //     //     // labelIcon: "\ue88e"
        //     //     Layout.fillWidth: true
        //     //     Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        //     //     Layout.preferredHeight: 40
        //     //     enabled: false
        //     // }
        //     // FirelightMenuItem {
        //     //     labelText: "Debug"
        //     //     // labelIcon: "\ue88e"
        //     //     Layout.fillWidth: true
        //     //     Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        //     //     Layout.preferredHeight: 40
        //     //     onToggled: {
        //     //         if (checked) {
        //     //             rightHalf.replace(debugSettings)
        //     //         }
        //     //     }
        //     // }
        //
        //     Item {
        //         Layout.fillWidth: true
        //         Layout.fillHeight: true
        //     }
        // }

        // SwipeView {
        //
        //     Layout.fillHeight: true
        //     Layout.fillWidth: true
        //     Layout.maximumWidth: 1000
        //     Layout.preferredWidth: 1000
        //     Layout.minimumWidth: 500
        //     Layout.alignment: Qt.AlignLeft
        //     Layout.leftMargin: 12
        //
        //     orientation: Qt.Vertical
        //     interactive: false
        //     currentIndex: settingsCategoryList.currentIndex
        //
        //     LibrarySettings {
        //     }
        //     NotificationSettings {
        //     }
        //     SoundSettings {
        //     }
        //     RetroAchievementSettings {
        //     }
        //     VideoSettings {
        //     }
        //     PlatformSettingsPage {
        //     }
        // }

        StackView {
            id: rightHalf

            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.maximumWidth: 1000
            Layout.preferredWidth: 1000
            Layout.minimumWidth: 500
            Layout.alignment: Qt.AlignLeft
            Layout.leftMargin: 12

            KeyNavigation.up: closeButton

            Keys.onPressed: function (event) {
                if (event.key === Qt.Key_Back || event.key === Qt.Key_Escape) {
                    settingsCategoryList.forceActiveFocus()
                    event.accept = true
                }
            }

            Connections {
                target: root

                function onSectionChanged() {
                    if (root.section === "directories") {
                        rightHalf.replace(directorySettings)
                    } else if (root.section === "notifications") {
                        rightHalf.replace(notificationSettings)
                    } else if (root.section === "achievements") {
                        rightHalf.replace(retroAchievementSettings)
                    } else if (root.section === "sound") {
                        rightHalf.replace(soundSettings)
                    } else if (root.section === "audiovideo") {
                        rightHalf.replace(videoSettings)
                    } else if (root.section === "controllers") {
                        rightHalf.replace(controllerSettings)
                    } else if (root.section === "platforms/gbc") {
                        rightHalf.replace(gbcSettings)
                    } else if (root.section === "platforms/gb") {
                        rightHalf.replace(gbSettings)
                    } else if (root.section === "platforms/snes") {
                        rightHalf.replace(snesSettings)
                    } else if (root.section === "platforms") {
                        rightHalf.replace(platformSettings)
                    } else if (root.section === "platforms/gba") {
                        rightHalf.replace(gbaSettings)
                    } else if (root.section === "platforms/n64") {
                        rightHalf.replace(n64Settings)
                    } else if (root.section === "platforms/nds") {
                        rightHalf.replace(ndsSettings)
                    } else if (root.section === "platforms/nes") {
                        rightHalf.replace(nesSettings)
                    } else if (root.section === "platforms/mastersystem") {
                        rightHalf.replace(masterSystemSettings)
                    } else if (root.section === "platforms/genesis") {
                        rightHalf.replace(genesisSettings)
                    } else if (root.section === "platforms/gamegear") {
                        rightHalf.replace(gameGearSettings)
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
                NumberAnimation {
                    property: "opacity";
                    from: 0.0;
                    to: 1.0
                    duration: 200
                }
                NumberAnimation {
                    property: "y";
                    from: 30 * (root.movingDown ? 1 : -1);
                    to: 0
                    duration: 200
                    easing.type: Easing.InOutQuad
                }
            }

            replaceExit: Transition {
                NumberAnimation {
                    property: "opacity";
                    from: 1.0;
                    to: 0.0
                    duration: 20
                }
                NumberAnimation {
                    property: "y";
                    from: 0;
                    to: 30 * (root.movingDown ? -1 : 1)
                    duration: 200
                    easing.type: Easing.InOutQuad
                }
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
        id: directorySettings
        DirectorySettings {
        }
    }

    Component {
        id: soundSettings
        SoundSettings {
        }
    }

    Component {
        id: controllerSettings
        ControllerSettings {
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
        id: gbSettings
        GameBoySettings {
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
        id: nesSettings
        NesSettings {
        }
    }

    Component {
        id: n64Settings
        Nintendo64Settings {
        }
    }

    Component {
        id: ndsSettings
        NintendoDsSettings {
        }
    }

    Component {
        id: masterSystemSettings
        MasterSystemSettings {
        }
    }

    Component {
        id: genesisSettings
        GenesisSettings {
        }
    }

    Component {
        id: gameGearSettings
        GameGearSettings {
        }
    }

    // Component {
    //     id: saturnSettings
    //     SaturnSettings {
    //     }
    // }
    //
    // Component {
    //     id: dreamcastSettings
    //     DreamcastSettings {
    //     }
    // }


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