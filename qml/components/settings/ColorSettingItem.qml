import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

// Color setting: a row of preset swatches plus a hex entry for arbitrary
// colors. `value` is a hex string (e.g. "#ff8800"); `presets` is an optional
// list of {label, value} swatches (from the setting's `options`)
BaseSettingItem {
    id: root

    controlBelow: false

    property string value: ""
    property var presets: []
    signal picked(string hex)

    // Fall back to a default palette when the setting declares no swatches
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
                width: AppStyle.iconSizeMd
                height: AppStyle.iconSizeMd
                radius: width / 2
                color: modelData
                border.width: root.value.toLowerCase() === modelData.toLowerCase() ? 3 : 1
                border.color: root.value.toLowerCase() === modelData.toLowerCase() ? "white" : "#00000040"

                TapHandler {
                    enabled: root.enabled
                    onTapped: root.picked(parent.modelData)
                }
            }
        }

        FLTextField {
            id: hexField
            size: "sm"
            Layout.leftMargin: AppStyle.spacingXs
            implicitWidth: Math.round(90 * AppStyle.scale)
            enabled: root.enabled
            placeholderText: "#rrggbb"
            inputMask: "\\#HHHHHH"

            Component.onCompleted: text = root.value
            onEditingFinished: if (text.length === 7)
                root.picked(text)

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
