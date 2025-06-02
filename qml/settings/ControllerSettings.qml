import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

FocusScope {
    ColumnLayout {
        id: theColumn
        spacing: 8
        anchors.fill: parent
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
                focus: true
                label: "Prioritize controllers over keyboard"
                description: "When you connect a controller, replace the keyboard in the first available player number. \n\nFor example, connecting a controller while a keyboard is player one will push the keyboard to player two and the controller will be player one."

                checked: controller_manager.prioritizeControllerOverKeyboard

                onCheckedChanged: {
                    controller_manager.prioritizeControllerOverKeyboard = checked
                }
            }

            ToggleOption {
                Layout.fillWidth: true
                focus: true
                label: "Only allow player one to navigate menus"
                description: "When this is enabled, only the controller in the player one slot will be able to navigate menus. \n\nNote: The keyboard is always able to navigate menus, regardless of this setting."

                checked: controller_manager.onlyPlayerOneCanNavigateMenus

                onCheckedChanged: {
                    controller_manager.onlyPlayerOneCanNavigateMenus = checked
                }
            }

        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

        }
    }
}
