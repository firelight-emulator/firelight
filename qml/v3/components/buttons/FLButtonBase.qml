import QtQuick
import QtQuick.Controls
import Firelight 1.0

Button {
    id: control

    // primary | default | danger | subtle | flat
    property string variant: "default"
    property bool rounded: true

    property bool compact: false
    property string tooltipText: ""

    // Allows the button to be disabled but still allow it to receive focus
    property bool canInteract: true

    property color checkedColor: Theme.textPrimary
    FLFocus.showCursor: true

    FLFocus.actions: [
        FLAction {
            label: "OK"
            keys: [Qt.Key_Enter, Qt.Key_Return, Qt.Key_Space, Qt.Key_Select]
            sound: control.canInteract ? SoundEffects.openPopup : SoundEffects.cursorBump
            onTriggered: {
                if (!control.canInteract) {
                    return;
                }

                control.click();
            }
        }
    ]

    readonly property bool _subtle: variant === "subtle"
    readonly property color _fill: variant === "primary" ? Theme.accent : variant === "danger" ? Theme.danger : variant === "flat" ? "transparent" : Theme.surfaceElevated
    readonly property color _fg: {
        if (variant === "flat") {
            if (hover.hovered || checked) {
                return Theme.textPrimary;
            }

            // TODO!!!!
            return Theme.textMuted;
        }

        if (checked && variant === "subtle") {
            return checkedColor;
        }

        if (variant === "primary") {
            return Theme.onAccent;
        }

        if (variant === "danger") {
            return "white";
        }

        return Theme.textPrimary;
    }

    hoverEnabled: true
    opacity: canInteract ? 1 : 0.5
    implicitHeight: compact ? AppStyle.buttonHeightCompact : AppStyle.buttonHeight
    implicitWidth: AppStyle.buttonStandardWidth

    readonly property bool cursorFocused: FocusCursor.isOn(control)
    readonly property var activationKeys: [Qt.Key_Select, Qt.Key_Return, Qt.Key_Enter, Qt.Key_Space]

    // Handle Qt's built-in space and select stuff for buttons
    Connections {
        target: control.Keys

        function onPressed(event) {
            const action = control.FLFocus.getActionFor(event.key, event.modifiers);

            if (action !== null) {
                action.trigger();
                event.accepted = true;
                return;
            }

            if (control.activationKeys.indexOf(event.key) !== -1) {
                control.click();
                event.accepted = true;
            }
        }
    }

    HoverHandler {
        id: hover
        cursorShape: Qt.PointingHandCursor
    }

    background: Rectangle {
        radius: control.rounded ? width / 2 : AppStyle.radiusMd
        color: control._subtle ? "transparent" : control._fill
        border.width: control.variant === "default" ? 1 : 0
        border.color: Theme.border

        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: Theme.textPrimary
            opacity: {
                if (!control.canInteract) {
                    return 0;
                }

                if (control.variant === "flat") {
                    return control.cursorFocused ? Theme.buttonBgOpacityFocused : 0;
                }

                if (control._subtle) {
                    return control.pressed ? 0.14 : (hover.hovered || control.cursorFocused) ? 0.10 : 0;
                }

                return control.pressed ? 0.12 : hover.hovered ? 0.07 : 0;
            }
            Behavior on opacity {
                NumberAnimation {
                    duration: 64
                }
            }
        }
    }

    FLToolTip {
        x: control.width + AppStyle.spacingMd
        y: control.height / 2 - height / 2
        visible: (hover.hovered || control.cursorFocused) && control.tooltipText !== ""
        text: control.tooltipText
    }
}
