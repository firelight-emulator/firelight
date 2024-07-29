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
                    text: qsTr("Default Game Boy Settings")
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
                label: "Colorization"

                content: [
                    Option {
                        id: colorizePalette
                        Layout.fillWidth: true
                        // Layout.leftMargin: 36
                        label: "Colorization mode"
                        property string currentValue
                        // description: "Simulates the physical response of the Game Boy Color LCD panel when illuminated from different angles."
                        control: MyComboBox {
                            model: [{
                                text: "Disabled",
                                value: "disabled"
                            }, {
                                text: "Auto",
                                value: "auto"
                            }, {
                                text: "Specific palette",
                                value: "internal"
                            }]
                            textRole: "text"
                            valueRole: "value"

                            Component.onCompleted: {
                                const value = emulator_config_manager.getOptionValueForPlatform(1, "gambatte_gb_internal_palette")

                                if (value) {
                                    currentIndex = find(value)
                                    colorizePalette.currentValue = value
                                } else {
                                    currentIndex = 0
                                }
                            }

                            onCurrentValueChanged: function () {
                                if (!currentValue) {
                                    return
                                }
                                emulator_config_manager.setOptionValueForPlatform(1, "gambatte_gb_internal_palette", currentValue)
                                colorizePalette.currentValue = currentValue
                            }
                        }
                    },
                    Option {
                        id: colorizationOption
                        Layout.fillWidth: true
                        // Layout.leftMargin: 36
                        isSubItem: true
                        visible: colorizePalette.currentValue === "internal"
                        label: "Color palette"
                        property string currentValue
                        control: MyComboBox {
                            enabled: colorizePalette.currentValue === "internal"
                            model: [{
                                text: "GB - DMG",
                                value: "GB - DMG"
                            }, {
                                text: "GB - Pocket",
                                value: "GB - Pocket"
                            }, {
                                text: "GB - Light",
                                value: "GB - Light"
                            }, {
                                text: "GBC - Blue",
                                value: "GBC - Blue"
                            }, {
                                text: "GBC - Brown",
                                value: "GBC - Brown"
                            }, {
                                text: "GBC - Dark Blue",
                                value: "GBC - Dark Blue"
                            }, {
                                text: "GBC - Dark Brown",
                                value: "GBC - Dark Brown"
                            }, {
                                text: "GBC - Dark Green",
                                value: "GBC - Dark Green"
                            }, {
                                text: "GBC - Grayscale",
                                value: "GBC - Grayscale"
                            }, {
                                text: "GBC - Green",
                                value: "GBC - Green"
                            }, {
                                text: "GBC - Inverted",
                                value: "GBC - Inverted"
                            }, {
                                text: "GBC - Orange",
                                value: "GBC - Orange"
                            }, {
                                text: "GBC - Pastel Mix",
                                value: "GBC - Pastel Mix"
                            }, {
                                text: "GBC - Red",
                                value: "GBC - Red"
                            }, {
                                text: "GBC - Yellow",
                                value: "GBC - Yellow"
                            }, {
                                text: "SGB - 1A",
                                value: "SGB - 1A"
                            }, {
                                text: "SGB - 1B",
                                value: "SGB - 1B"
                            }, {
                                text: "SGB - 1C",
                                value: "SGB - 1C"
                            }, {
                                text: "SGB - 1D",
                                value: "SGB - 1D"
                            }, {
                                text: "SGB - 1E",
                                value: "SGB - 1E"
                            }, {
                                text: "SGB - 1F",
                                value: "SGB - 1F"
                            }, {
                                text: "SGB - 1G",
                                value: "SGB - 1G"
                            }, {
                                text: "SGB - 1H",
                                value: "SGB - 1H"
                            }, {
                                text: "SGB - 2A",
                                value: "SGB - 2A"
                            }, {
                                text: "SGB - 2B",
                                value: "SGB - 2B"
                            }, {
                                text: "SGB - 2C",
                                value: "SGB - 2C"
                            }, {
                                text: "SGB - 2D",
                                value: "SGB - 2D"
                            }, {
                                text: "SGB - 2E",
                                value: "SGB - 2E"
                            }, {
                                text: "SGB - 2F",
                                value: "SGB - 2F"
                            }, {
                                text: "SGB - 2G",
                                value: "SGB - 2G"
                            }, {
                                text: "SGB - 2H",
                                value: "SGB - 2H"
                            }, {
                                text: "SGB - 3A",
                                value: "SGB - 3A"
                            }, {
                                text: "SGB - 3B",
                                value: "SGB - 3B"
                            }, {
                                text: "SGB - 3C",
                                value: "SGB - 3C"
                            }, {
                                text: "SGB - 3D",
                                value: "SGB - 3D"
                            }, {
                                text: "SGB - 3E",
                                value: "SGB - 3E"
                            }, {
                                text: "SGB - 3F",
                                value: "SGB - 3F"
                            }, {
                                text: "SGB - 3G",
                                value: "SGB - 3G"
                            }, {
                                text: "SGB - 3H",
                                value: "SGB - 3H"
                            }, {
                                text: "SGB - 4A",
                                value: "SGB - 4A"
                            }, {
                                text: "SGB - 4B",
                                value: "SGB - 4B"
                            }, {
                                text: "SGB - 4C",
                                value: "SGB - 4C"
                            }, {
                                text: "SGB - 4D",
                                value: "SGB - 4D"
                            }, {
                                text: "SGB - 4E",
                                value: "SGB - 4E"
                            }, {
                                text: "SGB - 4F",
                                value: "SGB - 4F"
                            }, {
                                text: "SGB - 4G",
                                value: "SGB - 4G"
                            }, {
                                text: "SGB - 4H",
                                value: "SGB - 4H"
                            }, {
                                text: "Special 1",
                                value: "Special 1"
                            }, {
                                text: "Special 2",
                                value: "Special 2"
                            }, {
                                text: "Special 3",
                                value: "Special 3"
                            }]
                            textRole: "text"
                            valueRole: "value"

                            Component.onCompleted: {
                                const value = emulator_config_manager.getOptionValueForPlatform(1, "gambatte_gb_internal_palette")

                                if (value) {
                                    currentIndex = find(value)
                                    colorizationOption.currentValue = value
                                } else {
                                    currentIndex = 0
                                }
                            }

                            onCurrentValueChanged: function () {
                                if (!currentValue) {
                                    return
                                }
                                emulator_config_manager.setOptionValueForPlatform(1, "gambatte_gb_internal_palette", currentValue)
                                colorizationOption.currentValue = currentValue
                            }
                        }
                    },
                    Text {
                        Layout.topMargin: 8
                        Layout.fillWidth: true
                        visible: colorizePalette.currentValue === "internal"
                        text: "Example"
                        color: ColorPalette.neutral300
                        horizontalAlignment: Text.AlignHCenter
                        font.pixelSize: 15
                        Layout.alignment: Qt.AlignLeft
                        font.family: Constants.regularFontFamily
                        font.weight: Font.DemiBold
                    },
                    Item {
                        Layout.topMargin: 8
                        Layout.bottomMargin: 16
                        Layout.fillWidth: true
                        Layout.preferredHeight: 400
                        visible: colorizePalette.currentValue === "internal"
                        Image {
                            id: colorPreviewImg
                            anchors.fill: parent
                            source: "file:system/_img/gb-dmg.png"
                            fillMode: Image.PreserveAspectFit

                            Connections {
                                target: colorizationOption

                                function onCurrentValueChanged() {
                                    let filename = colorizationOption.currentValue ? colorizationOption.currentValue.toLowerCase() : ""
                                    let removedSpacesText = filename.replace(
                                        / /g,
                                        ""
                                    );
                                    colorPreviewImg.source = "file:system/_img/" + removedSpacesText + ".png"
                                }
                            }
                        }
                    }
                ]
            }

            OptionGroup {
                Layout.topMargin: 30
                Layout.fillWidth: true
                label: "Display"

                content: [
                    ToggleOption {
                        Layout.fillWidth: true
                        label: "Simulate LCD ghosting"
                        // description: "Enables simulation of LCD ghosting effects by blending the current and previous frames."

                        Component.onCompleted: {
                            checked = emulator_config_manager.getOptionValueForPlatform(1, "gambatte_mix_frames") === "accurate"
                        }

                        onCheckedChanged: {
                            if (checked) {
                                emulator_config_manager.setOptionValueForPlatform(1, "gambatte_mix_frames", "accurate")
                            } else {
                                emulator_config_manager.setOptionValueForPlatform(1, "gambatte_mix_frames", "disabled")
                            }
                        }

                    },

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                        color: "#333333"
                    },
                    ToggleOption {
                        id: colorCorrectionOption
                        Layout.fillWidth: true
                        label: "Make colors more accurate to the original hardware"
                        // label: "Color correction"
                        // description: "Make the screen colors more accurate to the original hardware."

                        Component.onCompleted: {
                            checked = emulator_config_manager.getOptionValueForPlatform(1, "gambatte_gbc_color_correction") === "GBC only"
                        }

                        onCheckedChanged: function () {
                            if (checked) {
                                emulator_config_manager.setOptionValueForPlatform(1, "gambatte_gbc_color_correction", "GBC only")
                            } else {
                                emulator_config_manager.setOptionValueForPlatform(1, "gambatte_gbc_color_correction", "disabled")
                            }
                        }
                    },

                    // Rectangle {
                    //     Layout.fillWidth: true
                    //     Layout.preferredHeight: 1
                    //     color: "#333333"
                    // },

                    Option {
                        Layout.fillWidth: true
                        // Layout.leftMargin: 36
                        isSubItem: true
                        visible: colorCorrectionOption.checked
                        label: "Frontlight position"
                        // description: "Simulates the physical response of the Game Boy Color LCD panel when illuminated from different angles."
                        control: MyComboBox {
                            enabled: colorCorrectionOption.checked
                            model: [{
                                text: "Center of screen",
                                value: "central"
                            }, {
                                text: "Above screen",
                                value: "above screen"
                            }, {
                                text: "Below screen",
                                value: "below screen"
                            }]
                            textRole: "text"
                            valueRole: "value"

                            Component.onCompleted: {
                                currentIndex = find(emulator_config_manager.getOptionValueForPlatform(1, "gambatte_gbc_frontlight_position"))
                            }

                            onCurrentValueChanged: function () {
                                emulator_config_manager.setOptionValueForPlatform(1, "gambatte_gbc_frontlight_position", currentValue)
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
                        label: "Darken screen to reduce harshness"
                    },

                    MySlider {
                        Layout.fillWidth: true
                        // Layout.preferredHeight: 48
                        from: 0
                        to: 50
                        stepSize: 5
                        snapMode: Slider.SnapOnRelease
                        live: false
                    }


                ]
            }

            // OptionGroup {
            //     Layout.topMargin: 30
            //     Layout.fillWidth: true
            //     label: "Darken screen to reduce harshness"
            //
            //     MySlider {
            //         Layout.fillWidth: true
            //         Layout.preferredHeight: 48
            //         from: 0
            //         to: 50
            //         stepSize: 5
            //         snapMode: Slider.SnapAlways
            //     }
            // }


            // Pane {
            //     Layout.fillWidth: true
            //     verticalPadding: stuffCol.spacing
            //     background: Rectangle {
            //         radius: 8
            //         color: ColorPalette.neutral800
            //     }
            //
            //     contentItem: ColumnLayout {
            //         id: stuffCol
            //
            //         ToggleOption {
            //             Layout.fillWidth: true
            //             label: "Simulate LCD ghosting"
            //             // description: "Enables simulation of LCD ghosting effects by blending the current and previous frames."
            //
            //             Component.onCompleted: {
            //                 checked = emulator_config_manager.getOptionValueForPlatform(1, "gambatte_mix_frames") === "accurate"
            //             }
            //
            //             onCheckedChanged: {
            //                 if (checked) {
            //                     emulator_config_manager.setOptionValueForPlatform(1, "gambatte_mix_frames", "accurate")
            //                 } else {
            //                     emulator_config_manager.setOptionValueForPlatform(1, "gambatte_mix_frames", "disabled")
            //                 }
            //             }
            //
            //         }
            //
            //         ToggleOption {
            //             Layout.fillWidth: true
            //             label: "Allow opposing D-Pad directions"
            //             // description: "Enabling this will allow pressing / quickly alternating / holding both left and right (or up and down) directions at the same time. This may cause movement-based glitches."
            //
            //             Component.onCompleted: {
            //                 // checked = emulator_config_manager.getOptionValueForPlatform(1, "snes9x_up_down_allowed") === "enabled"
            //                 emulator_config_manager.getOptionValueForPlatform(1, "gambatte_up_down_allowed")
            //             }
            //
            //             onCheckedChanged: function () {
            //                 if (checked) {
            //                     emulator_config_manager.setOptionValueForPlatform(1, "gambatte_up_down_allowed", "enabled")
            //                 } else {
            //                     emulator_config_manager.setOptionValueForPlatform(1, "gambatte_up_down_allowed", "disabled")
            //                 }
            //             }
            //         }
            //
            //         Rectangle {
            //             Layout.fillWidth: true
            //             Layout.preferredHeight: 1
            //             color: "#333333"
            //         }
            //
            //         ToggleOption {
            //             id: colorCorrectionOption
            //             Layout.fillWidth: true
            //             label: "Correct colors to be more accurate to the original hardware"
            //             // label: "Color correction"
            //             // description: "Make the screen colors more accurate to the original hardware."
            //
            //             Component.onCompleted: {
            //                 checked = emulator_config_manager.getOptionValueForPlatform(1, "gambatte_gbc_color_correction") === "GBC only"
            //             }
            //
            //             onCheckedChanged: function () {
            //                 if (checked) {
            //                     emulator_config_manager.setOptionValueForPlatform(1, "gambatte_gbc_color_correction", "GBC only")
            //                 } else {
            //                     emulator_config_manager.setOptionValueForPlatform(1, "gambatte_gbc_color_correction", "disabled")
            //                 }
            //             }
            //         }
            //
            //         Option {
            //             Layout.fillWidth: true
            //             Layout.leftMargin: 36
            //             enabled: colorCorrectionOption.checked
            //             label: "Frontlight position"
            //             description: "Simulates the physical response of the Game Boy Color LCD panel when illuminated from different angles."
            //             control: MyComboBox {
            //                 enabled: colorCorrectionOption.checked
            //                 model: [{
            //                     text: "Off",
            //                     value: "off"
            //                 }, {
            //                     text: "Top",
            //                     value: "top"
            //                 }]
            //                 textRole: "text"
            //                 valueRole: "value"
            //
            //                 Component.onCompleted: {
            //                     currentIndex = find(emulator_config_manager.getOptionValueForPlatform(1, "gambatte_gbc_color_correction"))
            //                 }
            //
            //                 onCurrentValueChanged: function () {
            //                     emulator_config_manager.setOptionValueForPlatform(1, "gambatte_gbc_color_correction", currentValue)
            //                 }
            //             }
            //         }
            //
            //         Rectangle {
            //             Layout.fillWidth: true
            //             Layout.preferredHeight: 1
            //             color: "#333333"
            //         }
            //
            //         Option {
            //             Layout.fillWidth: true
            //             label: "Darken screen to reduce harshness"
            //             // description: "Darken the screen to reduce glare and/or eye strain. This is useful for games with white backgrounds, as they appear harsher than intended on modern displays."
            //         }
            //
            //         MySlider {
            //             Layout.fillWidth: true
            //             Layout.preferredHeight: 48
            //             from: 0
            //             to: 50
            //             stepSize: 5
            //             snapMode: Slider.SnapAlways
            //         }
            //
            //         Rectangle {
            //             Layout.fillWidth: true
            //             Layout.preferredHeight: 1
            //             color: "#333333"
            //         }
            //     }
            // }
        }
    }
}