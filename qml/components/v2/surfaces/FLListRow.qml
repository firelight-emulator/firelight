import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// A standard selectable row: optional leading icon, a label, and an optional
// trailing item (placed as a child). Height derives from content and is floored
// at rowHeight, so it reflows when text grows
//
//   FLListRow { iconName: "settings"; label: "Appearance" }
ItemDelegate {
    id: control

    property string iconName: ""
    property string label: ""
    default property alias trailing: trailingSlot.data
    property bool showGlobalCursor: true

    opacity: control.enabled ? 1 : 0.4

    padding: AppStyle.spacingXs
    leftPadding: AppStyle.spacingSm
    rightPadding: AppStyle.spacingSm
    implicitHeight: Math.max(AppStyle.listRowHeight, rowLayout.implicitHeight + topPadding + bottomPadding)
    implicitWidth: rowLayout.implicitWidth + leftPadding + rightPadding
    focusPolicy: Qt.StrongFocus

    HoverHandler {
        id: rowHover
        enabled: control.enabled
        cursorShape: Qt.PointingHandCursor
    }

    contentItem: RowLayout {
        id: rowLayout
        spacing: AppStyle.spacingMd
        Icon {
            visible: control.iconName !== ""
            Layout.alignment: Qt.AlignVCenter
            name: control.iconName
            size: AppStyle.iconSizeMd
            color: control.highlighted ? Theme.accent : Theme.textPrimary
        }
        Text {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            text: control.label
            color: Theme.textPrimary
            font.pixelSize: AppStyle.fontSizeMedium
            font.family: AppStyle.fontFamily
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
        }
        Item {
            id: trailingSlot
            Layout.alignment: Qt.AlignVCenter
            implicitWidth: childrenRect.width
            implicitHeight: childrenRect.height
        }
    }

    background: Rectangle {
        radius: AppStyle.radiusMd
        color: control.highlighted ? Theme.surfaceHover : rowHover.hovered ? Theme.surfaceElevated : "transparent"
    }
}
