import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

FocusScope {
    Flickable {
        anchors.fill: parent
        contentHeight: column.height
        ColumnLayout {
            id: column
            width: parent.width

            // Image {
            //     Layout.fillWidth: true
            //     Layout.preferredHeight: parent.width
            //     source: "file:system/_img/snes.svg"
            //     sourceSize.height: height
            //     fillMode: Image.PreserveAspectFit
            // }


            Text {
                Layout.fillWidth: true
                text: qsTr("Default SNES Settings")
                font.pointSize: 16
                font.family: Constants.regularFontFamily
                font.weight: Font.Bold
                Layout.bottomMargin: 8
                color: "white"
            }

            Text {
                Layout.fillWidth: true
                text: qsTr("These are the default settings for SNES games. These settings can be overridden on a per-game basis by selecting the game in your library and going to 'Settings'.")
                font.pointSize: 11
                font.family: Constants.regularFontFamily
                font.weight: Font.Normal
                wrapMode: Text.WordWrap
                Layout.bottomMargin: 8
                color: "white"
            }

            Rectangle {
                Layout.topMargin: 8
                Layout.bottomMargin: 8
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: "#333333"
            }

            Text {
                Layout.fillWidth: true
                text: qsTr("Advanced settings")
                font.pointSize: 11
                font.family: Constants.regularFontFamily
                font.weight: Font.DemiBold
                Layout.bottomMargin: 8
                color: "#a6a6a6"
            }

            Option {
                Layout.fillWidth: true
                Layout.minimumHeight: 42
                label: "Console region"
                description: "Specify which region the system is from. 'PAL' is 50hz, 'NTSC' is 60hz. Games will run faster or slower than normal if the incorrect region is selected."
                control: MyComboBox {
                    model: [{
                        text: "Auto",
                        value: "auto"
                    }, {
                        text: "NTSC",
                        value: "ntsc"
                    }, {
                        text: "PAL",
                        value: "pal"
                    }]
                    textRole: "text"
                    valueRole: "value"
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: "#333333"
            }

            ToggleOption {
                Layout.fillWidth: true
                Layout.minimumHeight: 42
                label: "Allow opposing D-Pad directions"
                description: "Enabling this will allow pressing / quickly alternating / holding both left and right (or up and down) directions at the same time. This may cause movement-based glitches."
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: "#333333"
            }

            ToggleOption {
                Layout.fillWidth: true
                Layout.minimumHeight: 42
                label: "Crop overscan"
                description: "Remove the borders at the top and bottom of the screen, typically unused by games and hidden by the bezel of a standard-definition television. 'Auto' will attempt to detect and crop the ~8 pixel overscan based on the current content."
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: "#333333"
            }

            ToggleOption {
                Layout.fillWidth: true
                Layout.minimumHeight: 42
                label: "Enable graphic clip windows"
                description: "TBD."
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: "#333333"
            }

            ToggleOption {
                Layout.fillWidth: true
                Layout.minimumHeight: 42
                label: "Enable transparency effects"
                description: "TBD."
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: "#333333"
            }

            ToggleOption {
                Layout.fillWidth: true
                Layout.minimumHeight: 42
                label: "Hi-Res blending"
                description: "Blend adjacent pixels when game switches to hi-res mode (512x448). Required for certain games that use hi-res mode to produce transparency effects (Kirby's Dream Land, Jurassic Park...)."
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: "#333333"
            }

            Text {
                Layout.topMargin: 30
                Layout.fillWidth: true
                text: qsTr("OVERCLOCKING")
                font.pointSize: 10
                font.family: Constants.regularFontFamily
                font.weight: Font.DemiBold
                Layout.bottomMargin: 4
                color: "#a6a6a6"
            }

            Pane {
                Layout.fillWidth: true
                verticalPadding: overclockCol.spacing
                background: Rectangle {
                    radius: 8
                    color: "#25282C"
                }

                contentItem: ColumnLayout {
                    id: overclockCol
                    Option {
                        Layout.fillWidth: true
                        Layout.minimumHeight: 42
                        label: "SuperFX overclock"
                        description: "SuperFX coprocessor frequency multiplier. Can improve frame rate or cause timing errors. Values under 100% can improve game performance on slow devices."

                        control: MyComboBox {
                            model: [{
                                text: "50%",
                                value: "50%"
                            }, {
                                text: "60%",
                                value: "60%"
                            }, {
                                text: "70%",
                                value: "70%"
                            }, {
                                text: "80%",
                                value: "80%"
                            }, {
                                text: "90%",
                                value: "90%"
                            }, {
                                text: "100%",
                                value: "100%"
                            }, {
                                text: "150%",
                                value: "150%"
                            }, {
                                text: "200%",
                                value: "200%"
                            }, {
                                text: "250%",
                                value: "250%"
                            }, {
                                text: "300%",
                                value: "300%"
                            }, {
                                text: "350%",
                                value: "350%"
                            }, {
                                text: "400%",
                                value: "400%"
                            }, {
                                text: "450%",
                                value: "450%"
                            }, {
                                text: "500%",
                                value: "500%"
                            }]

                            currentIndex: 5
                            textRole: "text"
                            valueRole: "value"
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                        color: "#333333"
                    }

                    Option {
                        Layout.fillWidth: true
                        Layout.minimumHeight: 42
                        label: "CPU overclock"
                        description: "Overclock SNES CPU. May cause games to crash! Use 'Light' for shorter loading times, 'Compatible' for most games exhibiting slowdown and 'Max' only if absolutely required (Gradius 3, Super R-type...)."
                        control: MyComboBox {
                            model: [{
                                text: "None",
                                value: "disabled"
                            }, {
                                text: "Light",
                                value: "light"
                            }, {
                                text: "Compatible",
                                value: "compatible"
                            }, {
                                text: "Max",
                                value: "max"
                            }]
                            textRole: "text"
                            valueRole: "value"
                        }
                    }
                }
            }

            Text {
                Layout.topMargin: 30
                Layout.fillWidth: true
                text: qsTr("EMULATION HACKS")
                font.pointSize: 10
                font.family: Constants.regularFontFamily
                font.weight: Font.DemiBold
                Layout.bottomMargin: 4
                color: "#a6a6a6"
            }

            Pane {
                Layout.fillWidth: true
                verticalPadding: hackCol.spacing
                background: Rectangle {
                    radius: 8
                    color: "#25282C"
                }

                contentItem: ColumnLayout {
                    id: hackCol
                    ToggleOption {
                        Layout.fillWidth: true
                        label: "Increase sprite limit to reduce flickering"
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                        color: "#333333"
                    }

                    ToggleOption {
                        Layout.fillWidth: true
                        label: "Randomize system RAM at startup"
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                        color: "#333333"
                    }

                    ToggleOption {
                        Layout.fillWidth: true
                        Layout.minimumHeight: 42
                        label: "Block invalid VRAM access"
                        description: "Some homebrew/ROM hacks require this option to be disabled for correct operation."
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                        color: "#333333"
                    }

                    ToggleOption {
                        Layout.fillWidth: true
                        Layout.minimumHeight: 42
                        label: "Echo buffer hack"
                        description: "Some homebrew/ROM hacks require this option to be enabled for correct operation."
                    }
                }
            }

            Text {
                Layout.topMargin: 30
                Layout.fillWidth: true
                text: qsTr("GRAPHICAL LAYERS")
                font.pointSize: 10
                font.family: Constants.regularFontFamily
                font.weight: Font.DemiBold
                Layout.bottomMargin: 4
                color: "#a6a6a6"
            }

            Pane {
                Layout.fillWidth: true
                verticalPadding: layerCol.spacing
                background: Rectangle {
                    radius: 8
                    color: "#25282C"
                }

                contentItem: ColumnLayout {
                    id: layerCol
                    ToggleOption {
                        Layout.fillWidth: true
                        Layout.minimumHeight: 20
                        label: "Show layer 1"
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                        color: "#333333"
                    }

                    ToggleOption {
                        Layout.fillWidth: true
                        Layout.minimumHeight: 20
                        label: "Show layer 2"
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                        color: "#333333"
                    }

                    ToggleOption {
                        Layout.fillWidth: true
                        Layout.minimumHeight: 20
                        label: "Show layer 3"
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                        color: "#333333"
                    }

                    ToggleOption {
                        Layout.fillWidth: true
                        Layout.minimumHeight: 20
                        label: "Show layer 4"
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                        color: "#333333"
                    }

                    ToggleOption {
                        Layout.fillWidth: true
                        Layout.minimumHeight: 20
                        label: "Show layer 5 (sprite layer)"
                    }
                }
            }

            Text {
                Layout.topMargin: 30
                Layout.fillWidth: true
                text: qsTr("SOUND CHANNELS")
                font.pointSize: 10
                font.family: Constants.regularFontFamily
                font.weight: Font.DemiBold
                Layout.bottomMargin: 4
                color: "#a6a6a6"
            }

            Pane {
                Layout.fillWidth: true
                verticalPadding: soundCol.spacing
                background: Rectangle {
                    radius: 8
                    color: "#25282C"
                }

                contentItem: ColumnLayout {
                    id: soundCol
                    ToggleOption {
                        Layout.fillWidth: true
                        Layout.minimumHeight: 24
                        label: "Enable channel 1"
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                        color: "#333333"
                    }

                    ToggleOption {
                        Layout.fillWidth: true
                        Layout.minimumHeight: 24
                        label: "Enable channel 2"
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                        color: "#333333"
                    }

                    ToggleOption {
                        Layout.fillWidth: true
                        Layout.minimumHeight: 24
                        label: "Enable channel 3"
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                        color: "#333333"
                    }

                    ToggleOption {
                        Layout.fillWidth: true
                        Layout.minimumHeight: 24
                        label: "Enable channel 4"
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                        color: "#333333"
                    }

                    ToggleOption {
                        Layout.fillWidth: true
                        Layout.minimumHeight: 24
                        label: "Enable channel 5"
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                        color: "#333333"
                    }

                    ToggleOption {
                        Layout.fillWidth: true
                        Layout.minimumHeight: 24
                        label: "Enable channel 6"
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                        color: "#333333"
                    }

                    ToggleOption {
                        Layout.fillWidth: true
                        Layout.minimumHeight: 24
                        label: "Enable channel 7"
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                        color: "#333333"
                    }

                    ToggleOption {
                        Layout.fillWidth: true
                        Layout.minimumHeight: 24
                        label: "Enable channel 8"
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                        color: "#333333"
                    }

                    ToggleOption {
                        Layout.fillWidth: true
                        Layout.minimumHeight: 24
                        label: "Enable channel 9"
                    }
                }
            }

            // TODO: Advanced option
            // ToggleOption {
            //     Layout.fillWidth: true
            //     label: "Allow Opposing Directions"
            //     description: "Allow pressing (D-Pad Up + D-Pad Down) or (D-Pad Left + D-Pad Right) simultaneously. This isn't allowed on original hardware, so it may cause issues in some games."
            //
            //     checked: achievement_manager.unlockNotificationsEnabled
            //
            //     onCheckedChanged: {
            //         achievement_manager.unlockNotificationsEnabled = checked
            //     }
            // }

            // ToggleOption {
            //     Layout.fillWidth: true
            //     label: "Play sound"
            //     Layout.leftMargin: 24
            // }

            // Rectangle {
            //     Layout.fillWidth: true
            //     Layout.preferredHeight: 1
            //     color: "#333333"
            // }

            // ToggleOption {
            //     id: colorCorrectionOption
            //     Layout.fillWidth: true
            //     Layout.minimumHeight: 42
            //     label: "Color correction"
            //     description: "Make the screen colors more accurate to the original hardware."
            // }
            //
            // ComboBoxOption {
            //     Layout.fillWidth: true
            //     Layout.minimumHeight: 42
            //     Layout.leftMargin: 24
            //     visible: colorCorrectionOption.checked
            //     label: "Frontlight position"
            //     description: "Simulates the physical response of the Game Boy Color LCD panel when illuminated from different angles."
            // }
            //
            // Rectangle {
            //     Layout.fillWidth: true
            //     Layout.preferredHeight: 1
            //     color: "#333333"
            // }
            //
            // SliderOption {
            //     Layout.fillWidth: true
            //     Layout.minimumHeight: 42
            //     label: "Dark filter level"
            //     description: "Darken the screen to reduce glare and/or eye strain. This is useful for games with white backgrounds, as they appear harsher than intended on modern displays."
            // }
            //
            // Rectangle {
            //     Layout.fillWidth: true
            //     Layout.preferredHeight: 1
            //     color: "#333333"
            // }
            //
            // ToggleOption {
            //     Layout.fillWidth: true
            //     Layout.minimumHeight: 42
            //     label: "Simulate LCD ghosting"
            //     description: "Enables simulation of LCD ghosting effects by blending the current and previous frames."
            //
            //     onCheckedChanged: {
            //         if (checked) {
            //             emulator_config_manager.setOptionValueForPlatform(1, "gambatte_gb_colorization", "auto")
            //         } else {
            //             emulator_config_manager.setOptionValueForPlatform(1, "gambatte_gb_colorization", "disabled")
            //         }
            //     }
            //
            // }
            //
            // Rectangle {
            //     Layout.fillWidth: true
            //     Layout.preferredHeight: 1
            //     color: "#333333"
            // }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }

}