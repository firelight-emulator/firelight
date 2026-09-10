// TODO: NEEDS REVIEW
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

//   FLListRow { iconName: "settings"; label: "Appearance" }
//   FLListRow { label: "Name"; FLRadioIndicator { selected: true } }
ItemDelegate {
    id: control

    property string iconName: ""
    property string label: ""
    default property alias trailing: trailingSlot.data
    FLFocus.showCursor: true

    Layout.fillWidth: true

    opacity: control.enabled ? 1 : 0.4

    padding: AppStyle.spacingXs
    leftPadding: AppStyle.spacingLg
    rightPadding: AppStyle.spacingLg
    implicitHeight: Math.max(AppStyle.listRowHeight, rowLayout.implicitHeight + topPadding + bottomPadding)
    implicitWidth: rowLayout.implicitWidth + leftPadding + rightPadding
    focusPolicy: Qt.StrongFocus

    readonly property bool cursorFocused: FocusCursor.isOn(control)

    highlighted: control.cursorFocused || control.pressed || rowHover.hovered

    readonly property var activationKeys: [Qt.Key_Select, Qt.Key_Return, Qt.Key_Enter, Qt.Key_Space]

    // Handle space and select for buttons (Qt normally handles it)
    Connections {
        target: control.Keys

        function onPressed(event) {
            const action = control.FLFocus.getActionFor(event.key, event.modifiers);

            if (action !== null) {
                action.triggerForPress(event.isAutoRepeat);
                event.accepted = true;
                return;
            }

            // TODO
            // Only an action opts into repeating: a held key never presses the row itself
            if (event.isAutoRepeat) {
                return;
            }

            if (control.activationKeys.indexOf(event.key) !== -1) {
                control.click();
                event.accepted = true;
            }
        }
    }

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
            color: control.checked ? Theme.switch2Color : Theme.textPrimary
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
