import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

// Color setting: a row of preset swatches plus a hex entry for arbitrary
// colors. `value` is a hex string (e.g. "#ff8800"); `presets` is an optional
// list of {label, value} swatches (from the setting's `options`).
BaseSettingItem {
    id: root

    controlBelow: true

    property string value: ""
    property var presets: []
    signal picked(string hex)

    // Fall back to a default palette when the setting declares no swatches.
    readonly property var swatches: {
        if (presets && presets.length > 0) {
            var out = [];
            for (var i = 0; i < presets.length; i++) {
                out.push(presets[i].value);
            }
            return out;
        }
        return ["#e5484d", "#f76b15", "#f5d90a", "#46a758", "#0091ff", "#8e4ec6", "#e93d82", "#ffffff"];
    }

    control: RowLayout {
        spacing: 6

        Repeater {
            model: root.swatches
            delegate: Rectangle {
                required property string modelData
                width: 24
                height: 24
                radius: 12
                color: modelData
                border.width: root.value.toLowerCase() === modelData.toLowerCase() ? 3 : 1
                border.color: root.value.toLowerCase() === modelData.toLowerCase()
                    ? "white" : "#00000040"

                TapHandler {
                    enabled: root.enabled
                    onTapped: root.picked(parent.modelData)
                }
            }
        }

        TextField {
            id: hexField
            Layout.leftMargin: 4
            implicitWidth: 90
            enabled: root.enabled
            placeholderText: "#rrggbb"
            color: Theme.textPrimary
            font.family: Constants.regularFontFamily
            font.pixelSize: AppStyle.fontSizeSmall
            inputMask: "\\#HHHHHH"

            background: Rectangle {
                radius: 4
                color: Theme.surface
                border.width: hexField.activeFocus ? 2 : 0
                border.color: Theme.border
            }

            Component.onCompleted: text = root.value
            onEditingFinished: if (text.length === 7) root.picked(text)

            Connections {
                target: root
                function onValueChanged() {
                    if (!hexField.activeFocus) {
                        hexField.text = root.value;
                    }
                }
            }
        }
    }
}
