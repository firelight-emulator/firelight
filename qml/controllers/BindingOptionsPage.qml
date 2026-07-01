import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

// Standalone editor for the bindings attached to a single emulated button.
// Lets the user add alternate physical bindings and toggle turbo/autofire and
// press-to-latch (toggle) per binding. Bind profileId/platformId/
// controllerTypeId/targetInput plus a human-readable targetLabel.
FocusScope {
    id: root

    property int profileId: -1
    property int platformId: -1
    property int controllerTypeId: 1
    property int targetInput: -1
    property string targetLabel: "Button"
    // Which player's controller to read when capturing a new binding.
    property int capturePlayerNumber: 1

    property bool capturing: false

    BindingListModel {
        id: bindings
        profileId: root.profileId
        platformId: root.platformId
        controllerTypeId: root.controllerTypeId
        targetInput: root.targetInput
    }

    GamepadStatus {
        id: gamepad
        playerNumber: root.capturePlayerNumber
    }

    Connections {
        target: gamepad
        enabled: root.capturing
        function onInputChanged(input, pressed) {
            if (pressed) {
                bindings.addBinding(input);
                root.capturing = false;
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        Text {
            text: "Bindings for " + root.targetLabel
            color: ColorPalette.neutral100
            font.family: Constants.regularFontFamily
            font.weight: Font.DemiBold
            font.pixelSize: 20
        }

        Text {
            visible: bindings.count === 0 && !root.capturing
            text: "No custom bindings — the default mapping is used."
            color: ColorPalette.neutral400
            font.family: Constants.regularFontFamily
            font.pixelSize: 14
        }

        ListView {
            id: list
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 6
            model: bindings

            delegate: Rectangle {
                required property int index
                required property string sourceLabel
                required property bool toggle
                required property bool turboEnabled
                required property real turboRate

                width: ListView.view.width
                implicitHeight: 92
                radius: 4
                color: ColorPalette.neutral800

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 4

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10

                        Text {
                            text: sourceLabel
                            color: ColorPalette.neutral100
                            font.family: Constants.regularFontFamily
                            font.pixelSize: 16
                            Layout.fillWidth: true
                        }

                        Text {
                            text: "Toggle"
                            color: ColorPalette.neutral300
                            font.family: Constants.regularFontFamily
                            font.pixelSize: 13
                        }
                        Switch {
                            checked: toggle
                            onToggled: bindings.setToggle(index, checked)
                        }

                        Text {
                            text: "Turbo"
                            color: ColorPalette.neutral300
                            font.family: Constants.regularFontFamily
                            font.pixelSize: 13
                        }
                        Switch {
                            checked: turboEnabled
                            onToggled: bindings.setTurboEnabled(index, checked)
                        }

                        Button {
                            text: "Remove"
                            onClicked: bindings.removeBinding(index)
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        visible: turboEnabled
                        spacing: 8

                        Text {
                            text: "Rate"
                            color: ColorPalette.neutral300
                            font.family: Constants.regularFontFamily
                            font.pixelSize: 13
                        }
                        Slider {
                            id: rateSlider
                            Layout.fillWidth: true
                            from: 1
                            to: 30
                            stepSize: 1
                            value: turboRate
                            onMoved: bindings.setTurboRate(index, rateSlider.value)
                        }
                        Text {
                            text: Math.round(turboRate) + " Hz"
                            color: ColorPalette.neutral300
                            font.family: Constants.regularFontFamily
                            font.pixelSize: 13
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Text {
                visible: root.capturing
                text: "Press a button on player " + root.capturePlayerNumber + "…"
                color: "#d14c20"
                font.family: Constants.regularFontFamily
                font.pixelSize: 15
                Layout.fillWidth: true
            }
            Item {
                visible: !root.capturing
                Layout.fillWidth: true
            }
            Button {
                text: root.capturing ? "Cancel" : "Add alternate binding"
                onClicked: root.capturing = !root.capturing
            }
        }
    }
}
