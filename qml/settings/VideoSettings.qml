import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

FocusScope {
    ColumnLayout {
        spacing: 8
        anchors.fill: parent
        // ComboBoxOption {
        //     Layout.fillWidth: true
        //     Layout.preferredHeight: 60
        //     label: "Window Mode?"
        // }
        ToggleOption {
            id: fullscreenOption
            Layout.fillWidth: true
            label: "Fullscreen"
            focus: true

            checked: GeneralSettings.fullscreen

            onCheckedChanged: {
                GeneralSettings.fullscreen = checked
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: 8
            Layout.preferredHeight: 60
            spacing: 12

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2
                Text {
                    text: "Audio output device"
                    color: Theme.textPrimary
                    font.pixelSize: 16
                    font.family: Constants.regularFontFamily
                    font.weight: Font.DemiBold
                }
                Text {
                    text: "Applies the next time a game starts."
                    color: Theme.textMuted
                    font.pixelSize: 14
                    font.family: Constants.regularFontFamily
                    font.weight: Font.Medium
                }
            }

            ComboBox {
                id: deviceCombo
                Layout.preferredWidth: 280
                model: AudioSettings.availableDevices

                function syncSelection() {
                    currentIndex = Math.max(0, model.indexOf(AudioSettings.outputDevice))
                }

                Component.onCompleted: syncSelection()
                onActivated: AudioSettings.outputDevice = currentText

                Connections {
                    target: AudioSettings
                    function onDevicesChanged() {
                        deviceCombo.syncSelection()
                    }
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

        }
    }
}