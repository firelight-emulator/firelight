import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

FocusScope {
    id: root
    required property GameSettings2 gameSettings

    // property var level: GameSettings.Game

    ColumnLayout {
        spacing: 4
        anchors.fill: parent
        // ComboBoxOption {
        //     Layout.fillWidth: true
        //     Layout.preferredHeight: 60
        //     label: "Window Mode?"
        // }

        // RowLayout {
        //     Layout.fillWidth: true
        //     Layout.preferredHeight: 80
        //     spacing: 0
        //
        //     Button {
        //         id: gameButton
        //         text: "Game"
        //         autoExclusive: true
        //         checkable: true
        //         checked: true
        //
        //         onToggled: {
        //             if (checked) {
        //                 console.log("Changing level to game")
        //                 root.level = GameSettings.Game
        //             }
        //         }
        //         Layout.fillWidth: true
        //     }
        //
        //     Button {
        //         text: "Platform"
        //         autoExclusive: true
        //         checkable: true
        //
        //         onToggled: {
        //             if (checked) {
        //                 console.log("Changing level to platform")
        //                 root.level = GameSettings.Platform
        //             }
        //         }
        //         Layout.fillWidth: true
        //     }
        //
        //     Button {
        //         text: "Global"
        //         autoExclusive: true
        //         checkable: true
        //
        //         onToggled: {
        //             if (checked) {
        //                 console.log("Changing level to global")
        //                 root.level = GameSettings.Global
        //             }
        //         }
        //         Layout.fillWidth: true
        //     }
        // }

        SettingsSectionHeader {
            showTopPadding: false
            sectionName: "Video"
        }

        ComboBoxOption2 {
            id: pictureModeOption
            Layout.fillWidth: true
            label: "Picture mode"

            KeyNavigation.down: aspectRatioModeOption

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

            property bool initialized: false

            model: [
                {text: "Aspect Ratio Fill", value: "aspect-ratio-fill"},
                {text: "Integer Scale", value: "integer-scale"},
                {text: "Stretch", value: "stretch"}
            ]

            Component.onCompleted: {
                let val = gameSettings.getGameValue("picture-mode")
                for (let i = 0; i < pictureModeOption.model.length; i++) {
                    if (pictureModeOption.model[i].value === val) {
                        pictureModeOption.currentIndex = i
                        initialized = true;
                        return
                    }
                }

                initialized = true;
            }

            onCurrentValueChanged: {
                if (!initialized) {
                    return
                }
                gameSettings.setGameValue("picture-mode", pictureModeOption.currentValue)
            }
        }

        Text {
            font.pixelSize: 15
            font.family: Constants.regularFontFamily
            font.weight: Font.Medium
            color: ColorPalette.neutral100
            text: "Determines how the game image fills the screen."
            leftPadding: 12
            Layout.bottomMargin: 20
        }

        ComboBoxOption2 {
            id: aspectRatioModeOption
            Layout.fillWidth: true
            label: "Aspect ratio"

            KeyNavigation.down: enableRewindOption

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

            property bool initialized: false

            model: [
                {text: "Pixel-perfect", value: "pixel-perfect"},
                {text: "Emulator-corrected", value: "core-corrected"}
            ]

            Component.onCompleted: {
                let val = gameSettings.getGameValue("aspect-ratio")
                for (let i = 0; i < aspectRatioModeOption.model.length; i++) {
                    if (aspectRatioModeOption.model[i].value === val) {
                        aspectRatioModeOption.currentIndex = i
                        initialized = true;
                        return
                    }
                }

                initialized = true;
            }

            onCurrentValueChanged: {
                if (!initialized) {
                    return
                }
                gameSettings.setGameValue("aspect-ratio", aspectRatioModeOption.currentValue)
            }
        }

        Text {
            font.pixelSize: 15
            font.family: Constants.regularFontFamily
            font.weight: Font.Medium
            color: ColorPalette.neutral100
            text: "Some platforms rendered at a different resolution than the one at which they were displayed on CRTs. This option lets you control which aspect ratio is used.\n\nPixel-perfect: Uses square pixels; ignores the emulator's preferred aspect ratio\nEmulator-corrected: Uses the aspect ratio preferred by the emulator"
            wrapMode: Text.WordWrap

            Layout.fillWidth: true
            leftPadding: 12
            Layout.bottomMargin: 20
        }



        // ComboBoxOption2 {
        //     id: testOption5
        //     Layout.fillWidth: true
        //     label: "Aspect ratio"
        //
        //     KeyNavigation.down: masterVolumeOption
        //
        //     model: [
        //         {text: "Pixel perfect", value: "3"},
        //         {text: "Corrected", value: "5"}
        //     ]
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
        // }
        //
        //     // Text {
        //     //     Layout.preferredWidth: 80
        //     //     visible: gameSettings.valueIsOverridingOther
        //     //     text: "Overriding platform-level setting"
        //     // }
        //
        //
        //
        // RowLayout {
        //     Layout.topMargin: 32
        //     Layout.bottomMargin: 12
        //     spacing: 12
        //     Rectangle {
        //         width: 4
        //         implicitHeight: 24
        //         color: ColorPalette.neutral300
        //     }
        //     Text {
        //         Layout.preferredHeight: 23
        //         font.pixelSize: 16
        //         font.family: Constants.regularFontFamily
        //         font.weight: Font.DemiBold
        //         color: ColorPalette.neutral300
        //         verticalAlignment: Text.AlignVCenter
        //         text: "Audio"
        //     }
        // }
        //
        //
        // ComboBoxOption2 {
        //     id: masterVolumeOption
        //     Layout.fillWidth: true
        //     label: "Master volume"
        //
        //     KeyNavigation.down: enableRewindOption
        //
        //     background: Rectangle {
        //         color: ColorPalette.neutral300
        //         radius: 8
        //         border.color: ColorPalette.neutral500
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
        //     property bool initialized: false
        //
        //     model: [
        //         {text: "Aspect Ratio Fill", value: "aspect-ratio-fill"},
        //         {text: "Integer Scale", value: "integer-scale"},
        //         {text: "Stretch", value: "stretch"}
        //     ]
        //
        //     Component.onCompleted: {
        //         let val = gameSettings.getGameValue("picture-mode")
        //         for (let i = 0; i < pictureModeOption.model.length; i++) {
        //             if (pictureModeOption.model[i].value === val) {
        //                 pictureModeOption.currentIndex = i
        //                 initialized = true;
        //                 return
        //             }
        //         }
        //
        //         initialized = true;
        //     }
        //
        //     onCurrentValueChanged: {
        //         if (!initialized) {
        //             return
        //         }
        //         gameSettings.setValue(level, "picture-mode", pictureModeOption.currentValue)
        //     }
        //
        //     Connections {
        //         target: gameSettings
        //         function onSettingChanged(key, value) {
        //             let val = gameSettings.getGameValue("picture-mode")
        //             for (let i = 0; i < pictureModeOption.model.length; i++) {
        //                 if (pictureModeOption.model[i].value === val) {
        //                     pictureModeOption.currentIndex = i
        //                     return
        //                 }
        //             }
        //         }
        //     }
        //
        //     Connections {
        //         target: root
        //         function onLevelChanged() {
        //             let val = gameSettings.getGameValue("picture-mode")
        //             for (let i = 0; i < pictureModeOption.model.length; i++) {
        //                 if (pictureModeOption.model[i].value === val) {
        //                     pictureModeOption.currentIndex = i
        //                     return
        //                 }
        //             }
        //         }
        //     }
        // }



        SettingsSectionHeader {
            sectionName: "Rewind"
        }

        ToggleOption {
            id: enableRewindOption
            Layout.fillWidth: true
            label: "Rewind"
            // description: "Note: Rewind is always disabled when using RetroAchievements in Hardcore mode."
            property bool initialized: false

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

            checked: gameSettings.getGameValue("rewind-enabled") === "true"

            onCheckedChanged: {
                if (!initialized) {
                    return
                }
                gameSettings.setGameValue("rewind-enabled", checked ? "true" : "false")
            }

            Component.onCompleted: {
                initialized = true
            }
        }
        //
        Text {
            font.pixelSize: 15
            font.family: Constants.regularFontFamily
            font.weight: Font.Medium
            color: ColorPalette.neutral100
            text: "Note: Rewind is always disabled when using RetroAchievements in Hardcore mode."
            leftPadding: 12
            Layout.bottomMargin: 20
        }

        // ComboBoxOption2 {
        //     id: testOption2
        //     Layout.fillWidth: true
        //     label: "Rewind period"
        //
        //     KeyNavigation.down: testOption4
        //
        //     model: [
        //         {text: "Every 3 seconds", value: "3"},
        //         {text: "Every 5 seconds", value: "5"},
        //         {text: "Every 10 seconds", value: "10"},
        //         {text: "Every 20 seconds", value: "20"}
        //     ]
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
        // }
        //
        // Text {
        //     font.pixelSize: 15
        //     font.family: Constants.regularFontFamily
        //     font.weight: Font.Medium
        //     color: ColorPalette.neutral100
        //     text: "Shorter values can increase CPU usage."
        //     leftPadding: 12
        //     Layout.bottomMargin: 20
        // }
        //
        // ComboBoxOption2 {
        //     id: testOption4
        //     Layout.fillWidth: true
        //     label: "Maximum number of rewind points"
        //
        //     KeyNavigation.down: testOption3
        //
        //     model: [
        //         {text: "30", value: "3"},
        //     ]
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
        // }
        //
        // Text {
        //     font.pixelSize: 15
        //     font.family: Constants.regularFontFamily
        //     font.weight: Font.Medium
        //     color: ColorPalette.neutral100
        //     text: "Higher values will increase memory usage."
        //     leftPadding: 12
        //     Layout.bottomMargin: 20
        // }
        //
        // BaseSettingItem {
        //     Layout.fillWidth: true
        //     label: "Testing"
        //     description: "This is a testing option!"
        // }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

        }
    }
}