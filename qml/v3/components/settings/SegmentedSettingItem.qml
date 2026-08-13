import QtQuick

BaseSettingItem {
    id: root

    property var options: []
    property string currentValue: ""
    property bool compact: false

    signal changed(string value)

    controlBelow: (options ? options.length : 0) >= 5

    control: FLSegmentedControl {
        segments: root.options
        compact: root.compact
        currentValue: root.currentValue

        onActivated: function (value) {
            root.changed(value);
        }
    }
}
