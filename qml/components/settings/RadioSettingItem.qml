import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

// A vertical list of mutually exclusive options, rendered full-width below the
// label. `options` is [{label, value, note?}]; `currentValue` is the selected
// value; emits `changed` on selection
BaseSettingItem {
    id: root

    controlBelow: true

    property var options: []
    property string currentValue: ""
    signal changed(string value)

    control: ColumnLayout {
        spacing: 0

        Repeater {
            model: root.options
            delegate: AbstractButton {
                id: opt
                required property var modelData
                readonly property bool on: root.currentValue === modelData.value

                Layout.fillWidth: true
                implicitHeight: 40
                enabled: root.enabled
                focusPolicy: Qt.NoFocus
                onClicked: root.changed(modelData.value)

                HoverHandler { cursorShape: Qt.PointingHandCursor }

                background: Item {}

                contentItem: RowLayout {
                    spacing: 11

                    Rectangle {
                        Layout.leftMargin: 4
                        implicitWidth: 18
                        implicitHeight: 18
                        radius: 9
                        color: "transparent"
                        border.width: 2
                        border.color: opt.on ? Theme.accent : Theme.borderStrong

                        Rectangle {
                            visible: opt.on
                            anchors.centerIn: parent
                            width: 10
                            height: 10
                            radius: 5
                            color: Theme.accent
                        }
                    }

                    Text {
                        text: opt.modelData.label
                        color: Theme.textPrimary
                        font.family: Constants.regularFontFamily
                        font.pixelSize: AppStyle.fontSizeMedium
                        font.weight: Font.Medium
                        verticalAlignment: Text.AlignVCenter
                    }

                    Item { Layout.fillWidth: true }

                    Text {
                        visible: opt.modelData.note !== undefined && opt.modelData.note !== ""
                        text: opt.modelData.note !== undefined ? opt.modelData.note : ""
                        color: Theme.textMuted
                        font.family: Constants.regularFontFamily
                        font.pixelSize: AppStyle.fontSizeSmall
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }
    }
}
