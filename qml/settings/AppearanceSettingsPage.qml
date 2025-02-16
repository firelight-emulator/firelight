import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.platform

FocusScope {
    implicitWidth: col.width
    implicitHeight: col.height
    ColumnLayout {
        id: col
        spacing: 8
        anchors.fill: parent

        ToggleOption {
            id: customBackgroundToggle
            Layout.fillWidth: true
            focus: true
            label: "Use custom background"

            checked: AppearanceSettings.usingCustomBackground

            onCheckedChanged: {
                AppearanceSettings.usingCustomBackground = checked
            }
        }

        FileOption {
            id: customBackgroundFile
            KeyNavigation.up: customBackgroundToggle
            visible: customBackgroundToggle.checked
            Layout.fillWidth: true
            leftPadding: 24
            label: "Background file"
            description: "This file will be used as your background. It can be either an image or a gif (pronounced jif)."
            value: AppearanceSettings.backgroundFile

            onValueChanged: function () {
                AppearanceSettings.backgroundFile = value
            }
        }

        Text {
            text: "I'll add other settings here soon to customize the background even more, but I wanted to get something in here quickly so it's pretty basic for now!"
            wrapMode: Text.WordWrap
            padding: 48
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            color: ColorPalette.neutral300
            font.pixelSize: 16
            font.weight: Font.Normal
            font.family: Constants.regularFontFamily
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

        }
    }
}