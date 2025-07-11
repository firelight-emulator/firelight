import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import Firelight 1.0

FocusScope {
    id: root

    // property alias controllerProfileId: gamepadProfile.profileId
    required property var playerNumber

    GamepadStatus {
        id: gamepadStatus
        playerNumber: root.playerNumber
    }

    InputMapping {
        id: inputMapping
        profileId: gamepadStatus.profileId
        platformId: platformList.currentItem.model.platform_id
    }

    PlatformMetadata {
        id: platformMetadata
        platformId: platformList.currentItem.model.platform_id
    }

    FirelightDialog {
        id: confirmDialog

        property var buttonList: []
        property var currentIndex: 0

        text: "You're about to walk through assigning an input on your controller to each " + platformList.currentItem.model.display_name + " input.\n\n Continue?"
        showButtons: true

        onAccepted: function () {
            dialog.buttons = platformList.currentItem.model.buttons
            dialog.currentIndex = 0
            dialog.open()
        }
    }

    // Turn into component
    // also use stackview
    FirelightDialog {
        id: dialog

        property variant buttons: []
        property var currentIndex: 0
        property bool canAcceptAxisInput: true
        // title: "Assign all buttons"
        // message: "Are you sure you want to assign all buttons to the default mappings?"
        showButtons: false
        closePolicy: Popup.NoAutoClose

        onAboutToShow: {
            controller_manager.blockGamepadInput = true
            currentIndex = 0
            frameAnimation.reset()
        }

        onOpened: function () {
            // theBar.widthThing = parent.width
            timer.start()
        }

        onClosed: function () {
            timer.stop()
            controller_manager.blockGamepadInput = false
            dialog.buttons = []
        }

        Timer {
            id: axisDebounceTimer
            interval: 300
            running: false
            repeat: false
        }

        Connections {
            target: controller_manager

            function onRetropadInputStateChanged(player, input, activated) {
                if (!dialog.visible) {
                    return
                }
                if (activated && !axisDebounceTimer.running) {
                    axisDebounceTimer.restart()
                    inputMapping.addMapping(dialog.buttons[dialog.currentIndex].retropad_button, input)
                    if (dialog.buttons.length > dialog.currentIndex + 1) {
                        dialog.currentIndex++
                        timer.stop()
                        frameAnimation.reset()
                        timer.restart()

                    } else {
                        dialog.accept()
                        // dialog.close()
                        // saveMapping()
                    }
                }
            }
        }

        contentItem: ColumnLayout {
            spacing: 12

            Keys.onPressed: function (event) {
                event.accept = false
            }

            Keys.onReleased: function (event) {
                event.accept = false
            }

            Image {
                Layout.preferredHeight: 300
                Layout.preferredWidth: 300
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                source: platformList.currentItem.model.icon_url
                sourceSize.height: 512
                fillMode: Image.PreserveAspectFit
            }
            Text {
                text: "Press a button on your controller to assign it to this " + platformList.currentItem.model.display_name + " input:"
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
                text: dialog.buttons.length > 0 && dialog.currentIndex < dialog.buttons.length ? dialog.buttons[dialog.currentIndex].display_name : "Nothing"
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
                Layout.topMargin: 8
                Layout.preferredHeight: 10
                color: "green"
            }


        }
    }

    ButtonGroup {
        id: menuButtonGroup
        exclusive: true
    }

    Pane {
        id: menuPane
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 300

        // background: Rectangle {
        //     color: ColorPalette.neutral800
        // }
        background: Item {}

        contentItem: ColumnLayout {
            FirelightMenuItem {
                 focus: true

                 labelText: "Details"
                 property bool showGlobalCursor: true
                 // labelIcon: "\ue40a"
                 Layout.preferredHeight: 50
                 Layout.fillWidth: true
                 checked: true

                 ButtonGroup.group: menuButtonGroup

                 // onToggled: {
                 //     if (checked) {
                 //         ListView.view.currentIndex = index
                 //         col.forceActiveFocus()
                 //     }
                 // }
             }
            FirelightMenuItem {
                 labelText: "Shortcuts"
                 property bool showGlobalCursor: true
                 // labelIcon: "\ue40a"
                 Layout.preferredHeight: 50
                 Layout.fillWidth: true

                 ButtonGroup.group: menuButtonGroup

                 // onToggled: {
                 //     if (checked) {
                 //         ListView.view.currentIndex = index
                 //         col.forceActiveFocus()
                 //     }
                 // }
             }
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: ColorPalette.neutral600
            }
            ListView {
                 id: platformList
                 spacing: 0
                 Layout.fillHeight: true
                 Layout.fillWidth: true
                 // Layout.maximumWidth: 300
                 // Layout.preferredWidth: 300
                 // Layout.minimumWidth: 200
                 // Layout.fillHeight: true
                 // Layout.alignment: Qt.AlignRight
                 focus: true
                 clip: true

                 KeyNavigation.right: buttonList

                 Keys.onBackPressed: {
                     root.StackView.view.popCurrentItem(StackView.PopTransition)
                 }

                 Keys.onEscapePressed: {
                     root.StackView.view.popCurrentItem(StackView.PopTransition)
                 }

                 currentIndex: 0

                 // onCurrentItemChanged: {
                 //     gamepadProfile.currentPlatformId = platformList.currentItem.model.platform_id
                 // }

                 model: platform_model
                 delegate: FirelightMenuItem {
                     required property var model
                     required property var index
                     // focus: true

                    ButtonGroup.group: menuButtonGroup

                     labelText: model.display_name
                     labelIcon: model.icon_name
                     property bool showGlobalCursor: true
                     // labelIcon: "\ue40a"
                     height: 54
                     width: ListView.view.width

                     // checked: ListView.isCurrentItem

                     onToggled: {
                         if (checked) {
                             ListView.view.currentIndex = index
                             col.forceActiveFocus()
                         }
                     }
                 }
             }
         }
    }

    ControllerInputMappingView {
         inputMapping: inputMapping
         platformMetadataModel: platformList.currentItem.model

        anchors.left: menuPane.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.leftMargin: 16
    }



    RowLayout {
        anchors.left: menuPane.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.leftMargin: 40
        anchors.rightMargin: 40
        visible: false

        // Item {
        //     Layout.fillHeight: true
        //     Layout.fillWidth: true
        //     Layout.horizontalStretchFactor: 1
        // }



        Image {
            id: imagey
            Layout.preferredHeight: 500
            Layout.preferredWidth: 500
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            source: platformList.currentItem.model.icon_url
            sourceSize.height: 500
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


            FirelightButton {
                id: assignAllButton
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.rightMargin: 12
                anchors.bottomMargin: 12
                label: "Assign all"

                KeyNavigation.left: platformList

                onClicked: function () {
                    confirmDialog.open()
                }
            }
        }

        ListView {
            id: buttonList
            Layout.fillHeight: true
            Layout.fillWidth: true
            // Layout.alignment: Qt.AlignHCenter
            clip: true
            focus: true

            keyNavigationEnabled: true

            KeyNavigation.up: assignAllButton

            ScrollBar.vertical: ScrollBar {
            }

            visible: platformList.currentItem.model.buttons.length > 0

            spacing: 12

            header: ColumnLayout {
                width: ListView.view.width
                RowLayout {
                    Layout.fillWidth: true
                    Layout.maximumHeight: 40

                    Item {
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        Layout.horizontalStretchFactor: 1
                    }
                    Text {
                        Layout.preferredWidth: buttonList.width / 3
                        Layout.maximumWidth: buttonList.width / 3
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
                        Layout.preferredWidth: buttonList.width / 3
                        Layout.maximumWidth: buttonList.width / 3
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
                        Layout.preferredWidth: height * 2 + 12
                        Layout.maximumWidth: height * 2 + 12
                    }

                    Item {
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        Layout.horizontalStretchFactor: 1
                    }
                }
                Rectangle {
                    Layout.minimumHeight: 1
                    Layout.maximumHeight: 1
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignHCenter
                    color: ColorPalette.neutral600
                }
            }

            footer: Item {
                width: ListView.view.width
                height: 24
            }

            model: platformList.currentItem.model.buttons
            delegate: FocusScope {
                height: 48
                width: ListView.view.width
                RowLayout {
                    anchors.fill: parent
                    spacing: 12

                    Item {
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        Layout.horizontalStretchFactor: 1
                    }
                    Text {
                        Layout.preferredWidth: buttonList.width / 3
                        Layout.maximumWidth: buttonList.width / 3
                        Layout.alignment: Qt.AlignLeft
                        Layout.fillHeight: true
                        text: modelData.display_name
                        color: "white"
                        font.pixelSize: 16
                        font.family: Constants.regularFontFamily
                        font.weight: Font.DemiBold
                        verticalAlignment: Text.AlignVCenter
                    }

                    Text {
                        Layout.preferredWidth: buttonList.width / 3
                        Layout.maximumWidth: buttonList.width / 3
                        Layout.alignment: Qt.AlignLeft
                        Layout.fillHeight: true
                        text: inputMapping.inputMappings[modelData.retropad_button] === undefined ? (gamepadStatus.inputLabels[modelData.retropad_button] + " (default)") : gamepadStatus.inputLabels[inputMapping.inputMappings[modelData.retropad_button]]
                        color: inputMapping.inputMappings[modelData.retropad_button] === undefined ? ColorPalette.neutral400 : "white"
                        font.pixelSize: 16
                        font.family: Constants.regularFontFamily
                        font.weight: Font.DemiBold
                        verticalAlignment: Text.AlignVCenter
                    }
                    FirelightButton {
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        focus: true
                        tooltipLabel: "Assign"
                        flat: true

                        KeyNavigation.right: resetButton

                        Layout.preferredHeight: 42
                        Layout.preferredWidth: height
                        Layout.maximumWidth: height

                        iconCode: "\ue3c9"

                        onClicked: {
                            dialog.buttons = []
                            dialog.buttons = [{
                                display_name: modelData.display_name,
                                retropad_button: modelData.retropad_button
                            }]
                            dialog.open()
                        }
                    }

                    FirelightButton {
                        id: resetButton
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        tooltipLabel: "Reset to default"
                        flat: true

                        Layout.preferredHeight: 42
                        Layout.preferredWidth: height
                        Layout.maximumWidth: height

                        iconCode: "\ue5d5"

                        onClicked: {
                            inputMapping.removeMapping(modelData.retropad_button)
                            // dialog.buttons = []
                            // dialog.buttons = [{
                            //     display_name: modelData.display_name,
                            //     retropad_button: modelData.retropad_button
                            // }]
                            // dialog.open()
                        }
                    }

                    Item {
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        Layout.horizontalStretchFactor: 1
                    }

                }
            }

        }

        // RowLayout {
        //     id: col
        //     Layout.fillHeight: true
        //     Layout.preferredWidth: height
        //     Layout.alignment: Qt.AlignLeft
        //     Layout.leftMargin: 12
        //
        //     Keys.onBackPressed: {
        //         platformList.forceActiveFocus()
        //     }
        //
        //     spacing: 8
        //
        //     // Image {
        //     //     id: imagey
        //     //     Layout.preferredHeight: 300
        //     //     Layout.preferredWidth: 300
        //     //     Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
        //     //     source: platformList.currentItem.model.icon_url
        //     //     sourceSize.height: 300
        //     //     mipmap: true                    // sourceSize.height: 512
        //     //     // sourceClipRect: Qt.rect(0, 0, 1024, 1024)
        //     //     fillMode: Image.PreserveAspectFit
        //     //
        //     //     Behavior on width {
        //     //         NumberAnimation {
        //     //             duration: 100
        //     //             easing.type: Easing.InOutQuad
        //     //         }
        //     //     }
        //     //     Behavior on height {
        //     //         NumberAnimation {
        //     //             duration: 100
        //     //             easing.type: Easing.InOutQuad
        //     //         }
        //     //     }
        //     //
        //     //     // Rectangle {
        //     //     //     color: "transparent"
        //     //     //     anchors.fill: parent
        //     //     //
        //     //     //     radius: 4
        //     //     //     border.color: "white"
        //     //     //     border.width: 1
        //     //     // }
        //     // }
        //
        //     FirelightButton {
        //         id: assignAllButton
        //         Layout.alignment: Qt.AlignRight
        //         label: "Assign all"
        //         Layout.bottomMargin: 8
        //
        //         KeyNavigation.left: platformList
        //
        //         onClicked: function () {
        //             confirmDialog.open()
        //         }
        //     }
        //
        //     // RowLayout {
        //     //     Layout.fillWidth: true
        //     //     Layout.alignment: Qt.AlignLeft
        //     //     Layout.maximumHeight: 40
        //     //     Text {
        //     //         Layout.preferredWidth: 240
        //     //         Layout.maximumWidth: 240
        //     //         Layout.alignment: Qt.AlignLeft
        //     //         Layout.fillHeight: true
        //     //         text: platformList.currentItem.model.display_name + " controls"
        //     //         color: ColorPalette.neutral200
        //     //         font.pixelSize: 16
        //     //         font.family: Constants.regularFontFamily
        //     //         font.weight: Font.DemiBold
        //     //         verticalAlignment: Text.AlignVCenter
        //     //     }
        //     //     Text {
        //     //         id: dropdown
        //     //         Layout.preferredWidth: 240
        //     //         Layout.maximumWidth: 240
        //     //         Layout.alignment: Qt.AlignLeft
        //     //         Layout.fillHeight: true
        //     //         Layout.leftMargin: 12
        //     //         text: "Your controls"
        //     //         color: ColorPalette.neutral200
        //     //         font.pixelSize: 16
        //     //         font.family: Constants.regularFontFamily
        //     //         font.weight: Font.DemiBold
        //     //         verticalAlignment: Text.AlignVCenter
        //     //     }
        //     //     Item {
        //     //         Layout.alignment: Qt.AlignLeft
        //     //
        //     //         Layout.preferredHeight: 42
        //     //         Layout.preferredWidth: height
        //     //         Layout.maximumWidth: height
        //     //     }
        //     // }
        //     // Rectangle {
        //     //     Layout.minimumHeight: 1
        //     //     Layout.maximumHeight: 1
        //     //     Layout.fillWidth: true
        //     //     Layout.alignment: Qt.AlignHCenter
        //     //     color: ColorPalette.neutral600
        //     // }
        //
        //     Item {
        //         Layout.fillHeight: true
        //         Layout.fillWidth: true
        //         visible: platformList.currentItem.model.buttons.length === 0
        //     }
        //
        //     // ListView {
        //     //     id: buttonList
        //     //     Layout.fillHeight: true
        //     //     Layout.fillWidth: true
        //     //     Layout.alignment: Qt.AlignHCenter
        //     //     clip: true
        //     //     focus: true
        //     //
        //     //     keyNavigationEnabled: true
        //     //
        //     //     KeyNavigation.up: assignAllButton
        //     //
        //     //     ScrollBar.vertical: ScrollBar {
        //     //     }
        //     //
        //     //     visible: platformList.currentItem.model.buttons.length > 0
        //     //
        //     //     spacing: 12
        //     //
        //     //     footer: Item {
        //     //         width: ListView.view.width
        //     //         height: 24
        //     //     }
        //     //
        //     //     model: platformList.currentItem.model.buttons
        //     //     delegate: FocusScope {
        //     //         height: 48
        //     //         width: ListView.view.width
        //     //         RowLayout {
        //     //             anchors.fill: parent
        //     //             spacing: 12
        //     //             Text {
        //     //                 Layout.preferredWidth: 240
        //     //                 Layout.maximumWidth: 240
        //     //                 Layout.alignment: Qt.AlignLeft
        //     //                 Layout.fillHeight: true
        //     //                 text: modelData.display_name
        //     //                 color: "white"
        //     //                 font.pixelSize: 16
        //     //                 font.family: Constants.regularFontFamily
        //     //                 font.weight: Font.DemiBold
        //     //                 verticalAlignment: Text.AlignVCenter
        //     //             }
        //     //
        //     //             Text {
        //     //                 Layout.preferredWidth: 240
        //     //                 Layout.maximumWidth: 240
        //     //                 Layout.alignment: Qt.AlignLeft
        //     //                 Layout.fillHeight: true
        //     //                 text: inputMapping.inputMappings[modelData.retropad_button] === undefined ? (gamepadStatus.inputLabels[modelData.retropad_button] + " (default)") : gamepadStatus.inputLabels[inputMapping.inputMappings[modelData.retropad_button]]
        //     //                 color: inputMapping.inputMappings[modelData.retropad_button] === undefined ? ColorPalette.neutral400 : "white"
        //     //                 font.pixelSize: 16
        //     //                 font.family: Constants.regularFontFamily
        //     //                 font.weight: Font.DemiBold
        //     //                 verticalAlignment: Text.AlignVCenter
        //     //             }
        //     //             FirelightButton {
        //     //                 Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
        //     //                 focus: true
        //     //                 tooltipLabel: "Assign"
        //     //                 flat: true
        //     //
        //     //                 KeyNavigation.right: resetButton
        //     //
        //     //                 Layout.preferredHeight: 42
        //     //                 Layout.preferredWidth: height
        //     //                 Layout.maximumWidth: height
        //     //
        //     //                 iconCode: "\ue3c9"
        //     //
        //     //                 onClicked: {
        //     //                     dialog.buttons = []
        //     //                     dialog.buttons = [{
        //     //                         display_name: modelData.display_name,
        //     //                         retropad_button: modelData.retropad_button
        //     //                     }]
        //     //                     dialog.open()
        //     //                 }
        //     //             }
        //     //
        //     //             FirelightButton {
        //     //                 id: resetButton
        //     //                 Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
        //     //                 tooltipLabel: "Reset to default"
        //     //                 flat: true
        //     //
        //     //                 Layout.preferredHeight: 42
        //     //                 Layout.preferredWidth: height
        //     //                 Layout.maximumWidth: height
        //     //
        //     //                 iconCode: "\ue5d5"
        //     //
        //     //                 onClicked: {
        //     //                     inputMapping.removeMapping(modelData.retropad_button)
        //     //                     // dialog.buttons = []
        //     //                     // dialog.buttons = [{
        //     //                     //     display_name: modelData.display_name,
        //     //                     //     retropad_button: modelData.retropad_button
        //     //                     // }]
        //     //                     // dialog.open()
        //     //                 }
        //     //             }
        //     //
        //     //             Item {
        //     //                 Layout.fillHeight: true
        //     //                 Layout.fillWidth: true
        //     //             }
        //     //
        //     //         }
        //     //     }
        //     //
        //     // }
        // }

        // Item {
        //     Layout.fillHeight: true
        //     Layout.fillWidth: true
        //     Layout.horizontalStretchFactor: 1
        // }
    }

    Component {
        id: shortcutsList
        ListView {
            model: ListModel {
                ListElement { name: "A"; key: "A" }
                ListElement { name: "B"; key: "B" }
                ListElement { name: "X"; key: "X" }
                ListElement { name: "Y"; key: "Y" }
                ListElement { name: "Start"; key: "Start" }
                ListElement { name: "Select"; key: "Select" }
                ListElement { name: "Left Shoulder"; key: "LeftShoulder" }
                ListElement { name: "Right Shoulder"; key: "RightShoulder" }
                ListElement { name: "Left Stick"; key: "LeftStick" }
                ListElement { name: "Right Stick"; key: "RightStick" }
            }
            delegate: Button {
                required property var model
                text: model.name
                width: parent.width
                height: 40
            }
        }
    }


}