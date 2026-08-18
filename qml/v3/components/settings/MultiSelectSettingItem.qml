import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Multi-select setting: a full-width wrap of toggleable chips over `options`
// ({label, value}). `value` is a JSON array of the selected option values;
// emits `changed` with the new JSON array string when the selection changes
FLMenuItem {
    id: root

    controlBelow: true

    property string value: "[]"
    property var options: []
    signal changed(string jsonValue)

    function parseSelected() {
        try {
            var a = JSON.parse(root.value || "[]");
            return Array.isArray(a) ? a : [];
        } catch (e) {
            return [];
        }
    }

    property var selected: parseSelected()
    onValueChanged: selected = parseSelected()

    function isSelected(v) {
        return selected.indexOf(v) !== -1;
    }

    function toggle(v) {
        var copy = selected.slice();
        var i = copy.indexOf(v);
        if (i === -1) {
            copy.push(v);
        } else {
            copy.splice(i, 1);
        }
        root.changed(JSON.stringify(copy));
    }

    controlItem: Flow {
        spacing: AppStyle.spacingSm

        Repeater {
            model: root.options
            delegate: Button {
                id: chip
                required property var modelData
                readonly property bool on: root.isSelected(modelData.value)

                enabled: root.enabled
                focusPolicy: Qt.NoFocus
                hoverEnabled: true
                padding: 0
                onClicked: root.toggle(modelData.value)

                HoverHandler {
                    cursorShape: Qt.PointingHandCursor
                }

                contentItem: RowLayout {
                    spacing: AppStyle.spacingXs
                    Icon {
                        Layout.leftMargin: AppStyle.spacingMd
                        visible: chip.on
                        name: "check"
                        size: AppStyle.iconSizeSm
                        color: Theme.accent
                    }
                    Text {
                        Layout.leftMargin: chip.on ? 0 : 12
                        Layout.rightMargin: AppStyle.spacingMd
                        text: chip.modelData.label
                        color: chip.on ? Theme.accent : Theme.textMuted
                        font.family: AppStyle.fontFamily
                        font.pixelSize: AppStyle.fontSizeSmall
                        font.weight: Font.DemiBold
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                background: Rectangle {
                    implicitHeight: 30
                    radius: 15
                    color: chip.on ? Theme.blend(Theme.surface, Theme.accent, 0.18) : (chip.hovered ? Theme.surfaceHover : "transparent")
                    border.width: 1
                    border.color: chip.on ? Theme.accent : Theme.border
                }
            }
        }
    }
}
