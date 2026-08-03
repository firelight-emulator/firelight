import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

// A standard selectable row: optional leading icon, a label, and an optional
// trailing item (placed as a child). Height derives from content and is floored
// at listRowHeight, so it reflows when text grows
//
//   FLListRow { iconName: "settings"; label: "Appearance" }
//   FLListRow { label: "Name"; FLRadioIndicator { selected: true } }
ItemDelegate {
    id: control

    property string iconName: ""
    property string label: ""
    default property alias trailing: trailingSlot.data
    FLFocus.showCursor: true

    opacity: control.enabled ? 1 : 0.4

    padding: AppStyle.spacingXs
    leftPadding: AppStyle.spacingLg
    rightPadding: AppStyle.spacingLg
    implicitHeight: Math.max(AppStyle.listRowHeight, rowLayout.implicitHeight + topPadding + bottomPadding)
    implicitWidth: rowLayout.implicitWidth + leftPadding + rightPadding
    focusPolicy: Qt.StrongFocus

    // TODO
    // Focus as the controller cursor shows it, shared with every other control
    // so a row never stays lit while the ring is blinked out
    readonly property bool cursorFocused: FocusCursor.isOn(control)

    highlighted: control.cursorFocused || control.pressed || rowHover.hovered

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
            color: control.label === "Name" ? "#1bcbfd" : Theme.textPrimary
            font.pixelSize: AppStyle.fontSizeMedium
            font.family: AppStyle.fontFamily
            font.weight: Font.DemiBold
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

    // TODO
    // Hover and keyboard focus read the same; pressing drops a step down the
    // surface ladder so the row darkens under the finger
    background: Rectangle {
        radius: 6
        color: control.pressed ? Theme.surfaceElevated : control.highlighted ? Theme.surfaceHover : "transparent"

        Behavior on color {
            ColorAnimation {
                duration: AppStyle.durationSnap
            }
        }
    }
}
