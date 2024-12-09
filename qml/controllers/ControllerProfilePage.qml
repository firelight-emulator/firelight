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

        currentPlatformId: platformList.currentItem.model.platform_id

        onCurrentPlatformIdChanged: {
            root.currentMapping = gamepadProfile.getCurrentMapping()
            if (root.currentMapping) {
                console.log("currentMapping: " + JSON.stringify(root.currentMapping))
            } else {
                console.log("currentMapping: null")
            }
        }
    }

    PlatformMetadata {
        id: platformMetadata
        platformId: platformList.currentItem.model.platform_id
    }

    FirelightDialog {
        id: dialog
        // title: "Assign all buttons"
        // message: "Are you sure you want to assign all buttons to the default mappings?"
        showButtons: false

        onAboutToShow: {
            frameAnimation.reset()
        }

        onOpened: function () {
            // theBar.widthThing = parent.width
            timer.start()
        }

        contentItem: ColumnLayout {
            spacing: 12
            Image {
                Layout.preferredHeight: 300
                Layout.preferredWidth: 300
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                source: platformList.currentItem.model.icon_url
                sourceSize.height: 300
                fillMode: Image.PreserveAspectFit
            }
            Text {
                text: "Press a button on your Xbox Series controller to assign it to this Nintendo 64 input:"
                wrapMode: Text.WordWrap
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: parent.width * 5 / 6

                color: "white"
                font.family: Constants.regularFontFamily
                font.weight: Font.Light
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: 18
            }

            Text {
                text: "C-Up"
                wrapMode: Text.WordWrap
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                color: ColorPalette.neutral200
                font.pixelSize: 20
                font.weight: Font.Bold
                font.family: Constants.regularFontFamily
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            Timer {
                id: timer
                interval: 4000
                running: false
                repeat: false
                onTriggered: {
                    dialog.reject()
                    // dialog.close()
                    // saveMapping()
                }
            }

            FrameAnimation {
                id: frameAnimation
                running: timer.running
            }

            Rectangle {
                id: theBar
                property var widthThing: parent.width * ((timer.interval - frameAnimation.elapsedTime * 1000) / timer.interval)
                Layout.preferredWidth: widthThing
                Layout.preferredHeight: 10
                color: "green"
            }


        }
    }

    Pane {
        id: headerBar
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.left: parent.left
        height: 72
        padding: 16

        background: Item {
        }

        contentItem: RowLayout {
            spacing: 24

            Text {
                text: "Edit controller profile"
                color: ColorPalette.neutral100
                font.pixelSize: 24
                font.weight: Font.Normal
                font.family: Constants.regularFontFamily
                Layout.fillHeight: true
                Layout.leftMargin: 24
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }


            Item {
                Layout.fillHeight: true
                Layout.fillWidth: true
            }

            FirelightButton {
                tooltipLabel: "Close"
                flat: true

                Layout.fillHeight: true
                Layout.preferredWidth: height
                Layout.rightMargin: 24

                iconCode: "\ue5cd"

                onClicked: {
                    root.StackView.view.popCurrentItem(StackView.PopTransition)
                }
            }
        }
    }

    RowLayout {
        anchors.top: headerBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: 40
        anchors.rightMargin: 40

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.horizontalStretchFactor: 1
        }

        ListView {
            id: platformList
            spacing: 0
            Layout.maximumWidth: 300
            Layout.preferredWidth: 300
            Layout.minimumWidth: 200
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignRight
            focus: true

            // onCurrentItemChanged: {
            //     gamepadProfile.currentPlatformId = platformList.currentItem.model.platform_id
            // }

            model: platform_model
            delegate: FirelightMenuItem {
                required property var model
                required property var index

                labelText: model.display_name
                property bool showGlobalCursor: true
                // labelIcon: "\ue40a"
                height: 50
                width: ListView.view.width
                checked: ListView.isCurrentItem

                onToggled: {
                    if (checked) {
                        ListView.view.currentIndex = index
                    }
                }
            }
        }

        // StackView {
        //     id: rightHalf
        //
        //     Layout.fillHeight: true
        //     Layout.fillWidth: true
        //     Layout.maximumWidth: 1000
        //     Layout.preferredWidth: 1000
        //     Layout.minimumWidth: 500
        //     Layout.alignment: Qt.AlignLeft
        //     Layout.leftMargin: 12
        //
        //     Connections {
        //         target: root
        //
        //         function onSectionChanged() {
        //             if (root.section === "library") {
        //                 rightHalf.replace(librarySettings)
        //             } else if (root.section === "notifications") {
        //                 rightHalf.replace(notificationSettings)
        //             } else if (root.section === "achievements") {
        //                 rightHalf.replace(retroAchievementSettings)
        //             } else if (root.section === "sound") {
        //                 rightHalf.replace(soundSettings)
        //             } else if (root.section === "audiovideo") {
        //                 rightHalf.replace(videoSettings)
        //             } else if (root.section === "platforms/gbc") {
        //                 rightHalf.replace(gbcSettings)
        //             } else if (root.section === "platforms/gb") {
        //                 rightHalf.replace(gbSettings)
        //             } else if (root.section === "platforms/snes") {
        //                 rightHalf.replace(snesSettings)
        //             } else if (root.section === "platforms") {
        //                 rightHalf.replace(platformSettings)
        //             } else if (root.section === "platforms/gba") {
        //                 rightHalf.replace(gbaSettings)
        //             } else if (root.section === "platforms/n64") {
        //                 rightHalf.replace(n64Settings)
        //             } else if (root.section === "platforms/nds") {
        //                 rightHalf.replace(ndsSettings)
        //             } else if (root.section === "platforms/nes") {
        //                 rightHalf.replace(nesSettings)
        //             } else if (root.section === "platforms/mastersystem") {
        //                 rightHalf.replace(masterSystemSettings)
        //             } else if (root.section === "platforms/genesis") {
        //                 rightHalf.replace(genesisSettings)
        //             } else if (root.section === "platforms/gamegear") {
        //                 rightHalf.replace(gameGearSettings)
        //             }
        //         }
        //     }
        //
        //     // initialItem: librarySettings
        //
        //     pushEnter: Transition {
        //     }
        //
        //     pushExit: Transition {
        //     }
        //
        //     popEnter: Transition {
        //     }
        //
        //     popExit: Transition {
        //     }
        //
        //     replaceEnter: Transition {
        //         NumberAnimation {
        //             property: "opacity";
        //             from: 0.0;
        //             to: 1.0
        //             duration: 200
        //         }
        //         NumberAnimation {
        //             property: "y";
        //             from: 30 * (root.movingDown ? 1 : -1);
        //             to: 0
        //             duration: 200
        //             easing.type: Easing.InOutQuad
        //         }
        //     }
        //
        //     replaceExit: Transition {
        //         NumberAnimation {
        //             property: "opacity";
        //             from: 1.0;
        //             to: 0.0
        //             duration: 20
        //         }
        //         NumberAnimation {
        //             property: "y";
        //             from: 0;
        //             to: 30 * (root.movingDown ? -1 : 1)
        //             duration: 200
        //             easing.type: Easing.InOutQuad
        //         }
        //     }
        // }


        ColumnLayout {
            id: col
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.maximumWidth: 840
            Layout.preferredWidth: 840
            Layout.minimumWidth: 500
            Layout.alignment: Qt.AlignLeft
            Layout.leftMargin: 12

            spacing: 8

            Image {
                id: imagey
                Layout.preferredHeight: 300
                Layout.preferredWidth: 300
                Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
                source: platformList.currentItem.model.icon_url
                sourceSize.height: 300
                mipmap: true                    // sourceSize.height: 512
                // sourceClipRect: Qt.rect(0, 0, 1024, 1024)
                fillMode: Image.PreserveAspectFit

                Behavior on width {
                    NumberAnimation {
                        duration: 100
                        easing.type: Easing.InOutQuad
                    }
                }
                Behavior on height {
                    NumberAnimation {
                        duration: 100
                        easing.type: Easing.InOutQuad
                    }
                }

                // Rectangle {
                //     color: "transparent"
                //     anchors.fill: parent
                //
                //     radius: 4
                //     border.color: "white"
                //     border.width: 1
                // }
            }

            FirelightButton {
                Layout.alignment: Qt.AlignRight
                label: "Assign all"
                Layout.bottomMargin: 8
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignLeft
                Layout.maximumHeight: 40
                Text {
                    Layout.preferredWidth: 240
                    Layout.maximumWidth: 240
                    Layout.alignment: Qt.AlignLeft
                    Layout.fillHeight: true
                    text: platformList.currentItem.model.display_name + " controls"
                    color: ColorPalette.neutral200
                    font.pixelSize: 16
                    font.family: Constants.regularFontFamily
                    font.weight: Font.DemiBold
                    verticalAlignment: Text.AlignVCenter
                }
                Text {
                    id: dropdown
                    Layout.preferredWidth: 240
                    Layout.maximumWidth: 240
                    Layout.alignment: Qt.AlignLeft
                    Layout.fillHeight: true
                    Layout.leftMargin: 12
                    text: "Your controls"
                    color: ColorPalette.neutral200
                    font.pixelSize: 16
                    font.family: Constants.regularFontFamily
                    font.weight: Font.DemiBold
                    verticalAlignment: Text.AlignVCenter
                }
                Item {
                    Layout.alignment: Qt.AlignLeft

                    Layout.preferredHeight: 42
                    Layout.preferredWidth: height
                    Layout.maximumWidth: height
                }
            }
            Rectangle {
                Layout.minimumHeight: 1
                Layout.maximumHeight: 1
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter
                color: ColorPalette.neutral600
            }

            Item {
                Layout.fillHeight: true
                Layout.fillWidth: true
                visible: platformList.currentItem.model.buttons.length === 0
            }

            ListView {
                id: buttonList
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter
                clip: true

                ScrollBar.vertical: ScrollBar {
                }

                visible: platformList.currentItem.model.buttons.length > 0

                spacing: 12

                footer: Item {
                    width: ListView.view.width
                    height: 24
                }

                model: platformList.currentItem.model.buttons
                delegate: RowLayout {
                    height: 48
                    width: ListView.view.width
                    spacing: 12
                    Text {
                        Layout.preferredWidth: 240
                        Layout.maximumWidth: 240
                        Layout.alignment: Qt.AlignLeft
                        Layout.fillHeight: true
                        text: modelData.display_name
                        color: "white"
                        font.pixelSize: 16
                        font.family: Constants.regularFontFamily
                        font.weight: Font.DemiBold
                        verticalAlignment: Text.AlignVCenter
                    }
                    MyComboBox {
                        id: dropdown
                        Layout.preferredWidth: 240
                        Layout.maximumWidth: 240
                        Layout.alignment: Qt.AlignLeft
                        Layout.fillHeight: true
                        textRole: "text"
                        valueRole: "value"
                        currentIndex: modelData.retropad_button
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
                    FirelightButton {
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        tooltipLabel: "Assign"
                        flat: true

                        Layout.preferredHeight: 42
                        Layout.preferredWidth: height
                        Layout.maximumWidth: height

                        iconCode: "\ue3c9"

                        onClicked: {
                            dialog.open()
                        }
                    }

                    Item {
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                    }

                }
                // delegate: Pane {
                //     required property var modelData
                //     property alias currentMappingId: dropdown.currentIndex
                //
                //     width: buttonList.width
                //     height: 48
                //     padding: 0
                //     // background: Rectangle {
                //     //     color: "#25282C"
                //     //     radius: 4
                //     // }
                //
                //     background: Item {
                //     }
                //
                //     contentItem: RowLayout {
                //         spacing: 16
                //         // Image {
                //         //     source: modelData.icon_url
                //         //     anchors.verticalCenter: parent.verticalCenter
                //         //     width: 40
                //         //     height: 40
                //         //     fillMode: Image.PreserveAspectFit
                //         //
                //         //     Rectangle {
                //         //         color: "transparent"
                //         //         anchors.fill: parent
                //         //
                //         //         radius: 4
                //         //         border.color: "white"
                //         //         border.width: 1
                //         //         z: -1
                //         //     }
                //         // }
                //         Text {
                //             Layout.leftMargin: 4
                //             Layout.preferredWidth: 100
                //             Layout.fillHeight: true
                //             text: modelData.display_name
                //             color: "white"
                //             font.pointSize: 12
                //             font.family: Constants.regularFontFamily
                //             font.weight: Font.DemiBold
                //             horizontalAlignment: Text.AlignRight
                //             verticalAlignment: Text.AlignVCenter
                //         }
                //
                //         MyComboBox {
                //             id: dropdown
                //             Layout.fillWidth: true
                //             Layout.fillHeight: true
                //             textRole: "text"
                //             valueRole: "value"
                //             currentIndex: modelData.mapping_id
                //             model: [
                //                 {text: "Y", value: 0},
                //                 {text: "A", value: 1},
                //                 {text: "B", value: 2},
                //                 {text: "X", value: 3},
                //                 {text: "DPad Up", value: 4},
                //                 {text: "DPad Down", value: 5},
                //                 {text: "DPad Left", value: 6},
                //                 {text: "DPad Right", value: 7},
                //                 {text: "Start", value: 8},
                //                 {text: "Select", value: 9},
                //                 {text: "Misc 1", value: 10},
                //                 {text: "Misc 2", value: 11},
                //                 {text: "Misc 3", value: 12},
                //                 {text: "Misc 4", value: 13},
                //                 {text: "Misc 5", value: 14},
                //                 {text: "Misc 6", value: 15},
                //                 {text: "Misc 7", value: 16},
                //                 {text: "R1", value: 17},
                //                 {text: "R2", value: 18},
                //                 {text: "R3", value: 19},
                //                 {text: "L1", value: 20},
                //                 {text: "L2", value: 21},
                //                 {text: "L3", value: 22},
                //                 {text: "Left Stick Up", value: 23},
                //                 {text: "Left Stick Down", value: 24},
                //                 {text: "Left Stick Left", value: 25},
                //                 {text: "Left Stick Right", value: 26},
                //                 {text: "Right Stick Up", value: 27},
                //                 {text: "Right Stick Down", value: 28},
                //                 {text: "Right Stick Left", value: 29},
                //                 {text: "Right Stick Right", value: 30}
                //             ]
                //
                //             // currentIndex: 0
                //             // onCurrentIndexChanged: {
                //             //     controller_manager.setPlatformIndex(currentIndex)
                //             // }
                //         }
                //
                //     }
                // }
            }
        }


        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.horizontalStretchFactor: 1
        }

        // Pane {
        //     Layout.fillWidth: true
        //     Layout.fillHeight: true
        //
        //     background: Item {
        //     }
        //
        //     contentItem: RowLayout {
        //         Image {
        //             id: imagey
        //             Layout.preferredHeight: parent.height / 2
        //             source: platformList.currentItem.model.icon_url
        //             Layout.preferredWidth: parent.height / 2
        //             sourceSize.width: parent.height / 2
        //             sourceSize.height: parent.height / 2
        //             mipmap: true                    // sourceSize.height: 512
        //             // sourceClipRect: Qt.rect(0, 0, 1024, 1024)
        //             fillMode: Image.PreserveAspectFit
        //
        //             Rectangle {
        //                 color: "transparent"
        //                 anchors.fill: parent
        //
        //                 radius: 4
        //                 border.color: "white"
        //                 border.width: 1
        //             }
        //         }
        //
        //
        //         ListView {
        //             id: buttonList
        //             Layout.fillHeight: true
        //             Layout.preferredWidth: 300
        //             clip: true
        //
        //             ScrollBar.vertical: ScrollBar {
        //             }
        //
        //             visible: platformList.currentItem.model.buttons.length > 0
        //
        //             spacing: 6
        //
        //             model: platformList.currentItem.model.buttons
        //             delegate: Pane {
        //                 required property var modelData
        //                 property alias currentMappingId: dropdown.currentIndex
        //
        //                 width: buttonList.width
        //                 height: 48
        //                 padding: 0
        //                 // background: Rectangle {
        //                 //     color: "#25282C"
        //                 //     radius: 4
        //                 // }
        //
        //                 background: Item {
        //                 }
        //
        //                 contentItem: RowLayout {
        //                     spacing: 16
        //                     // Image {
        //                     //     source: modelData.icon_url
        //                     //     anchors.verticalCenter: parent.verticalCenter
        //                     //     width: 40
        //                     //     height: 40
        //                     //     fillMode: Image.PreserveAspectFit
        //                     //
        //                     //     Rectangle {
        //                     //         color: "transparent"
        //                     //         anchors.fill: parent
        //                     //
        //                     //         radius: 4
        //                     //         border.color: "white"
        //                     //         border.width: 1
        //                     //         z: -1
        //                     //     }
        //                     // }
        //                     Text {
        //                         Layout.leftMargin: 4
        //                         Layout.preferredWidth: 100
        //                         Layout.fillHeight: true
        //                         text: modelData.display_name
        //                         color: "white"
        //                         font.pointSize: 12
        //                         font.family: Constants.regularFontFamily
        //                         font.weight: Font.DemiBold
        //                         horizontalAlignment: Text.AlignRight
        //                         verticalAlignment: Text.AlignVCenter
        //                     }
        //
        //                     MyComboBox {
        //                         id: dropdown
        //                         Layout.fillWidth: true
        //                         Layout.fillHeight: true
        //                         textRole: "text"
        //                         valueRole: "value"
        //                         currentIndex: modelData.mapping_id
        //                         model: [
        //                             {text: "Y", value: 0},
        //                             {text: "A", value: 1},
        //                             {text: "B", value: 2},
        //                             {text: "X", value: 3},
        //                             {text: "DPad Up", value: 4},
        //                             {text: "DPad Down", value: 5},
        //                             {text: "DPad Left", value: 6},
        //                             {text: "DPad Right", value: 7},
        //                             {text: "Start", value: 8},
        //                             {text: "Select", value: 9},
        //                             {text: "Misc 1", value: 10},
        //                             {text: "Misc 2", value: 11},
        //                             {text: "Misc 3", value: 12},
        //                             {text: "Misc 4", value: 13},
        //                             {text: "Misc 5", value: 14},
        //                             {text: "Misc 6", value: 15},
        //                             {text: "Misc 7", value: 16},
        //                             {text: "R1", value: 17},
        //                             {text: "R2", value: 18},
        //                             {text: "R3", value: 19},
        //                             {text: "L1", value: 20},
        //                             {text: "L2", value: 21},
        //                             {text: "L3", value: 22},
        //                             {text: "Left Stick Up", value: 23},
        //                             {text: "Left Stick Down", value: 24},
        //                             {text: "Left Stick Left", value: 25},
        //                             {text: "Left Stick Right", value: 26},
        //                             {text: "Right Stick Up", value: 27},
        //                             {text: "Right Stick Down", value: 28},
        //                             {text: "Right Stick Left", value: 29},
        //                             {text: "Right Stick Right", value: 30}
        //                         ]
        //
        //                         // currentIndex: 0
        //                         // onCurrentIndexChanged: {
        //                         //     controller_manager.setPlatformIndex(currentIndex)
        //                         // }
        //                     }
        //
        //                 }
        //             }
        //         }
        //     }
        // }


    }


}