import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import Firelight 1.0

FocusScope {
    id: root

    // property alias controllerProfileId: gamepadProfile.profileId
    property var currentMapping: null
    required property var playerNumber
    focus: true

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
                text: "Editing controller profile (" + gamepadStatus.name + ")"
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
                focus: true

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




}