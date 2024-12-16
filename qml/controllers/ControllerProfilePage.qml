import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import Firelight 1.0

Item {
    id: root

    // property alias controllerProfileId: gamepadProfile.profileId
    property var currentMapping: null

    // GamepadProfile {
    //     id: gamepadProfile
    //
    //     currentPlatformId: platformList.currentItem.model.platform_id
    //
    //     onCurrentPlatformIdChanged: {
    //         root.currentMapping = gamepadProfile.getCurrentMapping()
    //         if (root.currentMapping) {
    //             console.log("currentMapping: " + JSON.stringify(root.currentMapping))
    //         } else {
    //             console.log("currentMapping: null")
    //         }
    //     }
    // }

    InputMapping {
        id: inputMapping
        profileId: 0
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
                    console.log(JSON.stringify(dialog.buttons[dialog.currentIndex]))
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

            // function onButtonStateChanged(player, button, pressed) {
            //     if (!dialog.visible) {
            //         return
            //     }
            //     if (pressed) {
            //         inputMapping.setButtonMapping(dialog.buttons[dialog.currentIndex].retropad_button, button)
            //         if (dialog.buttons.length > dialog.currentIndex + 1) {
            //             dialog.currentIndex++
            //             timer.stop()
            //             frameAnimation.reset()
            //             timer.restart()
            //
            //         } else {
            //             dialog.accept()
            //             // dialog.close()
            //             // saveMapping()
            //         }
            //     }
            // }

            //
            // function onAxisStateChanged(player, axis, value) {
            //     if (!dialog.visible) {
            //         return
            //     }
            //     if (value < 8192 && value > -8192) {
            //         dialog.canAcceptAxisInput = true
            //         return
            //     }
            //     if (dialog.canAcceptAxisInput && !axisDebounceTimer.running) {
            //         dialog.canAcceptAxisInput = false
            //         axisDebounceTimer.restart()
            //         inputMapping.setAxisMapping(dialog.buttons[dialog.currentIndex].retropad_button, axis, value > 0)
            //         if (dialog.buttons.length > dialog.currentIndex + 1) {
            //             dialog.currentIndex++
            //             timer.stop()
            //             frameAnimation.reset()
            //             timer.restart()
            //
            //         } else {
            //             dialog.accept()
            //             // dialog.close()
            //             // saveMapping()
            //         }
            //
            //     }
            //     // inputMapping.setButtonMapping(dialog.buttons[dialog.currentIndex].retropad_button, axis)
            //
            // }
        }

        TapHandler {
            onTapped: function (event, button) {
                console.log(dialog.buttons.length)
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

        contentItem: ColumnLayout {
            spacing: 12


            Keys.onPressed: function (event) {
                event.accept = false
            }

            Keys.onReleased: function (event) {
                event.accept = false
            }

            // View3D {
            //     Layout.preferredHeight: 300
            //     Layout.preferredWidth: 300
            //     Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            //
            //     RuntimeLoader {
            //         id: importNode
            //         source: "file:system/nes.obj"
            //     }
            //
            //     environment: SceneEnvironment {
            //         backgroundMode: SceneEnvironment.Transparent
            //     }
            //
            //     camera: activeCamera
            //     PerspectiveCamera {
            //         id: activeCamera
            //         z: 400
            //     }
            //
            //     DirectionalLight {
            //         color: "white"
            //     }
            //
            //     // Model {
            //     //     x: -100
            //     //     source: "#Cube"
            //     //     materials: PrincipledMaterial {
            //     //         baseColor: "red"
            //     //     }
            //     // }
            //     // Model {
            //     //     x: 100
            //     //     source: "#Sphere"
            //     //     materials: PrincipledMaterial {
            //     //         baseColor: "green"
            //     //     }
            //     // }
            // }

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

                onClicked: function () {
                    confirmDialog.open()
                }
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

                    Text {
                        Layout.preferredWidth: 240
                        Layout.maximumWidth: 240
                        Layout.alignment: Qt.AlignLeft
                        Layout.fillHeight: true
                        text: inputMapping.inputMappings[modelData.retropad_button] === undefined ? "Nothing assigned" : inputMapping.inputMappings[modelData.retropad_button]
                        color: inputMapping.inputMappings[modelData.retropad_button] === undefined ? ColorPalette.neutral400 : "white"
                        font.pixelSize: 16
                        font.family: Constants.regularFontFamily
                        font.weight: Font.DemiBold
                        verticalAlignment: Text.AlignVCenter
                    }
                    // MyComboBox {
                    //     id: dropdown
                    //     Layout.preferredWidth: 240
                    //     Layout.maximumWidth: 240
                    //     Layout.alignment: Qt.AlignLeft
                    //     Layout.fillHeight: true
                    //     textRole: "text"
                    //     valueRole: "value"
                    //     currentIndex: modelData.retropad_button
                    //     model: [
                    //         {text: "Y", value: 0},
                    //         {text: "A", value: 1},
                    //         {text: "B", value: 2},
                    //         {text: "X", value: 3},
                    //         {text: "DPad Up", value: 4},
                    //         {text: "DPad Down", value: 5},
                    //         {text: "DPad Left", value: 6},
                    //         {text: "DPad Right", value: 7},
                    //         {text: "Start", value: 8},
                    //         {text: "Select", value: 9},
                    //         {text: "Misc 1", value: 10},
                    //         {text: "Misc 2", value: 11},
                    //         {text: "Misc 3", value: 12},
                    //         {text: "Misc 4", value: 13},
                    //         {text: "Misc 5", value: 14},
                    //         {text: "Misc 6", value: 15},
                    //         {text: "Misc 7", value: 16},
                    //         {text: "R1", value: 17},
                    //         {text: "R2", value: 18},
                    //         {text: "R3", value: 19},
                    //         {text: "L1", value: 20},
                    //         {text: "L2", value: 21},
                    //         {text: "L3", value: 22},
                    //         {text: "Left Stick Up", value: 23},
                    //         {text: "Left Stick Down", value: 24},
                    //         {text: "Left Stick Left", value: 25},
                    //         {text: "Left Stick Right", value: 26},
                    //         {text: "Right Stick Up", value: 27},
                    //         {text: "Right Stick Down", value: 28},
                    //         {text: "Right Stick Left", value: 29},
                    //         {text: "Right Stick Right", value: 30}
                    //     ]
                    //
                    //     // currentIndex: 0
                    //     // onCurrentIndexChanged: {
                    //     //     controller_manager.setPlatformIndex(currentIndex)
                    //     // }
                    // }
                    FirelightButton {
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        tooltipLabel: "Assign"
                        flat: true

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

                    Item {
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                    }

                }

            }
        }

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.horizontalStretchFactor: 1
        }
    }


}