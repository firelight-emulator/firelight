import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

FocusScope {
    id: root
    required property GameSettings gameSettings

    property var level: GameSettings.Game
    property var settings: null

    ColumnLayout {
        spacing: 8
        anchors.fill: parent
        // ComboBoxOption {
        //     Layout.fillWidth: true
        //     Layout.preferredHeight: 60
        //     label: "Window Mode?"
        // }

        // RowLayout {
        //     Layout.fillWidth: true
        //     Layout.preferredHeight: 80
        //     spacing: 0
        //
        //     Button {
        //         id: gameButton
        //         text: "Game"
        //         autoExclusive: true
        //         checkable: true
        //         checked: true
        //
        //         onToggled: {
        //             if (checked) {
        //                 console.log("Changing level to game")
        //                 root.level = GameSettings.Game
        //             }
        //         }
        //         Layout.fillWidth: true
        //     }
        //
        //     Button {
        //         text: "Platform"
        //         autoExclusive: true
        //         checkable: true
        //
        //         onToggled: {
        //             if (checked) {
        //                 console.log("Changing level to platform")
        //                 root.level = GameSettings.Platform
        //             }
        //         }
        //         Layout.fillWidth: true
        //     }
        //
        //     Button {
        //         text: "Global"
        //         autoExclusive: true
        //         checkable: true
        //
        //         onToggled: {
        //             if (checked) {
        //                 console.log("Changing level to global")
        //                 root.level = GameSettings.Global
        //             }
        //         }
        //         Layout.fillWidth: true
        //     }
        // }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            ComboBoxOption2 {
                id: pictureModeOption
                Layout.fillWidth: true
                label: "Picture mode"
                description: "Determines how the game image fills the screen."

                property bool initialized: false

                model: [
                    {text: "Aspect Ratio Fill", value: "aspect-ratio-fill"},
                    {text: "Integer Scale", value: "integer-scale"},
                    {text: "Stretch", value: "stretch"}
                ]

                Component.onCompleted: {
                    let val = gameSettings.getValue(level, "picture-mode")
                    for (let i = 0; i < pictureModeOption.model.length; i++) {
                        if (pictureModeOption.model[i].value === val) {
                            pictureModeOption.currentIndex = i
                            initialized = true;
                            return
                        }
                    }

                    initialized = true;
                }

                onCurrentValueChanged: {
                    if (!initialized) {
                        return
                    }
                    gameSettings.setValue(level, "picture-mode", pictureModeOption.currentValue)
                }

                Connections {
                    target: gameSettings
                    function onSettingChanged(key, value) {
                        let val = gameSettings.getValue(level, "picture-mode")
                        for (let i = 0; i < pictureModeOption.model.length; i++) {
                            if (pictureModeOption.model[i].value === val) {
                                pictureModeOption.currentIndex = i
                                return
                            }
                        }
                    }
                }

                Connections {
                    target: root
                    function onLevelChanged() {
                        let val = gameSettings.getValue(level, "picture-mode")
                        for (let i = 0; i < pictureModeOption.model.length; i++) {
                            if (pictureModeOption.model[i].value === val) {
                                pictureModeOption.currentIndex = i
                                return
                            }
                        }
                    }
                }
            }

            // Text {
            //     Layout.preferredWidth: 80
            //     visible: gameSettings.valueIsOverridingOther
            //     text: "Overriding platform-level setting"
            // }
        }


        ToggleOption {
            id: enableRewindOption
            Layout.fillWidth: true
            label: "Enable rewind"
            description: "Note: Rewind is always disabled when using RetroAchievements in Hardcore mode."
            property bool initialized: false

            checked: gameSettings.getValue(level, "rewind-enabled") === "true"

            onCheckedChanged: {
                if (!initialized) {
                    return
                }
                // console.log("Changing setting to: " + checked)
                gameSettings.setValue(level, "rewind-enabled", checked ? "true" : "false")
            }

            Component.onCompleted: {
                initialized = true
            }

            Connections {
                target: gameSettings
                function onSettingChanged(key, value) {
                    let val = gameSettings.getValue(level, "rewind-enabled")
                    enableRewindOption.checked = val === "true"
                }
            }

            Connections {
                target: root
                function onLevelChanged() {
                    let val = gameSettings.getValue(level, "rewind-enabled")
                    enableRewindOption.checked = val === "true"
                }
            }
        }
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

        }
    }
}