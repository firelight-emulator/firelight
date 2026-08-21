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

    Layout.fillWidth: true

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

    // TODO
    // The keys that activate a row when it declared no action answering to them
    readonly property var activationKeys: [Qt.Key_Select, Qt.Key_Return, Qt.Key_Enter, Qt.Key_Space]

    // TODO
    // Space and Select never leave a row — ItemDelegate takes both for itself and
    // turns them into a click — so a declared action answering to either has to
    // run here rather than at the window. A Connections because a derived type
    // declaring Keys.onPressed would replace an instance handler
    Connections {
        target: control.Keys

        function onPressed(event) {
            if (event.isAutoRepeat) {
                return;
            }

            const action = control.FLFocus.getActionFor(event.key, event.modifiers);

            if (action !== null) {
                if (action.sound) {
                    action.sound.play(false);
                }

                action.triggered();
                event.accepted = true;
                return;
            }

            // TODO
            // Activation happens on the press for every key. Left alone, the row
            // takes Space and Select for itself and commits them on the release
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
