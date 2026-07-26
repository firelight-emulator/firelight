import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// A single-select segmented button group (e.g. Day / Week / Month). `segments`
// is a list of {label, value, icon} (icon optional); the selection is the
// segment whose value equals `currentValue`, and `activated` carries that value
//
//   FLSegmentedControl {
//       segments: [{label: "Day", value: "day"}, {label: "Week", value: "week"}]
//       currentValue: "day"
//       onActivated: value => ...
//   }
Rectangle {
    id: control

    objectName: "FLSegmentedControl|" + currentValue

    property var segments: []
    property string currentValue: ""
    property bool compact: true
    signal activated(string value)

    onSegmentsChanged: segRow.maxImplicitWidth = 0

    readonly property int _pad: 4
    property bool _ready: false

    // TODO
    // Index of the selected segment, or -1 when nothing matches currentValue
    readonly property int _selectedIndex: {
        for (let i = 0; i < control.segments.length; ++i) {
            if (control.segments[i].value === control.currentValue) {
                return i;
            }
        }
        return -1;
    }

    implicitHeight: segRow.implicitHeight + control._pad * 2
    implicitWidth: segRow.implicitWidth + control._pad * 2
    radius: AppStyle.radiusLg
    color: Theme.backgroundInset
    border.width: 1
    border.color: Theme.border

    Component.onCompleted: control._ready = true

    // Accent pill behind the selected segment
    Rectangle {
        readonly property real segWidth: {
            const numSegments = control.segments.length;
            if (numSegments <= 0) {
                return 0;
            }

            return (segRow.width - segRow.spacing * (numSegments - 1)) / numSegments;
        }

        color: Theme.surfaceHover
        radius: AppStyle.radiusMd
        visible: control._selectedIndex >= 0
        y: segRow.y
        height: segRow.height
        width: segWidth
        x: segRow.x + control._selectedIndex * (segWidth + segRow.spacing)

        Behavior on x {
            enabled: control._ready
            NumberAnimation {
                duration: 60
                easing.type: Easing.OutCubic
            }
        }
    }

    RowLayout {
        id: segRow
        anchors.fill: parent
        anchors.margins: control._pad
        spacing: 2

        property int maxImplicitWidth: 0

        Repeater {
            id: segRepeater
            model: control.segments
            delegate: FLButton {
                required property var modelData

                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: segRow.maxImplicitWidth

                text: modelData.label
                iconName: modelData.icon ?? ""
                variant: "flat"
                checkable: false
                checked: modelData.value === control.currentValue
                checkedColor: Theme.onAccent
                compact: control.compact
                onClicked: control.activated(modelData.value)

                onImplicitWidthChanged: segRow.maxImplicitWidth = Math.max(segRow.maxImplicitWidth, implicitWidth)
            }

            onItemAdded: function (index, item) {
                segRow.maxImplicitWidth = Math.max(item.implicitWidth, segRow.maxImplicitWidth);
            }
        }
    }
}
