// TODO: NEEDS REVIEW
import QtQuick
import QtQuick.Controls
import Firelight 1.0

Item {
    id: root

    required property int index
    required property SelectionGroup selectionGroup

    property var topLeftRadius: 0
    property var topRightRadius: 0
    property var bottomLeftRadius: 0
    property var bottomRightRadius: 0

    readonly property bool _selected: root.selectionGroup.selected[root.index] === true
    readonly property bool _inActiveRange: {
        if (!root.selectionGroup.editingRange) {
            return false;
        }

        const rangeStart = root.selectionGroup.rangeStart;
        const rangeEnd = root.selectionGroup.rangeEnd;

        const aboveLow = root.index >= Math.min(rangeStart, rangeEnd);
        const belowHigh = root.index <= Math.max(rangeStart, rangeEnd);

        return rangeStart !== -1
            && rangeEnd !== -1
            && aboveLow
            && belowHigh;
    }

    anchors.fill: parent
    visible: root.selectionGroup.active

    Rectangle {
        color: {
            if (root._inActiveRange) {
                if (root._selected) {
                    return Theme.switch2Color
                } else {
                    return "#2c2c2c"
                }
            }

            return "black"
        }

        anchors.fill: parent
        opacity: root._inActiveRange || root._selected ? 0.6 : 0.0
        topLeftRadius: root.topLeftRadius
        topRightRadius: root.topRightRadius
        bottomLeftRadius: root.bottomLeftRadius
        bottomRightRadius: root.bottomRightRadius
    }

    Rectangle {
        border.color: "white"
        border.width: 3
        color: "transparent"
        width: 32
        height: 32
        radius: 6
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.rightMargin: 12
        anchors.topMargin: 12

        Rectangle {
            color: root._selected ? Theme.switch2Color : "white"
            opacity: root._selected ? 1.0 : 0.5
            anchors.fill: parent
            radius: 3
            anchors.margins: parent.border.width
        }

        Icon {
            anchors.centerIn: parent
            name: "check"
            size: AppStyle.iconSizeMd
            color: Theme.textPrimary
            weight: Font.DemiBold
            visible: root._selected
        }
    }
}