import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import Firelight 1.0

FocusScope {
    id: root

    required property GamepadStatus gamepad

    required property var platformId
    required property var profileId
    property var controllerType: 1

    required property var platformMetadataModel
    property var inputMappingsModel: InputMappingsModel {
        profileId: root.profileId
        platformId: root.platformId
        controllerTypeId: root.controllerType
    }

    FirelightDialog {
        id: confirmDialog

        property var buttonList: []
        property var currentIndex: 0

        text: "You're about to walk through assigning an input on your controller to each " + platformMetadataModel.display_name + " input.\n\n Continue?"
        showButtons: true

        onAccepted: function () {
            dialog.buttons = []
            for (var i = 0; i < buttonList.count; ++i) {
                dialog.buttons.push({
                    display_name: buttonList.model.index(i, 0).data(258),
                    retropad_button: buttonList.model.index(i, 0).data(257)
                })
            }
            // dialog.buttons = platformMetadataModel.buttons
            dialog.currentIndex = 0
            dialog.open()
        }
    }

    FirelightDialog {
            id: resetAllDialog

            text: "You're about to walk through assigning an input on your controller to each " + platformMetadataModel.display_name + " input.\n\n Continue?"
            showButtons: true

            onAccepted: function () {
                inputMappingsModel.resetAllToDefault()
            }
        }

    InputPromptDialog {
        id: dialog
        // imageSourceUrl: platformList.currentItem.model.icon_url
        imageSourceUrl: platformMetadataModel.controller_images[root.controllerType - 1]
        platformName: platformMetadataModel.display_name
        gamepad: root.gamepad

        onMappingAdded: function(original, mapped) {
            inputMappingsModel.setMapping(original, mapped)
        }
    }

    ListView {
        id: buttonList
        anchors.fill: parent
        anchors.rightMargin: 8
        // clip: true
        focus: true


        // highlightFollowsCurrentItem: true
        keyNavigationEnabled: true
        highlightMoveDuration: 80
        highlightMoveVelocity: -1
        highlightRangeMode: InputMethodManager.usingMouse ? ListView.NoHighlightRange : ListView.ApplyRange
        preferredHighlightBegin: 64
        preferredHighlightEnd: height - 64

        ScrollBar.vertical: ScrollBar {
        }

        spacing: 4

        header: ColumnLayout {
            id: theHeader
            width: ListView.view.width
            spacing: 16
            // height: 200

            TabBar {
                background: Rectangle {
                    color: ColorPalette.neutral800
                    opacity: 0.4
                    radius: 8
                    border.color: ColorPalette.neutral500
                }
                Layout.alignment: Qt.AlignHCenter
                Repeater {
                    model: root.platformMetadataModel.num_controller_types
                    delegate: TabButton {
                        required property var modelData

                        background: Rectangle {
                            color: root.controllerType === modelData + 1 ? ColorPalette.neutral700 : ColorPalette.neutral800
                            opacity: 0.5
                            topLeftRadius: modelData === 0 ? 8 : 0
                            topRightRadius: modelData === root.platformMetadataModel.num_controller_types - 1 ? 8 : 0
                            bottomLeftRadius: modelData === 0 ? 8 : 0
                            bottomRightRadius: modelData === root.platformMetadataModel.num_controller_types - 1 ? 8 : 0
                        }

                        contentItem: Text {
                            text: root.platformMetadataModel.controller_type_names[modelData]
                            color: "white"
                            font.pixelSize: 15
                            font.family: Constants.regularFontFamily
                            font.weight: Font.DemiBold
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            leftPadding: 16
                            rightPadding: 16
                            topPadding: 4
                            bottomPadding: 4
                        }

                        onClicked: {
                            root.controllerType = modelData + 1
                        }
                    }
                }
            }

            // RowLayout {
            //     Layout.fillHeight: true
            //     spacing: 16
            //     focus: true
            //     Item {
            //         Layout.fillHeight: true
            //         Layout.fillWidth: true
            //     }
            //
            //     Repeater {
            //         model: root.platformMetadataModel.num_controller_types
            //         delegate: FirelightButton {
            //             id: controllerTypeButton
            //             required property var modelData
            //             label: modelData
            //             onClicked: {
            //                 root.controllerType = modelData + 1
            //             }
            //         }
            //     }
            //     Item {
            //         Layout.fillHeight: true
            //         Layout.fillWidth: true
            //     }
            // }

            Image {
                id: imagey
                Layout.maximumHeight: 360
                Layout.maximumWidth: 360
                // Layout.preferredHeight: 420
                // Layout.preferredWidth: 420
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                // source: root.platformMetadataModel.icon_url
                source: platformMetadataModel.controller_images[root.controllerType - 1]
                sourceSize.height: 360
                mipmap: true
                fillMode: Image.PreserveAspectFit
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 16
                focus: true
                Item {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                }

                FirelightButton {
                    id: assignAllButton
                    label: "Assign all"
                    Layout.alignment: Qt.AlignLeft | Qt.AlignBottom
                    focus: true

                    onClicked: function () {
                        confirmDialog.open()
                    }
                }

                FirelightButton {
                    id: clearButton
                    label: "Reset all to default"
                    Layout.alignment: Qt.AlignLeft | Qt.AlignBottom

                    onClicked: function () {
                        resetAllDialog.open()
                    }
                }
            }
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 16
            }
        }

        model: inputMappingsModel
        delegate: Button {
            id: myDelegate
            required property var model
            required property var index
            property bool showGlobalCursor: true
            height: 60
            width: ListView.view.width
            hoverEnabled: true
            background: Rectangle {
                color: ColorPalette.neutral300
                radius: 8
                border.color: ColorPalette.neutral500
                opacity: parent.hovered || (!InputMethodManager.usingMouse && parent.activeFocus) ? 0.14 : 0

                Behavior on opacity {
                    NumberAnimation {
                        duration: 64
                        easing.type: Easing.InOutQuad
                    }
                }
            }
            ContextMenu.menu: RightClickMenu {
                RightClickMenuItem {
                    text: "Assign"

                    onTriggered: {
                      dialog.buttons = []
                      dialog.buttons = [{
                          display_name: model.originalInputName,
                          retropad_button: model.originalInput
                      }]
                      dialog.open()
                    }
                }

                RightClickMenuItem {
                    text: "Reset to default"

                    onTriggered: {
                      inputMappingsModel.resetToDefault(model.originalInput)
                    }
                }

                RightClickMenuItem {
                    text: "Clear mapping"
                    onTriggered: {
                          inputMappingsModel.clearMapping(model.originalInput)
                    }
                }
            }
            onClicked: function() {
                ListView.view.currentIndex = index;
            }
            onDoubleClicked: function() {
                dialog.buttons = []
                dialog.buttons = [{
                    display_name: model.originalInputName,
                    retropad_button: model.originalInput
                }]
                dialog.open()
            }
            contentItem: RowLayout {
                spacing: 12
                Item {
                    Layout.fillHeight: true
                    Layout.preferredWidth: 16
                }
                Text {
                    Layout.preferredWidth: 240
                    Layout.maximumWidth: 240
                    Layout.alignment: Qt.AlignLeft
                    Layout.fillHeight: true
                    text: model.originalInputName
                    color: "white"
                    font.pixelSize: 15
                    font.family: Constants.regularFontFamily
                    font.weight: Font.DemiBold
                    verticalAlignment: Text.AlignVCenter
                }

                Row {
                    Layout.preferredWidth: 240
                    Layout.maximumWidth: 240
                    Layout.alignment: Qt.AlignLeft
                    Layout.fillHeight: true

                    Text {
                        height: parent.height
                        // text: inputMapping.inputMappings[modelData.retropad_button] === undefined ? (gamepadStatus.inputLabels[modelData.retropad_button] + " (default)") : gamepadStatus.inputLabels[inputMapping.inputMappings[modelData.retropad_button]]
                        // color: inputMapping.inputMappings[modelData.retropad_button] === undefined ? ColorPalette.neutral400 : "white"
                        font.pixelSize: 15
                        color: model.hasConflict ? "yellow" : (!model.hasMapping ? ColorPalette.neutral400 : "white")
                        text: model.hasMapping ? model.mappedInputName : "(Not mapped)"

                        font.family: Constants.regularFontFamily
                        font.weight: !model.hasMapping ? Font.Medium : Font.DemiBold
                        verticalAlignment: Text.AlignVCenter
                    }

                    Text {
                        height: parent.height
                        visible: model.isDefault
                        font.pixelSize: 15
                        color: ColorPalette.neutral400
                        text: " (Default)"
                        font.family: Constants.regularFontFamily
                        font.weight: Font.Medium
                        verticalAlignment: Text.AlignVCenter
                    }
                }


                Item {
                    Layout.preferredHeight: 32
                    Layout.preferredWidth: 32
                    Layout.alignment: Qt.AlignVCenter

                    FLIcon {
                        icon: "bar-chart"
                        color: "yellow"
                        anchors.fill: parent
                        size: height
                        visible: model.hasConflict
                    }
                }

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }

                // Item {
                //     Layout.preferredHeight: parent.height
                //     Layout.preferredWidth: parent.height
                //     Layout.alignment: Qt.AlignRight
                //
                //     FirelightButton {
                //         id: dotsButton
                //         anchors.centerIn: parent
                //         circle: true
                //         flat: true
                //         iconCode: "\ue5d4"
                //         visible: myDelegate.hovered || (!InputMethodManager.usingMouse && myDelegate.activeFocus)
                //
                //         onClicked: function () {
                //             myDelegate.ContextMenu.menu.popup(dotsButton, 0, 0)
                //             console.log("hi")
                //         }
                //     }
                // }




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

    RowLayout {
        anchors.fill: parent
        spacing: 16
        visible: false
        //
        // Item {
        //     Layout.fillWidth: true
        //     Layout.fillHeight: true
        //     Layout.horizontalStretchFactor: 1
        // }

        // ColumnLayout {
        //     Layout.fillHeight: true
        //     Layout.leftMargin: 24
        //     Layout.rightMargin: 24
        //     Layout.preferredWidth: 300
        //     Layout.preferredHeight: 300
        //
        //     Image {
        //         id: imagey
        //         Layout.fillWidth: true
        //         // Layout.preferredHeight: 420
        //         // Layout.preferredWidth: 420
        //         Layout.alignment: Qt.AlignTop | Qt.AlignLeft
        //         // source: root.platformMetadataModel.icon_url
        //         source: "file:///C:/Users/alexs/Downloads/Full Color Controllers PNGs/PNGs/Nintendo/Nintendo NES Controller 600dpi.png"
        //         sourceSize.height: 600
        //         mipmap: true
        //         fillMode: Image.PreserveAspectFit
        //     }
        //
        //     FirelightButton {
        //         id: assignAllButton
        //         label: "Assign all"
        //         Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
        //
        //         onClicked: function () {
        //             confirmDialog.open()
        //         }
        //     }
        //
        //     FirelightButton {
        //         id: clearButton
        //         label: "Reset all to default"
        //         Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
        //
        //         onClicked: function () {
        //             confirmDialog.open()
        //         }
        //     }
        // }
        //
        // Item {
        //     Layout.fillWidth: true
        //     Layout.fillHeight: true
        //     Layout.horizontalStretchFactor: 1
        // }


    }
}