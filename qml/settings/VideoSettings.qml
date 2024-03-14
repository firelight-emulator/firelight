import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FirelightStyle 1.0

Item {
    ColumnLayout {
        spacing: 0
        anchors.fill: parent
        ToggleOption {
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            label: "testing the thing"
        }
        ToggleOption {
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            label: "another test!"
        }
        ToggleOption {
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            label: "well now"
        }
        ToggleOption {
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            label: "aint that something"
        }
        ToggleOption {
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            label: "he really did it"
        }
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

        }
    }
}