import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

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
            label: "Fullscreen"

            checked: GeneralSettings.fullscreen

            onCheckedChanged: {
                GeneralSettings.fullscreen = checked
            }
        }
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

        }
    }
}