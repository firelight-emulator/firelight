import QtQuick
import QtQuick.Controls
import Firelight 1.0

// TODO
// A single-select segmented button group (e.g. Day / Week / Month). Feed
// `segments` a list of labels; `currentIndex` is the selection
//
//   FLSegmentedControl { segments: ["Day", "Week", "Month"]; onActivated: ... }
Row {
    id: control

    property var segments: []
    property int currentIndex: 0
    signal activated(int index)

    spacing: 0

    Repeater {
        model: control.segments
        delegate: AbstractButton {
            id: seg
            required property int index
            required property string modelData
            readonly property bool selected: control.currentIndex === index

            implicitWidth: Math.max(AppStyle.minTarget, label.implicitWidth + AppStyle.spacingLg * 2)
            implicitHeight: Math.max(AppStyle.minTarget, label.implicitHeight + AppStyle.spacingSm * 2)
            property bool showGlobalCursor: true

            HoverHandler { id: segHover; cursorShape: Qt.PointingHandCursor }

            onClicked: {
                control.currentIndex = index;
                control.activated(index);
            }

            contentItem: Text {
                id: label
                text: seg.modelData
                font.pixelSize: AppStyle.fontSizeMedium
                font.family: Constants.regularFontFamily
                font.weight: seg.selected ? Font.DemiBold : Font.Medium
                color: seg.selected ? Theme.textPrimary : Theme.textMuted
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            background: Rectangle {
                color: seg.selected ? Theme.surfaceHover : segHover.hovered ? Theme.surfaceElevated : "transparent"
                border.width: 1
                border.color: Theme.border
                // Round only the outer corners of the first/last segment
                topLeftRadius: seg.index === 0 ? AppStyle.radiusMd : 0
                bottomLeftRadius: seg.index === 0 ? AppStyle.radiusMd : 0
                topRightRadius: seg.index === control.segments.length - 1 ? AppStyle.radiusMd : 0
                bottomRightRadius: seg.index === control.segments.length - 1 ? AppStyle.radiusMd : 0
            }
        }
    }
}
