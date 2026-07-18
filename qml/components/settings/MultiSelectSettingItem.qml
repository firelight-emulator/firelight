import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

// Multi-select setting: a full-width wrap of toggleable chips over `options`
// ({label, value}). `value` is a JSON array of the selected option values;
// emits `changed` with the new JSON array string when the selection changes
BaseSettingItem {
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

    control: Flow {
        spacing: 6

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
                    spacing: 4
                    Icon {
                        Layout.leftMargin: 10
                        visible: chip.on
                        name: "check"
                        size: 14
                        color: Theme.accent
                    }
                    Text {
                        Layout.leftMargin: chip.on ? 0 : 12
                        Layout.rightMargin: 12
                        text: chip.modelData.label
                        color: chip.on ? Theme.accent : Theme.textMuted
                        font.family: Constants.regularFontFamily
                        font.pixelSize: AppStyle.fontSizeSmall
                        font.weight: Font.DemiBold
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                background: Rectangle {
                    implicitHeight: 30
                    radius: 15
                    color: chip.on ? Theme.blend(Theme.surface, Theme.accent, 0.18) : "transparent"
                    border.width: 1
                    border.color: chip.on ? Theme.accent : Theme.border
                }
            }
        }
    }
}
