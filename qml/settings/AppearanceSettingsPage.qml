import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.platform

FocusScope {
    implicitWidth: col.width
    implicitHeight: col.height
    ColumnLayout {
        id: col
        spacing: 4
        anchors.fill: parent

        // SettingsSectionHeader {
        //     sectionName: "Colors"
        //     showTopPadding: false
        // }
        //
        // ToggleOption {
        //     id: cursorColor
        //     Layout.fillWidth: true
        //     focus: true
        //     label: "Cursor color"
        //
        //     KeyNavigation.down: customBackgroundToggle
        //
        //     background: Rectangle {
        //         color: ColorPalette.neutral300
        //         radius: 8
        //         border.color: ColorPalette.neutral500
        //         opacity: parent.hovered || (!InputMethodManager.usingMouse && cursorColor.activeFocus) ? 0.2 : 0.1
        //
        //         Behavior on opacity {
        //             NumberAnimation {
        //                 duration: 100
        //                 easing.type: Easing.InOutQuad
        //             }
        //         }
        //     }
        // }

        SettingsSectionHeader {
            sectionName: "Background"
            showTopPadding: false
        }

        ToggleOption {
            id: customBackgroundToggle
            Layout.fillWidth: true
            focus: true
            label: "Custom background"

            background: Rectangle {
                color: ColorPalette.neutral300
                radius: 8
                border.color: ColorPalette.neutral500
                opacity: parent.hovered || (!InputMethodManager.usingMouse && customBackgroundToggle.activeFocus) ? 0.2 : 0.1

                Behavior on opacity {
                    NumberAnimation {
                        duration: 100
                        easing.type: Easing.InOutQuad
                    }
                }
            }

            checked: AppearanceSettings.usingCustomBackground

            onCheckedChanged: {
                AppearanceSettings.usingCustomBackground = checked
            }
        }

        FileOption {
            id: customBackgroundFile
            KeyNavigation.up: customBackgroundToggle
            visible: customBackgroundToggle.checked

            background: Rectangle {
                color: ColorPalette.neutral300
                radius: 8
                border.color: ColorPalette.neutral500
                opacity: parent.hovered || (!InputMethodManager.usingMouse && customBackgroundFile.activeFocus) ? 0.2 : 0.1

                Behavior on opacity {
                    NumberAnimation {
                        duration: 100
                        easing.type: Easing.InOutQuad
                    }
                }
            }

            Layout.fillWidth: true
            label: "Background file"
            value: AppearanceSettings.backgroundFile

            onValueChanged: function () {
                AppearanceSettings.backgroundFile = value
            }
        }

        Text {
            font.pixelSize: 15
            visible: customBackgroundToggle.checked
            font.family: Constants.regularFontFamily
            font.weight: Font.Medium
            Layout.fillWidth: true
            color: ColorPalette.neutral100
            text: "Choose an image or gif (pronounced jif) to use as your background."
            leftPadding: 12
            lineHeight: 1.2
            wrapMode: Text.WordWrap
            Layout.bottomMargin: 20
        }

        // ToggleOption {
        //     id: dimming
        //     Layout.fillWidth: true
        //     focus: true
        //     label: "Dimming"
        //
        //     background: Rectangle {
        //         color: ColorPalette.neutral300
        //         radius: 8
        //         border.color: ColorPalette.neutral500
        //         opacity: parent.hovered || (!InputMethodManager.usingMouse && dimming.activeFocus) ? 0.2 : 0.1
        //
        //         Behavior on opacity {
        //             NumberAnimation {
        //                 duration: 100
        //                 easing.type: Easing.InOutQuad
        //             }
        //         }
        //     }
        // }
        //
        // Text {
        //     font.pixelSize: 15
        //     visible: customBackgroundToggle.checked
        //     font.family: Constants.regularFontFamily
        //     font.weight: Font.Medium
        //     Layout.fillWidth: true
        //     color: ColorPalette.neutral100
        //     text: "Lower values may make some UI elements more difficult to see."
        //     leftPadding: 12
        //     lineHeight: 1.2
        //     wrapMode: Text.WordWrap
        //     Layout.bottomMargin: 20
        // }
        //
        // ToggleOption {
        //     id: vignetting
        //     Layout.fillWidth: true
        //     focus: true
        //     label: "Vignette"
        //
        //     background: Rectangle {
        //         color: ColorPalette.neutral300
        //         radius: 8
        //         border.color: ColorPalette.neutral500
        //         opacity: parent.hovered || (!InputMethodManager.usingMouse && vignetting.activeFocus) ? 0.2 : 0.1
        //
        //         Behavior on opacity {
        //             NumberAnimation {
        //                 duration: 100
        //                 easing.type: Easing.InOutQuad
        //             }
        //         }
        //     }
        // }
        //
        // Text {
        //     font.pixelSize: 15
        //     visible: customBackgroundToggle.checked
        //     font.family: Constants.regularFontFamily
        //     font.weight: Font.Medium
        //     Layout.fillWidth: true
        //     color: ColorPalette.neutral100
        //     text: "Lower values may make some UI elements more difficult to see."
        //     leftPadding: 12
        //     lineHeight: 1.2
        //     wrapMode: Text.WordWrap
        //     Layout.bottomMargin: 20
        // }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

        }
    }
}