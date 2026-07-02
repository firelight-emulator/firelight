import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

// Bespoke delegate for the Game Boy palette (widget "gbc-palette"): a dropdown
// plus a live preview image of the selected palette.
FocusScope {
    id: root

    property string label: ""
    property string description: ""
    property var options: []
    property string value: ""
    property bool resettable: false

    signal reset()
    signal activated(string value)

    implicitHeight: col.implicitHeight

    ColumnLayout {
        id: col
        width: parent.width
        spacing: 4

        ComboBoxSettingItem {
            id: combo
            Layout.fillWidth: true
            label: root.label
            description: root.description
            resettable: root.resettable

            textRole: "label"
            valueRole: "value"
            comboBoxModel: root.options

            currentIndex: {
                for (let i = 0; i < root.options.length; i++) {
                    if (root.options[i].value === root.value) {
                        return i
                    }
                }
                return -1
            }

            onReset: root.reset()
            onClicked: function () {
                combo.popup.open()
                combo.popup.forceActiveFocus()
            }
            onCurrentValueChanged: {
                if (root.value !== currentValue && currentValue) {
                    root.activated(currentValue)
                }
            }
        }

        Image {
            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: 12
            Layout.preferredWidth: 240
            Layout.preferredHeight: 240
            fillMode: Image.PreserveAspectFit
            // Preview assets are named like "gb-dmg" for value "GB - DMG".
            source: root.value !== "" ? "qrc:images/gbc-previews/" + root.value.toLowerCase().replace(/ /g, "") : ""
            visible: source.toString() !== ""
        }
    }
}
