import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import Firelight 1.0

FocusScope {
    id: root

    required property InputMapping inputMapping
    required property var platformMetadataModel
    property var inputMappingsModel: InputMappingsModel {
        profileId: 2
        platformId: 1
    }

    FirelightDialog {
        id: confirmDialog

        property var buttonList: []
        property var currentIndex: 0

        text: "You're about to walk through assigning an input on your controller to each " + platformMetadataModel.display_name + " input.\n\n Continue?"
        showButtons: true

        onAccepted: function () {
            dialog.buttons = platformMetadataModel.buttons
            dialog.currentIndex = 0
            dialog.open()
        }
    }

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
                source: platformMetadataModel.icon_url
                sourceSize.height: 512
                fillMode: Image.PreserveAspectFit
            }
            Text {
                text: "Press a button on your controller to assign it to this " + platformMetadataModel.display_name + " input:"
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

    RowLayout {
        anchors.fill: parent
        spacing: 16

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.horizontalStretchFactor: 1
        }

        Image {
            id: imagey
            Layout.preferredHeight: 420
            Layout.preferredWidth: 420
            Layout.alignment: Qt.AlignVCenter  | Qt.AlignHCenter
            // source: root.platformMetadataModel.icon_url
            source: "file:///C:/Users/alexs/Downloads/Full Color Controllers PNGs/PNGs/Nintendo/Nintendo NES Controller 600dpi.png"
            sourceSize.height: 420
            mipmap: true
            fillMode: Image.PreserveAspectFit
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.horizontalStretchFactor: 1
        }

        ListView {
            id: buttonList
            Layout.fillHeight: true
            Layout.preferredWidth: 400
            // clip: true
            focus: true

            keyNavigationEnabled: true

            ScrollBar.vertical: ScrollBar {
            }

            spacing: 12

            model: inputMappingsModel
            delegate: FocusScope {
                required property var model
                height: 48
                width: ListView.view.width
                RowLayout {
                    anchors.fill: parent
                    spacing: 12
                    Text {
                        Layout.preferredWidth: buttonList.width / 3
                        Layout.maximumWidth: buttonList.width / 3
                        Layout.alignment: Qt.AlignLeft
                        Layout.fillHeight: true
                        text: model.originalInputName
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
                        // text: inputMapping.inputMappings[modelData.retropad_button] === undefined ? (gamepadStatus.inputLabels[modelData.retropad_button] + " (default)") : gamepadStatus.inputLabels[inputMapping.inputMappings[modelData.retropad_button]]
                        // color: inputMapping.inputMappings[modelData.retropad_button] === undefined ? ColorPalette.neutral400 : "white"
                        font.pixelSize: 16
                        color: model.hasConflict ? "yellow" : (model.isDefault ? ColorPalette.neutral400 : "white")
                        text: model.mappedInputName + (model.isDefault ? " (default)" : "")

                        font.family: Constants.regularFontFamily
                        font.weight: Font.DemiBold
                        verticalAlignment: Text.AlignVCenter
                    }
                    // FirelightButton {
                    //     Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                    //     focus: true
                    //     tooltipLabel: "Assign"
                    //     flat: true
                    //
                    //     KeyNavigation.right: resetButton
                    //
                    //     Layout.preferredHeight: 42
                    //     Layout.preferredWidth: height
                    //     Layout.maximumWidth: height
                    //
                    //     iconCode: "\ue3c9"
                    //
                    //     // onClicked: {
                    //     //     dialog.buttons = []
                    //     //     dialog.buttons = [{
                    //     //         display_name: modelData.display_name,
                    //     //         retropad_button: modelData.retropad_button
                    //     //     }]
                    //     //     dialog.open()
                    //     // }
                    // }
                    //
                    // FirelightButton {
                    //     id: resetButton
                    //     Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                    //     tooltipLabel: "Reset to default"
                    //     flat: true
                    //
                    //     Layout.preferredHeight: 42
                    //     Layout.preferredWidth: height
                    //     Layout.maximumWidth: height
                    //
                    //     iconCode: "\ue5d5"
                    //
                    //     // onClicked: {
                    //     //     inputMapping.removeMapping(modelData.retropad_button)
                    //     //     // dialog.buttons = []
                    //     //     // dialog.buttons = [{
                    //     //     //     display_name: modelData.display_name,
                    //     //     //     retropad_button: modelData.retropad_button
                    //     //     // }]
                    //     //     // dialog.open()
                    //     // }
                    // }
                }
            }
        }
    }
}