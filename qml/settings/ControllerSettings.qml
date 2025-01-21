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
        spacing: 0
        width: parent.width - 20
        // anchors.fill: parent

        ColumnLayout {
            Layout.fillWidth: true

            // Text {
            //     Layout.topMargin: 30
            //     Layout.fillWidth: true
            //     text: qsTr("Notifications")
            //     font.pointSize: 11
            //     font.family: Constants.regularFontFamily
            //     font.weight: Font.DemiBold
            //     Layout.bottomMargin: 8
            //     color: "#a6a6a6"
            // }

            ToggleOption {
                Layout.fillWidth: true
                label: "Prioritize controllers over keyboard"
                description: "When you connect a controller, replace the keyboard in the first available player number. \n\nFor example, connecting a controller while a keyboard is player one will push the keyboard to player two and the controller will be player one."

                checked: controller_manager.prioritizeControllerOverKeyboard

                onCheckedChanged: {
                    controller_manager.prioritizeControllerOverKeyboard = checked
                }
            }

        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

        }
    }
}
