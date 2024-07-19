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

            // Image {
            //     Layout.fillWidth: true
            //     Layout.preferredHeight: parent.width
            //     source: "file:system/_img/snes.svg"
            //     sourceSize.height: height
            //     fillMode: Image.PreserveAspectFit
            // }

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
                        color: "#c3c3c3"
                    }

                    checkable: false

                    onClicked: {
                        console.log(":)")
                        root.StackView.view.pop()
                    }
                }

                Text {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    text: qsTr("Default SNES Settings")
                    font.pointSize: 16
                    font.family: Constants.regularFontFamily
                    font.weight: Font.Bold
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    color: "white"
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

                    Component.onCompleted: {
                        // currentIndex = find(emulator_config_manager.getOptionValueForPlatform(1, "snes9x_region"))
                        emulator_config_manager.getOptionValueForPlatform(1, "snes9x_region")
                    }

                    onCurrentValueChanged: {
                        emulator_config_manager.setOptionValueForPlatform(1, "snes9x_region", currentValue)
                    }
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

                Component.onCompleted: {
                    // checked = emulator_config_manager.getOptionValueForPlatform(1, "snes9x_up_down_allowed") === "enabled"
                    emulator_config_manager.getOptionValueForPlatform(1, "snes9x_up_down_allowed")
                }

                onCheckedChanged: function () {
                    if (checked) {
                        emulator_config_manager.setOptionValueForPlatform(1, "snes9x_up_down_allowed", "enabled")
                    } else {
                        emulator_config_manager.setOptionValueForPlatform(1, "snes9x_up_down_allowed", "disabled")
                    }
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
                label: "Crop overscan"
                description: "Remove the borders at the top and bottom of the screen, typically unused by games and hidden by the bezel of a standard-definition television. 'Auto' will attempt to detect and crop the ~8 pixel overscan based on the current content."

                Component.onCompleted: {
                    // checked = emulator_config_manager.getOptionValueForPlatform(1, "snes9x_up_down_allowed") === "enabled"
                    emulator_config_manager.getOptionValueForPlatform(1, "snes9x_overscan")
                }

                onCheckedChanged: function () {
                    if (checked) {
                        emulator_config_manager.setOptionValueForPlatform(1, "snes9x_overscan", "enabled")
                    } else {
                        emulator_config_manager.setOptionValueForPlatform(1, "snes9x_overscan", "disabled")
                    }
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
                label: "Enable graphic clip windows"
                description: "TBD."

                Component.onCompleted: {
                    // checked = emulator_config_manager.getOptionValueForPlatform(1, "snes9x_up_down_allowed") === "enabled"
                    emulator_config_manager.getOptionValueForPlatform(1, "snes9x_gfx_clip")
                }

                onCheckedChanged: function () {
                    if (checked) {
                        emulator_config_manager.setOptionValueForPlatform(1, "snes9x_gfx_clip", "enabled")
                    } else {
                        emulator_config_manager.setOptionValueForPlatform(1, "snes9x_gfx_clip", "disabled")
                    }
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
                label: "Enable transparency effects"
                description: "TBD."

                Component.onCompleted: {
                    // checked = emulator_config_manager.getOptionValueForPlatform(1, "snes9x_up_down_allowed") === "enabled"
                    emulator_config_manager.getOptionValueForPlatform(1, "snes9x_gfx_transp")
                }

                onCheckedChanged: function () {
                    if (checked) {
                        emulator_config_manager.setOptionValueForPlatform(1, "snes9x_gfx_transp", "enabled")
                    } else {
                        emulator_config_manager.setOptionValueForPlatform(1, "snes9x_gfx_transp", "disabled")
                    }
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
                label: "Hi-Res blending"
                description: "Blend adjacent pixels when game switches to hi-res mode (512x448). Required for certain games that use hi-res mode to produce transparency effects (Kirby's Dream Land, Jurassic Park...)."


                Component.onCompleted: {
                    // checked = emulator_config_manager.getOptionValueForPlatform(1, "snes9x_up_down_allowed") === "enabled"
                    emulator_config_manager.getOptionValueForPlatform(1, "snes9x_hires_blend")
                }

                onCheckedChanged: function () {
                    if (checked) {
                        emulator_config_manager.setOptionValueForPlatform(1, "snes9x_hires_blend", "blur")
                    } else {
                        emulator_config_manager.setOptionValueForPlatform(1, "snes9x_hires_blend", "disabled")
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: "#333333"
            }

            Text {
                Layout.topMargin: 30
                Layout.fillWidth: true
                text: qsTr("Overclocking")
                font.pointSize: 11
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

                            Component.onCompleted: {
                                // currentIndex = find(emulator_config_manager.getOptionValueForPlatform(1, "snes9x_region"))
                                emulator_config_manager.getOptionValueForPlatform(1, "snes9x_overclock_superfx")
                            }

                            onCurrentValueChanged: {
                                emulator_config_manager.setOptionValueForPlatform(1, "snes9x_overclock_superfx", currentValue)
                            }
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

                            Component.onCompleted: {
                                // currentIndex = find(emulator_config_manager.getOptionValueForPlatform(1, "snes9x_region"))
                                emulator_config_manager.getOptionValueForPlatform(1, "snes9x_overclock_cycles")
                            }

                            onCurrentValueChanged: {
                                emulator_config_manager.setOptionValueForPlatform(1, "snes9x_overclock_cycles", currentValue)
                            }
                        }
                    }

                    Slider {
                        id: slider
                        Layout.fillWidth: true
                        Layout.preferredHeight: 32
                        from: 0
                        to: 3
                        stepSize: 1
                        snapMode: Slider.SnapAlways

                        handle: Rectangle {
                            color: "white"
                            width: 10
                            height: 24
                            radius: 2
                            x: slider.width * slider.visualPosition - (width / 2)
                            y: (slider.height / 2) - (height / 2)
                            z: 4
                        }

                        background: Rectangle {
                            height: 8
                            y: parent.height / 2 - (height / 2)
                            color: "grey"
                            Rectangle {
                                height: 24
                                color: "grey"
                                width: 2
                                x: -(width / 2)
                                y: -height / 2 + (parent.height / 2)
                            }

                            Rectangle {
                                height: 24
                                color: "grey"
                                width: 2
                                x: slider.width / 3 - (width / 2)
                                y: -height / 2 + (parent.height / 2)
                            }

                            Rectangle {
                                height: 24
                                width: 2
                                color: "grey"
                                x: slider.width * 2 / 3 - (width / 2)
                                y: -height / 2 + (parent.height / 2)
                            }

                            Rectangle {
                                height: 24
                                width: 2
                                color: "grey"
                                x: slider.width - (width / 2)
                                y: -height / 2 + (parent.height / 2)
                            }

                            Rectangle {
                                color: "lightblue"
                                height: parent.height
                                width: slider.width * slider.visualPosition
                            }
                        }
                    }
                }
            }

            Text {
                Layout.topMargin: 30
                Layout.fillWidth: true
                text: qsTr("Emulation hacks")
                font.pointSize: 11
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


                        Component.onCompleted: {
                            // checked = emulator_config_manager.getOptionValueForPlatform(1, "snes9x_up_down_allowed") === "enabled"
                            emulator_config_manager.getOptionValueForPlatform(1, "snes9x_reduce_sprite_flicker")
                        }

                        onCheckedChanged: function () {
                            if (checked) {
                                emulator_config_manager.setOptionValueForPlatform(1, "snes9x_reduce_sprite_flicker", "enabled")
                            } else {
                                emulator_config_manager.setOptionValueForPlatform(1, "snes9x_reduce_sprite_flicker", "disabled")
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                        color: "#333333"
                    }

                    ToggleOption {
                        Layout.fillWidth: true
                        label: "Randomize system RAM at startup"

                        Component.onCompleted: {
                            // checked = emulator_config_manager.getOptionValueForPlatform(1, "snes9x_up_down_allowed") === "enabled"
                            emulator_config_manager.getOptionValueForPlatform(1, "snes9x_randomize_memory")
                        }

                        onCheckedChanged: function () {
                            if (checked) {
                                emulator_config_manager.setOptionValueForPlatform(1, "snes9x_randomize_memory", "enabled")
                            } else {
                                emulator_config_manager.setOptionValueForPlatform(1, "snes9x_randomize_memory", "disabled")
                            }
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
                        label: "Block invalid VRAM access"
                        description: "Some homebrew/ROM hacks require this option to be disabled for correct operation."

                        Component.onCompleted: {
                            // checked = emulator_config_manager.getOptionValueForPlatform(1, "snes9x_up_down_allowed") === "enabled"
                            emulator_config_manager.getOptionValueForPlatform(1, "snes9x_block_invalid_vram_access")
                        }

                        onCheckedChanged: function () {
                            if (checked) {
                                emulator_config_manager.setOptionValueForPlatform(1, "snes9x_block_invalid_vram_access", "enabled")
                            } else {
                                emulator_config_manager.setOptionValueForPlatform(1, "snes9x_block_invalid_vram_access", "disabled")
                            }
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
                        label: "Echo buffer hack"
                        description: "Some homebrew/ROM hacks require this option to be enabled for correct operation."
                        Component.onCompleted: {
                            // checked = emulator_config_manager.getOptionValueForPlatform(1, "snes9x_up_down_allowed") === "enabled"
                            emulator_config_manager.getOptionValueForPlatform(1, "snes9x_echo_buffer_hack")
                        }

                        onCheckedChanged: function () {
                            if (checked) {
                                emulator_config_manager.setOptionValueForPlatform(1, "snes9x_echo_buffer_hack", "enabled")
                            } else {
                                emulator_config_manager.setOptionValueForPlatform(1, "snes9x_echo_buffer_hack", "disabled")
                            }
                        }
                    }
                }
            }

            Text {
                Layout.topMargin: 30
                Layout.fillWidth: true
                text: qsTr("Graphical layers")
                font.pointSize: 11
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

                        Component.onCompleted: {
                            // checked = emulator_config_manager.getOptionValueForPlatform(1, "snes9x_up_down_allowed") === "enabled"
                            emulator_config_manager.getOptionValueForPlatform(1, "snes9x_layer_1")
                        }

                        onCheckedChanged: function () {
                            if (checked) {
                                emulator_config_manager.setOptionValueForPlatform(1, "snes9x_layer_1", "enabled")
                            } else {
                                emulator_config_manager.setOptionValueForPlatform(1, "snes9x_layer_1", "disabled")
                            }
                        }
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

                        Component.onCompleted: {
                            // checked = emulator_config_manager.getOptionValueForPlatform(1, "snes9x_up_down_allowed") === "enabled"
                            emulator_config_manager.getOptionValueForPlatform(1, "snes9x_layer_2")
                        }

                        onCheckedChanged: function () {
                            if (checked) {
                                emulator_config_manager.setOptionValueForPlatform(1, "snes9x_layer_2", "enabled")
                            } else {
                                emulator_config_manager.setOptionValueForPlatform(1, "snes9x_layer_2", "disabled")
                            }
                        }
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

                        Component.onCompleted: {
                            // checked = emulator_config_manager.getOptionValueForPlatform(1, "snes9x_up_down_allowed") === "enabled"
                            emulator_config_manager.getOptionValueForPlatform(1, "snes9x_layer_3")
                        }

                        onCheckedChanged: function () {
                            if (checked) {
                                emulator_config_manager.setOptionValueForPlatform(1, "snes9x_layer_3", "enabled")
                            } else {
                                emulator_config_manager.setOptionValueForPlatform(1, "snes9x_layer_3", "disabled")
                            }
                        }
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

                        Component.onCompleted: {
                            // checked = emulator_config_manager.getOptionValueForPlatform(1, "snes9x_up_down_allowed") === "enabled"
                            emulator_config_manager.getOptionValueForPlatform(1, "snes9x_layer_4")
                        }

                        onCheckedChanged: function () {
                            if (checked) {
                                emulator_config_manager.setOptionValueForPlatform(1, "snes9x_layer_4", "enabled")
                            } else {
                                emulator_config_manager.setOptionValueForPlatform(1, "snes9x_layer_4", "disabled")
                            }
                        }
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

                        Component.onCompleted: {
                            // checked = emulator_config_manager.getOptionValueForPlatform(1, "snes9x_up_down_allowed") === "enabled"
                            emulator_config_manager.getOptionValueForPlatform(1, "snes9x_layer_5")
                        }

                        onCheckedChanged: function () {
                            if (checked) {
                                emulator_config_manager.setOptionValueForPlatform(1, "snes9x_layer_5", "enabled")
                            } else {
                                emulator_config_manager.setOptionValueForPlatform(1, "snes9x_layer_5", "disabled")
                            }
                        }
                    }
                }
            }

            Text {
                Layout.topMargin: 30
                Layout.fillWidth: true
                text: qsTr("Sound channels")
                font.pointSize: 11
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

                        Component.onCompleted: {
                            // checked = emulator_config_manager.getOptionValueForPlatform(1, "snes9x_up_down_allowed") === "enabled"
                            emulator_config_manager.getOptionValueForPlatform(1, "snes9x_sndchan_1")
                        }

                        onCheckedChanged: function () {
                            if (checked) {
                                emulator_config_manager.setOptionValueForPlatform(1, "snes9x_sndchan_1", "enabled")
                            } else {
                                emulator_config_manager.setOptionValueForPlatform(1, "snes9x_sndchan_1", "disabled")
                            }
                        }
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


                        Component.onCompleted: {
                            // checked = emulator_config_manager.getOptionValueForPlatform(1, "snes9x_up_down_allowed") === "enabled"
                            emulator_config_manager.getOptionValueForPlatform(1, "snes9x_sndchan_2")
                        }

                        onCheckedChanged: function () {
                            if (checked) {
                                emulator_config_manager.setOptionValueForPlatform(1, "snes9x_sndchan_2", "enabled")
                            } else {
                                emulator_config_manager.setOptionValueForPlatform(1, "snes9x_sndchan_2", "disabled")
                            }
                        }
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

                        Component.onCompleted: {
                            // checked = emulator_config_manager.getOptionValueForPlatform(1, "snes9x_up_down_allowed") === "enabled"
                            emulator_config_manager.getOptionValueForPlatform(1, "snes9x_sndchan_3")
                        }

                        onCheckedChanged: function () {
                            if (checked) {
                                emulator_config_manager.setOptionValueForPlatform(1, "snes9x_sndchan_3", "enabled")
                            } else {
                                emulator_config_manager.setOptionValueForPlatform(1, "snes9x_sndchan_3", "disabled")
                            }
                        }
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

                        Component.onCompleted: {
                            // checked = emulator_config_manager.getOptionValueForPlatform(1, "snes9x_up_down_allowed") === "enabled"
                            emulator_config_manager.getOptionValueForPlatform(1, "snes9x_sndchan_4")
                        }

                        onCheckedChanged: function () {
                            if (checked) {
                                emulator_config_manager.setOptionValueForPlatform(1, "snes9x_sndchan_4", "enabled")
                            } else {
                                emulator_config_manager.setOptionValueForPlatform(1, "snes9x_sndchan_4", "disabled")
                            }
                        }
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

                        Component.onCompleted: {
                            // checked = emulator_config_manager.getOptionValueForPlatform(1, "snes9x_up_down_allowed") === "enabled"
                            emulator_config_manager.getOptionValueForPlatform(1, "snes9x_sndchan_5")
                        }

                        onCheckedChanged: function () {
                            if (checked) {
                                emulator_config_manager.setOptionValueForPlatform(1, "snes9x_sndchan_5", "enabled")
                            } else {
                                emulator_config_manager.setOptionValueForPlatform(1, "snes9x_sndchan_5", "disabled")
                            }
                        }
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

                        Component.onCompleted: {
                            // checked = emulator_config_manager.getOptionValueForPlatform(1, "snes9x_up_down_allowed") === "enabled"
                            emulator_config_manager.getOptionValueForPlatform(1, "snes9x_sndchan_6")
                        }

                        onCheckedChanged: function () {
                            if (checked) {
                                emulator_config_manager.setOptionValueForPlatform(1, "snes9x_sndchan_6", "enabled")
                            } else {
                                emulator_config_manager.setOptionValueForPlatform(1, "snes9x_sndchan_6", "disabled")
                            }
                        }
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

                        Component.onCompleted: {
                            // checked = emulator_config_manager.getOptionValueForPlatform(1, "snes9x_up_down_allowed") === "enabled"
                            emulator_config_manager.getOptionValueForPlatform(1, "snes9x_sndchan_7")
                        }

                        onCheckedChanged: function () {
                            if (checked) {
                                emulator_config_manager.setOptionValueForPlatform(1, "snes9x_sndchan_7", "enabled")
                            } else {
                                emulator_config_manager.setOptionValueForPlatform(1, "snes9x_sndchan_7", "disabled")
                            }
                        }
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

                        Component.onCompleted: {
                            // checked = emulator_config_manager.getOptionValueForPlatform(1, "snes9x_up_down_allowed") === "enabled"
                            emulator_config_manager.getOptionValueForPlatform(1, "snes9x_sndchan_8")
                        }

                        onCheckedChanged: function () {
                            if (checked) {
                                emulator_config_manager.setOptionValueForPlatform(1, "snes9x_sndchan_8", "enabled")
                            } else {
                                emulator_config_manager.setOptionValueForPlatform(1, "snes9x_sndchan_8", "disabled")
                            }
                        }
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

                        Component.onCompleted: {
                            // checked = emulator_config_manager.getOptionValueForPlatform(1, "snes9x_up_down_allowed") === "enabled"
                            emulator_config_manager.getOptionValueForPlatform(1, "snes9x_sndchan_9")
                        }

                        onCheckedChanged: function () {
                            if (checked) {
                                emulator_config_manager.setOptionValueForPlatform(1, "snes9x_sndchan_9", "enabled")
                            } else {
                                emulator_config_manager.setOptionValueForPlatform(1, "snes9x_sndchan_9", "disabled")
                            }
                        }
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