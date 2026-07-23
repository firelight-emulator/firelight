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

    // Gamepad and keyboard focus has to be visible; a mouse user already knows
    // where they are, so the ring is suppressed for them
    readonly property bool _focusRing: activeFocus && !InputMethodManager.usingMouse

    opacity: control.enabled ? 1 : 0.4

    padding: AppStyle.spacingSm
    leftPadding: AppStyle.spacingMd
    rightPadding: AppStyle.spacingMd
    implicitHeight: Math.max(AppStyle.rowHeight, rowLayout.implicitHeight + topPadding + bottomPadding)
    implicitWidth: rowLayout.implicitWidth + leftPadding + rightPadding

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
        border.width: control._focusRing ? 2 : 0
        border.color: Theme.accent
    }
}
