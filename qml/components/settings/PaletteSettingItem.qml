import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

// TODO
// Bespoke delegate for the Game Boy palette (widget "gbc-palette"): the usual
// dropdown row plus a live preview of the selected palette underneath. The
// reference for what `type: "custom"` looks like — extend a row component and
// add to it, rather than rebuilding the row
ComboBoxSettingItem {
    id: root

    property var options: []
    property string value: ""
    signal activated(string value)

    textRole: "label"
    valueRole: "value"
    comboBoxModel: root.options

    currentIndex: {
        for (let i = 0; i < root.options.length; i++) {
            if (root.options[i].value === root.value) {
                return i;
            }
        }
        return -1;
    }

    onClicked: function () {
        root.popup.open();
        root.popup.forceActiveFocus();
    }
    onCurrentValueChanged: {
        if (root.value !== currentValue && currentValue) {
            root.activated(currentValue);
        }
    }

    // Lands in BaseSettingItem's nested slot, indented under the row
    Image {
        Layout.alignment: Qt.AlignHCenter
        Layout.preferredWidth: 240
        Layout.preferredHeight: 240
        fillMode: Image.PreserveAspectFit
        // Preview assets are named like "gb-dmg" for value "GB - DMG"
        source: root.value !== ""
                ? "qrc:images/gbc-previews/" + root.value.toLowerCase().replace(/ /g, "")
                : ""
        visible: source.toString() !== ""
    }
}
