import QtQuick
import QtQuick.Controls
import QtQuick.Layouts


Item {
    ColumnLayout {
        spacing: 0
        anchors.fill: parent
        // SliderOption {
        //     Layout.fillWidth: true
        //     Layout.preferredHeight: 60
        //     label: "Master Volume"
        // }
        // ToggleOption {
        //     Layout.fillWidth: true
        //     Layout.preferredHeight: 60
        //     label: "Menu Navigation Sounds"
        // }
        // SliderOption {
        //     Layout.fillWidth: true
        //     Layout.preferredHeight: 60
        //     label: "Menu Navigation Sounds Volume"
        // }
        ToggleOption {
            Layout.fillWidth: true
            Layout.minimumHeight: 42
            label: "Progress notifications"
            description: "Show a notification when you make progress on an achievement that has progress tracking."

            checked: true
        }
    }
}