import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

// Multi-select setting: a checklist over `options` ({label, value}). `value` is
// a JSON array of the selected option values; emits `changed` with the new JSON
// array string when the selection changes.
BaseSettingItem {
    id: root

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

    function summary() {
        if (selected.length === 0) {
            return qsTr("None");
        }
        var labels = [];
        for (var i = 0; i < options.length; i++) {
            if (isSelected(options[i].value)) {
                labels.push(options[i].label);
            }
        }
        return labels.join(", ");
    }

    control: Button {
        id: btn
        implicitWidth: 260
        enabled: root.enabled
        focusPolicy: Qt.NoFocus
        onClicked: popup.open()

        background: Rectangle {
            radius: 4
            color: Theme.surface
        }

        contentItem: Text {
            text: root.summary()
            leftPadding: 8
            rightPadding: 8
            color: Theme.textPrimary
            elide: Text.ElideRight
            font.family: Constants.regularFontFamily
            font.pixelSize: 14
            verticalAlignment: Text.AlignVCenter
        }

        Popup {
            id: popup
            y: btn.height + 4
            width: Math.max(btn.width, 220)
            padding: 4
            modal: true
            dim: false

            background: Rectangle {
                radius: 6
                color: Theme.surfaceElevated
                border.color: Theme.border
                border.width: 1
            }

            contentItem: ColumnLayout {
                spacing: 0
                Repeater {
                    model: root.options
                    delegate: CheckDelegate {
                        required property var modelData
                        Layout.fillWidth: true
                        padding: 6
                        checked: root.isSelected(modelData.value)
                        onToggled: root.toggle(modelData.value)
                        contentItem: Text {
                            leftPadding: 28
                            text: modelData.label
                            color: Theme.textPrimary
                            font.family: Constants.regularFontFamily
                            font.pixelSize: 14
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }
            }
        }
    }
}
