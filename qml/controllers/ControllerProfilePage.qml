import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import Firelight 1.0

Item {
    id: root

    property var sourceControllerType: 0
    property var targetControllerType: 0
    property int controllerProfileId: 0

    GamepadProfile {
        id: gamepadProfile
        currentPlatformId: 1
    }

    // GamepadDescriptor {
    //     id: targetDescriptor
    //     gamepadType: root.sourceControllerType
    // }
    //
    // GamepadDescriptor {
    //     id: targetDescriptor
    //     gamepadType: root.targetControllerType
    // }

    ColumnLayout {
        anchors.fill: parent

        // Text {
        //     text: "Controllers"
        //     color: "white"
        //     font.pointSize: 24
        //     font.family: Constants.regularFontFamily
        //     horizontalAlignment: Text.AlignLeft
        //     verticalAlignment: Text.AlignVCenter
        // }
        // Button {
        //     text: "down"
        //     onClicked: {
        //         gamepadProfile.currentPlatformId = gamepadProfile.currentPlatformId - 1
        //     }
        // }
        // Button {
        //     text: "up"
        //     onClicked: {
        //         gamepadProfile.currentPlatformId = gamepadProfile.currentPlatformId + 1
        //     }
        // }
        // Button {
        //     text: "do mapping thing"
        //     onClicked: {
        //         gamepadProfile.currentMapping.rightStickXDeadzone = gamepadProfile.currentMapping.rightStickXDeadzone + 1
        //     }
        // }

        Pane {
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            Layout.preferredHeight: 134 + padding * 2

            padding: 6

            // background: Rectangle {
            //     color: "#25282C"
            //     radius: 4
            // }

            background: Item {
            }

            contentItem: ListView {
                id: platformList
                orientation: ListView.Horizontal

                clip: true

                spacing: 0

                onCurrentItemChanged: {
                    gamepadProfile.currentPlatformId = platformList.currentItem.model.platform_id
                }

                model: platform_model
                delegate: Button {
                    id: myButton
                    required property var model
                    required property int index


                    background: Rectangle {
                        color: parent.checked ? "#42464a" : parent.hovered ? "#363a3e" : "transparent"
                        radius: 4
                    }

                    enabled: model.platform_id !== -1
                    checkable: model.platform_id !== -1
                    // hoverEnabled: true

                    width: 100
                    height: 134
                    autoExclusive: true

                    checked: ListView.isCurrentItem

                    onCheckedChanged: {
                        if (checked) {
                            ListView.view.currentIndex = myButton.index
                        }
                    }

                    contentItem: Column {
                        Image {
                            source: model.icon_url
                            width: parent.width
                            // mipmap: true
                            sourceSize.width: parent.width
                            fillMode: Image.PreserveAspectFit
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                        Text {
                            text: model.display_name
                            anchors.horizontalCenter: parent.horizontalCenter
                            color: "white"
                            font.pointSize: 10
                            font.family: Constants.regularFontFamily
                            font.weight: Font.DemiBold
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            width: parent.width
                            wrapMode: Text.WordWrap

                        }
                    }
                }
            }
        }

        Pane {
            Layout.fillWidth: true
            Layout.fillHeight: true

            background: Item {
            }

            contentItem: RowLayout {
                // ListView {
                //     id: buttonList222
                //     Layout.fillHeight: true
                //     Layout.fillWidth: true
                //     Layout.horizontalStretchFactor: 1
                //     clip: true
                //
                //     visible: platformList.currentItem.model.buttons.length > 0
                //
                //     spacing: 6
                //
                //     model: 0
                //     delegate: Pane {
                //         required property var modelData
                //
                //         width: buttonList222.width
                //         height: 48
                //         padding: 0
                //         // background: Rectangle {
                //         //     color: "#25282C"
                //         //     radius: 4
                //         // }
                //
                //         background: Item {
                //         }
                //
                //         contentItem: RowLayout {
                //             spacing: 16
                //             // Image {
                //             //     source: modelData.icon_url
                //             //     anchors.verticalCenter: parent.verticalCenter
                //             //     width: 40
                //             //     height: 40
                //             //     fillMode: Image.PreserveAspectFit
                //             //
                //             //     Rectangle {
                //             //         color: "transparent"
                //             //         anchors.fill: parent
                //             //
                //             //         radius: 4
                //             //         border.color: "white"
                //             //         border.width: 1
                //             //         z: -1
                //             //     }
                //             // }
                //             Text {
                //                 Layout.leftMargin: 4
                //                 Layout.preferredWidth: 100
                //                 Layout.fillHeight: true
                //                 text: modelData.display_name
                //                 color: "white"
                //                 font.pointSize: 12
                //                 font.family: Constants.regularFontFamily
                //                 font.weight: Font.DemiBold
                //                 horizontalAlignment: Text.AlignRight
                //                 verticalAlignment: Text.AlignVCenter
                //             }
                //
                //             MyComboBox {
                //                 id: dropdown
                //                 Layout.fillWidth: true
                //                 Layout.fillHeight: true
                //                 textRole: "text"
                //                 valueRole: "value"
                //                 model: [
                //                     {text: "Left Stick Up", value: "file:system/_img/xboxseries/abutton.svg"},
                //                     {text: "Left Stick Down", value: "file:system/_img/xboxseries/abutton.svg"},
                //                     {text: "Left Stick Left", value: "file:system/_img/xboxseries/abutton.svg"},
                //                     {text: "Left Stick Right", value: "file:system/_img/xboxseries/abutton.svg"},
                //                     {text: "Right Stick Up", value: "file:system/_img/xboxseries/abutton.svg"},
                //                     {text: "Right Stick Down", value: "file:system/_img/xboxseries/abutton.svg"},
                //                     {text: "Right Stick Left", value: "file:system/_img/xboxseries/abutton.svg"},
                //                     {text: "Right Stick Right", value: "file:system/_img/xboxseries/abutton.svg"},
                //                     {text: "A", value: "file:system/_img/xboxseries/abutton.svg"},
                //                     {text: "B", value: "file:system/_img/xboxseries/abutton.svg"},
                //                     {text: "X", value: "file:system/_img/xboxseries/abutton.svg"},
                //                     {text: "Y", value: "file:system/_img/xboxseries/abutton.svg"},
                //                     {text: "DPad Up", value: "file:system/_img/xboxseries/abutton.svg"},
                //                     {text: "DPad Down", value: "file:system/_img/xboxseries/abutton.svg"},
                //                     {text: "DPad Left", value: "file:system/_img/xboxseries/abutton.svg"},
                //                     {text: "DPad Right", value: "file:system/_img/xboxseries/abutton.svg"},
                //                     {text: "LB", value: "file:system/_img/xboxseries/abutton.svg"},
                //                     {text: "LT", value: "file:system/_img/xboxseries/abutton.svg"},
                //                     {text: "LSB", value: "file:system/_img/xboxseries/abutton.svg"},
                //                     {text: "RB", value: "file:system/_img/xboxseries/abutton.svg"},
                //                     {text: "RT", value: "file:system/_img/xboxseries/abutton.svg"},
                //                     {text: "RSB", value: "file:system/_img/xboxseries/abutton.svg"},
                //                     {text: "Options", value: "file:system/_img/xboxseries/abutton.svg"},
                //                     {text: "Menu", value: "file:system/_img/xboxseries/abutton.svg"},
                //                     {text: "Share", value: "file:system/_img/xboxseries/abutton.svg"}
                //                 ]
                //
                //                 // currentIndex: 0
                //                 // onCurrentIndexChanged: {
                //                 //     controller_manager.setPlatformIndex(currentIndex)
                //                 // }
                //             }
                //
                //         }
                //     }
                // }

                // ListView {
                //     id: stickList
                //     Layout.fillHeight: true
                //     Layout.fillWidth: true
                //     Layout.horizontalStretchFactor: 1
                //
                //     visible: platformList.currentItem.model.sticks.length > 0
                //
                //     interactive: false
                //     height: contentHeight
                //
                //     spacing: 8
                //
                //     model: platformList.currentItem.model.sticks
                //     delegate: Pane {
                //         required property var modelData
                //
                //         width: stickList.width
                //         height: 240
                //         padding: 8
                //         background: Rectangle {
                //             color: "#25282C"
                //             radius: 4
                //         }
                //
                //         contentItem: ColumnLayout {
                //             RowLayout {
                //                 Layout.fillWidth: true
                //                 spacing: 8
                //                 Text {
                //                     text: modelData.display_name
                //                     color: "white"
                //                     font.pointSize: 12
                //                     font.family: Constants.regularFontFamily
                //                     font.weight: Font.DemiBold
                //                     horizontalAlignment: Text.AlignLeft
                //                     verticalAlignment: Text.AlignVCenter
                //                 }
                //
                //                 MyComboBox {
                //                     id: stickDropdown
                //                     Layout.preferredWidth: 200
                //                     Layout.preferredHeight: 40
                //                     Layout.alignment: Qt.AlignCenter
                //                     textRole: "text"
                //                     valueRole: "value"
                //                     model: [
                //                         {text: "Left Stick", value: "file:system/_img/xboxseries/abutton.svg"},
                //                         {text: "Right Stick", value: "file:system/_img/xboxseries/abutton.svg"},
                //                     ]
                //                 }
                //
                //             }
                //             Text {
                //                 Layout.fillWidth: true
                //                 text: "Flip X (Horizontal) Axis"
                //                 color: "white"
                //                 font.pointSize: 12
                //                 font.family: Constants.regularFontFamily
                //                 font.weight: Font.DemiBold
                //                 horizontalAlignment: Text.AlignLeft
                //                 verticalAlignment: Text.AlignVCenter
                //             }
                //             Text {
                //                 Layout.fillWidth: true
                //                 text: "Flip Y (Vertical) Axis"
                //                 color: "white"
                //                 font.pointSize: 12
                //                 font.family: Constants.regularFontFamily
                //                 font.weight: Font.DemiBold
                //                 horizontalAlignment: Text.AlignLeft
                //                 verticalAlignment: Text.AlignVCenter
                //             }
                //             Text {
                //                 Layout.fillWidth: true
                //                 text: "Deadzone"
                //                 color: "white"
                //                 font.pointSize: 12
                //                 font.family: Constants.regularFontFamily
                //                 font.weight: Font.DemiBold
                //                 horizontalAlignment: Text.AlignLeft
                //                 verticalAlignment: Text.AlignVCenter
                //             }
                //             Text {
                //                 Layout.fillWidth: true
                //                 text: "Sensitivity"
                //                 color: "white"
                //                 font.pointSize: 12
                //                 font.family: Constants.regularFontFamily
                //                 font.weight: Font.DemiBold
                //                 horizontalAlignment: Text.AlignLeft
                //                 verticalAlignment: Text.AlignVCenter
                //             }
                //         }
                //     }
                // }

                Image {
                    id: imagey
                    Layout.fillHeight: true
                    source: platformList.currentItem.model.icon_url
                    // width: width > parent.width / 2 ? parent.width / 2 : width
                    // mipmap: true
                    Layout.preferredWidth: parent.height
                    sourceSize.width: 512
                    sourceSize.height: 512
                    // sourceSize.height: 512
                    sourceClipRect: Qt.rect(0, 0, 1024, 1024)
                    fillMode: Image.PreserveAspectFit
                }


                ListView {
                    id: buttonList
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Layout.horizontalStretchFactor: 1
                    clip: true

                    visible: platformList.currentItem.model.buttons.length > 0

                    spacing: 6

                    model: platformList.currentItem.model.buttons
                    delegate: Pane {
                        required property var modelData

                        width: buttonList.width
                        height: 48
                        padding: 0
                        // background: Rectangle {
                        //     color: "#25282C"
                        //     radius: 4
                        // }

                        background: Item {
                        }

                        contentItem: RowLayout {
                            spacing: 16
                            // Image {
                            //     source: modelData.icon_url
                            //     anchors.verticalCenter: parent.verticalCenter
                            //     width: 40
                            //     height: 40
                            //     fillMode: Image.PreserveAspectFit
                            //
                            //     Rectangle {
                            //         color: "transparent"
                            //         anchors.fill: parent
                            //
                            //         radius: 4
                            //         border.color: "white"
                            //         border.width: 1
                            //         z: -1
                            //     }
                            // }
                            Text {
                                Layout.leftMargin: 4
                                Layout.preferredWidth: 100
                                Layout.fillHeight: true
                                text: modelData.display_name
                                color: "white"
                                font.pointSize: 12
                                font.family: Constants.regularFontFamily
                                font.weight: Font.DemiBold
                                horizontalAlignment: Text.AlignRight
                                verticalAlignment: Text.AlignVCenter
                            }

                            MyComboBox {
                                id: dropdown
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                textRole: "text"
                                valueRole: "value"
                                model: [
                                    {text: "Left Stick Up", value: "file:system/_img/xboxseries/abutton.svg"},
                                    {text: "Left Stick Down", value: "file:system/_img/xboxseries/abutton.svg"},
                                    {text: "Left Stick Left", value: "file:system/_img/xboxseries/abutton.svg"},
                                    {text: "Left Stick Right", value: "file:system/_img/xboxseries/abutton.svg"},
                                    {text: "Right Stick Up", value: "file:system/_img/xboxseries/abutton.svg"},
                                    {text: "Right Stick Down", value: "file:system/_img/xboxseries/abutton.svg"},
                                    {text: "Right Stick Left", value: "file:system/_img/xboxseries/abutton.svg"},
                                    {text: "Right Stick Right", value: "file:system/_img/xboxseries/abutton.svg"},
                                    {text: "A", value: "file:system/_img/xboxseries/abutton.svg"},
                                    {text: "B", value: "file:system/_img/xboxseries/abutton.svg"},
                                    {text: "X", value: "file:system/_img/xboxseries/abutton.svg"},
                                    {text: "Y", value: "file:system/_img/xboxseries/abutton.svg"},
                                    {text: "DPad Up", value: "file:system/_img/xboxseries/abutton.svg"},
                                    {text: "DPad Down", value: "file:system/_img/xboxseries/abutton.svg"},
                                    {text: "DPad Left", value: "file:system/_img/xboxseries/abutton.svg"},
                                    {text: "DPad Right", value: "file:system/_img/xboxseries/abutton.svg"},
                                    {text: "LB", value: "file:system/_img/xboxseries/abutton.svg"},
                                    {text: "LT", value: "file:system/_img/xboxseries/abutton.svg"},
                                    {text: "LSB", value: "file:system/_img/xboxseries/abutton.svg"},
                                    {text: "RB", value: "file:system/_img/xboxseries/abutton.svg"},
                                    {text: "RT", value: "file:system/_img/xboxseries/abutton.svg"},
                                    {text: "RSB", value: "file:system/_img/xboxseries/abutton.svg"},
                                    {text: "Options", value: "file:system/_img/xboxseries/abutton.svg"},
                                    {text: "Menu", value: "file:system/_img/xboxseries/abutton.svg"},
                                    {text: "Share", value: "file:system/_img/xboxseries/abutton.svg"}
                                ]

                                // currentIndex: 0
                                // onCurrentIndexChanged: {
                                //     controller_manager.setPlatformIndex(currentIndex)
                                // }
                            }

                        }
                    }
                }
            }
        }


        // Component {
        //     id: buttonDropdown
        //     MyComboBox {
        //         id: dropdown
        //         width: 200
        //         height: 40
        //         model: [
        //             {display_name: "A", icon_url: "file:system/_img/xboxseries/abutton.svg"},
        //             {display_name: "A", icon_url: "file:system/_img/xboxseries/abutton.svg"},
        //             {display_name: "A", icon_url: "file:system/_img/xboxseries/abutton.svg"},
        //             {display_name: "A", icon_url: "file:system/_img/xboxseries/abutton.svg"}
        //         ]
        //
        //         // currentIndex: 0
        //         // onCurrentIndexChanged: {
        //         //     controller_manager.setPlatformIndex(currentIndex)
        //         // }
        //     }
        // }

        // RowLayout {
        //     Layout.alignment: Qt.AlignHCenter
        //     Layout.fillHeight: true
        //     Layout.fillWidth: true
        //
        //     spacing: 8
        //
        //
        //
        //
        // }


    }


}