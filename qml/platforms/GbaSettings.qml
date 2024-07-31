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

                    contentItem: Text {
                        text: "\ue5e0"
                        font.family: Constants.symbolFontFamily
                        leftPadding: 8
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: 24
                        color: ColorPalette.neutral400
                    }

                    checkable: false

                    onClicked: {
                        root.StackView.view.pop()
                    }
                }

                Text {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    text: qsTr("Default Game Boy Advance Settings")
                    font.pixelSize: 26
                    font.family: Constants.regularFontFamily
                    font.weight: Font.Bold
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    color: ColorPalette.neutral100
                }
                Layout.bottomMargin: 8
            }

            Rectangle {
                Layout.topMargin: 8
                Layout.bottomMargin: 8
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: "#333333"
            }

            OptionGroup {
                Layout.fillWidth: true
                label: "General"

                content: [
                    // ToggleOption {
                    //     Layout.fillWidth: true
                    //     label: "Allow opposing D-Pad directions"
                    //     description: "Enabling this will allow pressing / quickly alternating / holding both left and right (or up and down) directions at the same time. This may cause movement-based glitches."
                    // },
                    //
                    // Rectangle {
                    //     Layout.fillWidth: true
                    //     Layout.preferredHeight: 1
                    //     color: "#333333"
                    // },

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

                    // ToggleOption {
                    //     Layout.fillWidth: true
                    //     label: "Game Boy Player Rumble (Restart)"
                    //     description: "Enabling this will allow compatible games with the Game Boy Player boot logo to make the controller rumble. Due to how Nintendo decided this feature should work, it may cause glitches such as flickering or lag in some of these games."
                    // },
                    //
                    // Rectangle {
                    //     Layout.fillWidth: true
                    //     Layout.preferredHeight: 1
                    //     color: "#333333"
                    // },

                    // ToggleOption {
                    //     Layout.fillWidth: true
                    //     label: "Idle Loop Optimization"
                    //     description: "Reduce system load by optimizing so-called 'idle-loops' - sections in the code where nothing happens, but the CPU runs at full speed (like a car revving in neutral). Improves performance, and should be enabled on low-end hardware."
                    // },
                    //
                    // Rectangle {
                    //     Layout.fillWidth: true
                    //     Layout.preferredHeight: 1
                    //     color: "#333333"
                    // },

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

                    // Rectangle {
                    //     Layout.fillWidth: true
                    //     Layout.preferredHeight: 1
                    //     color: "#333333"
                    // },
                    //
                    // ToggleOption {
                    //     Layout.fillWidth: true
                    //     label: "Use BIOS"
                    //     description: "Use official BIOS/bootloader for emulated hardware, if present in RetroArch's system directory."
                    // }
                ]
            }
        }
    }
}