import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

// Global emulation defaults (the base tier). Consoles override these per-core,
// and individual games override those. Only shows the common (frontend) settings
// since per-core options don't apply globally.
FocusScope {
    id: root

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        Text {
            Layout.fillWidth: true
            text: qsTr("Emulation")
            font.pixelSize: AppStyle.fontSizeLarge
            font.family: Constants.regularFontFamily
            font.weight: Font.Bold
            color: Theme.textPrimary
        }

        Text {
            Layout.fillWidth: true
            text: qsTr("Global defaults for all games. Consoles and individual games can override these.")
            font.pixelSize: AppStyle.fontSizeMedium
            font.family: Constants.regularFontFamily
            font.weight: Font.Normal
            wrapMode: Text.WordWrap
            color: Theme.textMuted
            Layout.bottomMargin: 8
        }

        ToggleOption {
            id: showAdvancedToggle
            Layout.fillWidth: true
            Layout.bottomMargin: 8
            label: qsTr("Show advanced settings")

            background: Rectangle {
                color: ColorPalette.neutral300
                radius: 8
                border.color: ColorPalette.neutral500
                opacity: parent.hovered || (!InputMethodManager.usingMouse && showAdvancedToggle.activeFocus) ? 0.2 : 0.1

                Behavior on opacity {
                    NumberAnimation {
                        duration: 100
                        easing.type: Easing.InOutQuad
                    }
                }
            }

            checked: GeneralSettings.showAdvancedSettings
            onCheckedChanged: {
                GeneralSettings.showAdvancedSettings = checked
            }
        }

        EmulationSettingsSurface {
            Layout.fillWidth: true
            Layout.fillHeight: true
            level: 2 // Global tier
        }
    }
}
