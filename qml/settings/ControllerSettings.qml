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
                id: prioritizeControllerToggle
                Layout.fillWidth: true
                focus: true
                label: "Prioritize controllers over keyboard"



        background: Item {
                            property var radius: 2
                            Rectangle {
                                anchors.fill: parent
                                color: ColorPalette.neutral300
                                opacity: prioritizeControllerToggle.hovered || prioritizeControllerToggle.activeFocus ? 0.2 : 0
                            }

                            Rectangle {
                                width: parent.width
                                height: 1
                                anchors.top: parent.bottom
                                color: ColorPalette.neutral300
                                opacity: 0.3
                            }
                            Rectangle {
                                width: parent.width
                                height: 1
                                anchors.bottom: parent.top
                                color: ColorPalette.neutral300
                                opacity: 0.3
                            }
                        }

                checked: InputService.prioritizeControllerOverKeyboard

                onCheckedChanged: {
                    InputService.prioritizeControllerOverKeyboard = checked
                }
            }

            Text {
                font.pixelSize: 15
                font.family: Constants.regularFontFamily
                font.weight: Font.Medium
                Layout.fillWidth: true
                color: ColorPalette.neutral100
                text: "When you connect a controller, replace the keyboard in the first available player number. \nFor example, connecting a controller while a keyboard is player one will push the keyboard to player two and the controller will be player one."
                leftPadding: 12
                lineHeight: 1.2
                wrapMode: Text.WordWrap
                Layout.bottomMargin: 20
            }

            ToggleOption {
                id: playerOneOnlyMenusToggle
                Layout.fillWidth: true
                focus: true
                label: "Only allow player one to navigate menus"
                KeyNavigation.up: prioritizeControllerToggle



                background: Item {
                    property var radius: 2
                    Rectangle {
                        anchors.fill: parent
                        color: ColorPalette.neutral300
                        opacity: playerOneOnlyMenusToggle.hovered || playerOneOnlyMenusToggle.activeFocus ? 0.2 : 0
                    }

                    Rectangle {
                        width: parent.width
                        height: 1
                        anchors.top: parent.bottom
                        color: ColorPalette.neutral300
                        opacity: 0.3
                    }
                    Rectangle {
                        width: parent.width
                        height: 1
                        anchors.bottom: parent.top
                        color: ColorPalette.neutral300
                        opacity: 0.3
                    }
                }

                checked: InputService.onlyPlayerOneCanNavigateMenus

                onCheckedChanged: {
                    InputService.onlyPlayerOneCanNavigateMenus = checked
                }
            }

            Text {
                font.pixelSize: 15
                font.family: Constants.regularFontFamily
                font.weight: Font.Medium
                Layout.fillWidth: true
                color: ColorPalette.neutral100
                text: "When this is enabled, only the controller in the player one slot will be able to navigate menus. \nNote: The keyboard is always able to navigate menus, regardless of this setting."
                leftPadding: 12
                lineHeight: 1.2
                wrapMode: Text.WordWrap
                Layout.bottomMargin: 20
            }

        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

        }
    }
}
