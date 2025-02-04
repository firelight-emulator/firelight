import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.platform

FocusScope {
    ColumnLayout {
        spacing: 8
        anchors.fill: parent

        DirectoryOption {
            focus: true
            id: gameDirectoryOption
            Layout.fillWidth: true
            label: "Game directory"
            description: "This is where you’ll put your game files. Firelight will automatically detect files in this directory and add them to your library."
            value: "file:///C:/Users/alexs/OneDrive/Documents/Audacity"

            // Component.onCompleted: {
            //     // checked = emulator_config_manager.getOptionValueForPlatform(1, "snes9x_up_down_allowed") === "enabled"
            //     emulator_config_manager.getOptionValueForPlatform(1, "snes9x_up_down_allowed")
            // }
            //
            // onCheckedChanged: function () {
            //     if (checked) {
            //         emulator_config_manager.setOptionValueForPlatform(1, "snes9x_up_down_allowed", "enabled")
            //     } else {
            //         emulator_config_manager.setOptionValueForPlatform(1, "snes9x_up_down_allowed", "disabled")
            //     }
            // }
        }
        DirectoryOption {
            id: saveDirectoryOption
            KeyNavigation.up: gameDirectoryOption
            Layout.fillWidth: true
            label: "Saves directory"
            description: "This is where Firelight will save your save files and Suspend Point data."
            value: SaveManager.saveDirectory

            onValueChanged: function () {
                SaveManager.saveDirectory = value
            }

            // Component.onCompleted: {
            //     // checked = emulator_config_manager.getOptionValueForPlatform(1, "snes9x_up_down_allowed") === "enabled"
            //     emulator_config_manager.getOptionValueForPlatform(1, "snes9x_up_down_allowed")
            // }
            //
            // onCheckedChanged: function () {
            //     if (checked) {
            //         emulator_config_manager.setOptionValueForPlatform(1, "snes9x_up_down_allowed", "enabled")
            //     } else {
            //         emulator_config_manager.setOptionValueForPlatform(1, "snes9x_up_down_allowed", "disabled")
            //     }
            // }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

        }
    }
}