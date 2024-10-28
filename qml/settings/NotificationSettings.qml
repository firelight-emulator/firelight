import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Flickable {
    boundsBehavior: Flickable.StopAtBounds
    contentHeight: theColumn.height

    ScrollBar.vertical: ScrollBar {
    }

    ColumnLayout {
        id: theColumn
        width: parent.width
        spacing: 0

        Text {
            leftPadding: 12
            Layout.topMargin: 30
            Layout.fillWidth: true
            text: qsTr("Achievements")
            font.pixelSize: 15
            font.family: Constants.regularFontFamily
            font.weight: Font.DemiBold
            Layout.bottomMargin: 8
            color: "#a6a6a6"
        }


        ToggleOption {
            Layout.fillWidth: true
            label: "Achievement earned"
        }

        Text {
            leftPadding: 12
            Layout.topMargin: 30
            Layout.fillWidth: true
            text: qsTr("Controllers")
            font.pixelSize: 15
            font.family: Constants.regularFontFamily
            font.weight: Font.DemiBold
            Layout.bottomMargin: 8
            color: "#a6a6a6"
        }

        ToggleOption {
            Layout.fillWidth: true
            label: "Controller connected"
        }

        ToggleOption {
            Layout.fillWidth: true
            label: "Controller disconnected"
        }

        ToggleOption {
            Layout.fillWidth: true
            label: "Low battery warning"
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

        }
    }
}
