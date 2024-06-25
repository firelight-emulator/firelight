import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import Firelight 1.0

Item {
    id: root

    property alias controllerProfileId: gamepadProfile.profileId
    property var currentMapping: null

    function saveMapping() {
        // for every item in buttonList, print the modelData mapping ID and print out the value of the combo box
        for (var i = 0; i < buttonList.count; i++) {
            // console.log("hi")
            console.log("Button: " + buttonList.itemAtIndex(i).modelData.display_name + " Mapping ID: " + buttonList.itemAtIndex(i).modelData.mapping_id + " Value: " + buttonList.itemAtIndex(i).currentMappingId)
        }
    }

    GamepadProfile {
        id: gamepadProfile

        onCurrentPlatformIdChanged: {
            root.currentMapping = gamepadProfile.getCurrentMapping()
            if (root.currentMapping) {
                console.log("currentMapping: " + JSON.stringify(root.currentMapping))
            } else {
                console.log("currentMapping: null")
            }
        }
    }

    RowLayout {
        anchors.fill: parent

        Pane {
            // Layout.alignment: Qt.AlignLeft
            Layout.fillHeight: true
            Layout.preferredWidth: 200
            // Layout.preferredHeight: 134 + padding * 2

            padding: 6

            // background: Rectangle {
            //     color: "#25282C"
            //     radius: 4
            // }

            background: Item {
            }

            contentItem: ListView {
                id: platformList
                orientation: ListView.Vertical

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

                    width: ListView.view.width
                    height: 48
                    autoExclusive: true

                    checked: ListView.isCurrentItem

                    onCheckedChanged: {
                        if (checked) {
                            ListView.view.currentIndex = myButton.index
                        }
                    }

                    contentItem: Row {
                        spacing: 8
                        Image {
                            source: model.icon_url
                            height: parent.height
                            // mipmap: true
                            sourceSize.height: 48
                            fillMode: Image.PreserveAspectFit
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Text {
                            text: model.display_name
                            anchors.verticalCenter: parent.verticalCenter
                            color: "white"
                            font.pointSize: 11
                            font.family: Constants.regularFontFamily
                            font.weight: Font.DemiBold
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            height: parent.height
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
                Image {
                    id: imagey
                    Layout.preferredHeight: parent.height / 2
                    source: platformList.currentItem.model.icon_url
                    Layout.preferredWidth: parent.height / 2
                    sourceSize.width: parent.height / 2
                    sourceSize.height: parent.height / 2
                    mipmap: true                    // sourceSize.height: 512
                    // sourceClipRect: Qt.rect(0, 0, 1024, 1024)
                    fillMode: Image.PreserveAspectFit

                    Rectangle {
                        color: "transparent"
                        anchors.fill: parent

                        radius: 4
                        border.color: "white"
                        border.width: 1
                    }
                }


                ListView {
                    id: buttonList
                    Layout.fillHeight: true
                    Layout.preferredWidth: 300
                    clip: true

                    ScrollBar.vertical: ScrollBar {
                    }

                    visible: platformList.currentItem.model.buttons.length > 0

                    spacing: 6

                    model: platformList.currentItem.model.buttons
                    delegate: Pane {
                        required property var modelData
                        property alias currentMappingId: dropdown.currentIndex

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
                                currentIndex: modelData.mapping_id
                                model: [
                                    {text: "Y", value: 0},
                                    {text: "A", value: 1},
                                    {text: "B", value: 2},
                                    {text: "X", value: 3},
                                    {text: "DPad Up", value: 4},
                                    {text: "DPad Down", value: 5},
                                    {text: "DPad Left", value: 6},
                                    {text: "DPad Right", value: 7},
                                    {text: "Start", value: 8},
                                    {text: "Select", value: 9},
                                    {text: "Misc 1", value: 10},
                                    {text: "Misc 2", value: 11},
                                    {text: "Misc 3", value: 12},
                                    {text: "Misc 4", value: 13},
                                    {text: "Misc 5", value: 14},
                                    {text: "Misc 6", value: 15},
                                    {text: "Misc 7", value: 16},
                                    {text: "R1", value: 17},
                                    {text: "R2", value: 18},
                                    {text: "R3", value: 19},
                                    {text: "L1", value: 20},
                                    {text: "L2", value: 21},
                                    {text: "L3", value: 22},
                                    {text: "Left Stick Up", value: 23},
                                    {text: "Left Stick Down", value: 24},
                                    {text: "Left Stick Left", value: 25},
                                    {text: "Left Stick Right", value: 26},
                                    {text: "Right Stick Up", value: 27},
                                    {text: "Right Stick Down", value: 28},
                                    {text: "Right Stick Left", value: 29},
                                    {text: "Right Stick Right", value: 30}
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


    }


}