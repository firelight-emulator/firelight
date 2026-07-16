import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Flickable {
    boundsBehavior: Flickable.StopAtBounds
    contentHeight: theColumn.implicitHeight

    ScrollBar.vertical: ScrollBar {
    }

    ColumnLayout {
        id: theColumn
        width: parent.width

        SettingsSectionHeader {
            sectionName: "Achievements"
            showTopPadding: false
        }


        ToggleSettingItem {
            Layout.fillWidth: true
            label: "Achievement earned"
        }

        SettingsSectionHeader {
            Layout.topMargin: 8
            sectionName: "Controllers"
            showTopPadding: false
        }


        ToggleSettingItem {
            Layout.fillWidth: true
            label: "Controller connected"
        }


        ToggleSettingItem {
            Layout.fillWidth: true
            label: "Controller disconnected"
        }


        ToggleSettingItem {
            Layout.fillWidth: true
            label: "Low battery warning"
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

        }
    }
}
