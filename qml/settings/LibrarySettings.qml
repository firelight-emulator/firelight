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

        Text {
            Layout.topMargin: 30
            Layout.fillWidth: true
            text: qsTr("Game directories")
            font.pointSize: 11
            font.family: Constants.regularFontFamily
            font.weight: Font.DemiBold
            Layout.bottomMargin: 8
            color: "#a6a6a6"
        }

        ToggleOption {
            Layout.fillWidth: true
            label: "Achievement unlock notifications"
            description: "Show a notification when you earn an achievement."

            checked: true
        }
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

        }
    }
}