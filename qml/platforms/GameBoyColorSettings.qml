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
                    text: qsTr("Default Game Boy Color settings")
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
                Layout.topMargin: 30
                Layout.fillWidth: true
                label: "Display"

                content: [
                    ToggleOption {
                        Layout.fillWidth: true
                        label: "Simulate LCD ghosting effects"
                        // description: "Enables simulation of LCD ghosting effects by blending the current and previous frames."
                        checked: emulator_config_manager.getOptionValueForPlatform(2, "gambatte_mix_frames") === "accurate"

                        // Component.onCompleted: {
                        //     checked = emulator_config_manager.getOptionValueForPlatform(1, "gambatte_mix_frames") === "accurate"
                        // }

                        onCheckedChanged: {
                            if (checked) {
                                emulator_config_manager.setOptionValueForPlatform(2, "gambatte_mix_frames", "accurate")
                            } else {
                                emulator_config_manager.setOptionValueForPlatform(2, "gambatte_mix_frames", "disabled")
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
                        label: "Adjust output colors to match original hardware"

                        checked: emulator_config_manager.getOptionValueForPlatform(2, "gambatte_gbc_color_correction") === "GBC only"

                        onCheckedChanged: function () {
                            if (checked) {
                                emulator_config_manager.setOptionValueForPlatform(2, "gambatte_gbc_color_correction", "GBC only")
                            } else {
                                emulator_config_manager.setOptionValueForPlatform(2, "gambatte_gbc_color_correction", "disabled")
                            }
                        }
                    },

                    Option {
                        Layout.fillWidth: true
                        // Layout.leftMargin: 36
                        isSubItem: true
                        visible: colorCorrectionOption.checked
                        label: "Frontlight position"
                        // description: "Simulates the physical response of the Game Boy Color LCD panel when illuminated from different angles."
                        control: MyComboBox {
                            id: frontlightPositionComboBox
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
                                const val = frontlightPositionComboBox.indexOfValue(emulator_config_manager.getOptionValueForPlatform(2, "gambatte_gbc_frontlight_position"))
                                console.log("VALUE: " + val)
                                frontlightPositionComboBox.currentIndex = val
                            }

                            onActivated: function (index) {
                                if (!currentValue) {
                                    return
                                }
                                emulator_config_manager.setOptionValueForPlatform(2, "gambatte_gbc_frontlight_position", currentValue)
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

                        value: emulator_config_manager.getOptionValueForPlatform(2, "gambatte_dark_filter_level")

                        onValueChanged: function () {
                            emulator_config_manager.setOptionValueForPlatform(2, "gambatte_dark_filter_level", value)
                        }
                    }

                ]
            }

        }
    }
}
