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

    InputPromptDialog {
        id: dialog
        // imageSourceUrl: platformList.currentItem.model.icon_url
        imageSourceUrl: "file:///C:/Users/alexs/Downloads/Full Color Controllers PNGs/PNGs/Nintendo/Nintendo Gameboy 600dpi.png"
        platformName: platformMetadataModel.display_name
    }

    ListView {
        id: buttonList
        anchors.fill: parent
        anchors.rightMargin: 8
        // clip: true
        focus: true


        highlightFollowsCurrentItem: false
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

            Image {
                id: imagey
                Layout.maximumHeight: 240
                // Layout.preferredHeight: 420
                // Layout.preferredWidth: 420
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                // source: root.platformMetadataModel.icon_url
                source: "file:///C:/Users/alexs/Downloads/Full Color Controllers PNGs/PNGs/Nintendo/Nintendo Gameboy 600dpi.png"
                sourceSize.height: 320
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
                        confirmDialog.open()
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

                Text {
                    Layout.preferredWidth: 240
                    Layout.maximumWidth: 240
                    Layout.alignment: Qt.AlignLeft
                    Layout.fillHeight: true
                    // text: inputMapping.inputMappings[modelData.retropad_button] === undefined ? (gamepadStatus.inputLabels[modelData.retropad_button] + " (default)") : gamepadStatus.inputLabels[inputMapping.inputMappings[modelData.retropad_button]]
                    // color: inputMapping.inputMappings[modelData.retropad_button] === undefined ? ColorPalette.neutral400 : "white"
                    font.pixelSize: 15
                    color: model.hasConflict ? "yellow" : (model.isDefault ? ColorPalette.neutral400 : "white")
                    text: model.mappedInputName + (model.isDefault ? " (default)" : "")

                    font.family: Constants.regularFontFamily
                    font.weight: Font.DemiBold
                    verticalAlignment: Text.AlignVCenter
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

                Item {
                    Layout.preferredHeight: parent.height
                    Layout.preferredWidth: parent.height
                    Layout.alignment: Qt.AlignRight

                    FirelightButton {
                        id: dotsButton
                        anchors.centerIn: parent
                        circle: true
                        flat: true
                        iconCode: "\ue5d4"
                        visible: myDelegate.hovered || (!InputMethodManager.usingMouse && myDelegate.activeFocus)

                        onClicked: function () {
                            console.log("hi")
                        }
                    }
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