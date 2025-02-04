import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

FocusScope {
    ColumnLayout {
        spacing: 8
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
            focus: true

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