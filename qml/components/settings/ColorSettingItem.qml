import QtQuick
import QtQuick.Layouts

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
        return Theme.folderColors.concat(["#ffffff"]);
    }

    control: RowLayout {
        spacing: AppStyle.spacingSm

        Repeater {
            model: root.swatches
            delegate: Rectangle {
                id: swatchCell
                required property string modelData
                width: AppStyle.iconSizeMd + AppStyle.spacingSm
                height: AppStyle.iconSizeMd + AppStyle.spacingSm
                radius: width / 2
                color: swatchHover.hovered ? Theme.surfaceHover : "transparent"

                Rectangle {
                    anchors.centerIn: parent
                    width: AppStyle.iconSizeMd
                    height: AppStyle.iconSizeMd
                    radius: width / 2
                    color: swatchCell.modelData
                    border.width: root.value.toLowerCase() === swatchCell.modelData.toLowerCase() ? 3 : 1
                    border.color: root.value.toLowerCase() === swatchCell.modelData.toLowerCase() ? Theme.focusRing : Theme.border
                }

                HoverHandler {
                    id: swatchHover
                    enabled: root.enabled
                    cursorShape: Qt.PointingHandCursor
                }

                TapHandler {
                    enabled: root.enabled
                    onTapped: root.picked(swatchCell.modelData)
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
