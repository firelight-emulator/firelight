import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

FocusScope {
    id: root
    Flickable {
        anchors.fill: parent
        contentHeight: column.height
        ColumnLayout {
            id: column
            width: parent.width

            RowLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: 40
                spacing: 8

                Button {
                    id: backButton
                    background: Rectangle {
                        color: enabled ? (backButton.hovered ? "#404143" : "transparent") : "transparent"
                        radius: height / 2

                    }

                    Layout.fillHeight: true
                    Layout.preferredWidth: parent.height

                    hoverEnabled: true

                    contentItem: Icon {
                        name: "arrow_back_ios"
                        leftPadding: 8
                        size: 24
                        color: Theme.textMuted
                    }

                    checkable: false

                    onClicked: {
                        root.StackView.view.pop()
                    }
                }

                Text {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    text: qsTr("Default Game Boy Advance settings")
                    font.pixelSize: AppStyle.fontSizeLarge
                    font.family: Constants.regularFontFamily
                    font.weight: Font.Bold
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    color: Theme.textPrimary
                }
                Layout.bottomMargin: 8
            }

            Rectangle {
                Layout.topMargin: 8
                Layout.bottomMargin: 8
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: Theme.border
            }

            OptionGroup {
                Layout.fillWidth: true
                label: "General"

                content: [

                    ToggleOption {
                        Layout.fillWidth: true
                        label: "Filter out overly harsh audio"
                        // description: "Enables a low pass audio filter to reduce the 'harshness' of generated audio."

                        checked: emulator_config_manager.getOptionValueForPlatform(3, "mgba_audio_low_pass_filter") === "enabled"

                        onCheckedChanged: {
                            if (checked) {
                                emulator_config_manager.setOptionValueForPlatform(3, "mgba_audio_low_pass_filter", "enabled")
                            } else {
                                emulator_config_manager.setOptionValueForPlatform(3, "mgba_audio_low_pass_filter", "disabled")
                            }
                        }
                    },

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                        color: "#333333"
                    },

                    ToggleOption {
                        Layout.fillWidth: true
                        label: "Adjust output colors to match original hardware"
                        // description: "Adjusts output colors to match the display of real GBA hardware."

                        checked: emulator_config_manager.getOptionValueForPlatform(3, "mgba_color_correction") === "Auto"

                        onCheckedChanged: {
                            if (checked) {
                                emulator_config_manager.setOptionValueForPlatform(3, "mgba_color_correction", "Auto")
                            } else {
                                emulator_config_manager.setOptionValueForPlatform(3, "mgba_color_correction", "OFF")
                            }
                        }
                    },

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                        color: "#333333"
                    },

                    ToggleOption {
                        Layout.fillWidth: true
                        label: "Simulate LCD ghosting effects"
                        // description: "Simulates LCD ghosting effects. 'Simple' performs a 50:50 mix of the current and previous frames. 'Smart' attempts to detect screen flickering, and only performs a 50:50 mix on affected pixels. 'LCD Ghosting' mimics natural LCD response times by combining multiple buffered frames. 'Simple' or 'Smart' blending is required when playing games that aggressively exploit LCD ghosting for transparency effects (Wave Race, Chikyuu Kaihou Gun ZAS, F-Zero, the Boktai series...)."

                        checked: emulator_config_manager.getOptionValueForPlatform(3, "mgba_interframe_blending") === "mix_smart"

                        onCheckedChanged: {
                            if (checked) {
                                emulator_config_manager.setOptionValueForPlatform(3, "mgba_interframe_blending", "mix_smart")
                            } else {
                                emulator_config_manager.setOptionValueForPlatform(3, "mgba_interframe_blending", "OFF")
                            }
                        }
                    },

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                        color: "#333333"
                    },

                    Option {
                        Layout.fillWidth: true
                        label: "Solar sensor level"
                        // description: "Sets ambient sunlight intensity. Can be used by games that included a solar sensor in their cartridges, e.g: the Boktai series."
                    },

                    MySlider {
                        Layout.fillWidth: true
                        // Layout.preferredHeight: 48
                        from: 0
                        to: 100
                        stepSize: 10
                        snapMode: Slider.SnapOnRelease
                        live: false

                        value: emulator_config_manager.getOptionValueForPlatform(3, "mgba_solar_sensor_level") * 10

                        onValueChanged: function () {
                            emulator_config_manager.setOptionValueForPlatform(3, "mgba_solar_sensor_level", value / 10)
                        }
                    }

                ]
            }
        }
    }
}
