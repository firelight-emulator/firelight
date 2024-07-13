import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

FocusScope {
    ColumnLayout {
        anchors.fill: parent

        Text {
            Layout.fillWidth: true
            text: qsTr("General")
            font.pointSize: 11
            font.family: Constants.regularFontFamily
            font.weight: Font.DemiBold
            Layout.bottomMargin: 8
            color: "#a6a6a6"
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

        ToggleOption {
            id: colorCorrectionOption
            Layout.fillWidth: true
            Layout.minimumHeight: 42
            label: "Color correction"
            description: "Make the screen colors more accurate to the original hardware."
        }

        ComboBoxOption {
            Layout.fillWidth: true
            Layout.minimumHeight: 42
            Layout.leftMargin: 24
            visible: colorCorrectionOption.checked
            label: "Frontlight position"
            description: "Simulates the physical response of the Game Boy Color LCD panel when illuminated from different angles."
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: "#333333"
        }

        SliderOption {
            Layout.fillWidth: true
            Layout.minimumHeight: 42
            label: "Dark filter level"
            description: "Darken the screen to reduce glare and/or eye strain. This is useful for games with white backgrounds, as they appear harsher than intended on modern displays."
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: "#333333"
        }

        ToggleOption {
            Layout.fillWidth: true
            Layout.minimumHeight: 42
            label: "Simulate LCD ghosting"
            description: "Enables simulation of LCD ghosting effects by blending the current and previous frames."
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: "#333333"
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}