import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FirelightStyle 1.0

Item {
    ColumnLayout {
        spacing: 0
        anchors.fill: parent
        // ComboBoxOption {
        //     Layout.fillWidth: true
        //     Layout.preferredHeight: 60
        //     label: "Window Mode?"
        // }
        ToggleOption {
            id: fullscreenOption
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            label: "Fullscreen"

            checked: GeneralSettings.fullscreen

            onCheckedChanged: {
                GeneralSettings.fullscreen = checked
            }
        }
        // ToggleOption {
        //     Layout.fillWidth: true
        //     Layout.preferredHeight: 60
        //     label: "well now"
        // }
        // ToggleOption {
        //     Layout.fillWidth: true
        //     Layout.preferredHeight: 60
        //     label: "aint that something"
        // }
        // ToggleOption {
        //     Layout.fillWidth: true
        //     Layout.preferredHeight: 60
        //     label: "he really did it"
        // }
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

        }
    }
}