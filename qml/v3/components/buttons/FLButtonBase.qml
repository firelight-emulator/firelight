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
    // TODO
    // Icon/label tint when checked; subtle toggles only
    property color checkedColor: Theme.textPrimary
    FLFocus.showCursor: true

    property bool canInteract: true

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

    // TODO
    // Focus as the controller cursor shows it; a mouse click also lands focus,
    // and leaving that tinted reads as stuck
    readonly property bool cursorFocused: FocusCursor.isOn(control)

    // TODO
    // The keys that activate a button when it declared no action answering to them
    readonly property var activationKeys: [Qt.Key_Select, Qt.Key_Return, Qt.Key_Enter, Qt.Key_Space]

    // TODO
    // Space and Select never leave a button — it takes both for itself and turns
    // them into a click — so a declared action answering to either has to run
    // here rather than at the window. A Connections because a derived type
    // declaring Keys.onPressed would replace an instance handler
    Connections {
        target: control.Keys

        function onPressed(event) {
            const action = control.FLFocus.getActionFor(event.key);

            if (action !== null) {
                if (action.sound) {
                    action.sound.play(false);
                }

                action.triggered();
                event.accepted = true;
                return;
            }

            // TODO
            // Activation happens on the press for every key. Left alone, the
            // button takes Space and Select for itself and commits them on the
            // release
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
        // radius: AppStyle.radiusMd
        radius: control.rounded ? width / 2 : AppStyle.radiusMd
        color: control._subtle ? "transparent" : control._fill
        border.width: control.variant === "default" ? 1 : 0
        border.color: Theme.border

        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: Theme.textPrimary
            // TODO
            // Flat carries no chrome at rest, but the cursor still needs a fill
            // behind the ring; hover keeps signalling through the icon instead
            opacity: {
                if (!control.canInteract) {
                    return 0;
                }

                if (control.variant === "flat") {
                    return control.cursorFocused ? Theme.buttonBgOpacityFocused : 0;
                }

                if (control._subtle) {
                    return control.pressed ? 0.14 : (hover.hovered || control.checked || control.cursorFocused) ? 0.10 : 0;
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
        x: control.width + AppStyle.spacingSm
        y: control.height / 2 - height / 2 - verticalPadding / 2
        visible: (hover.hovered || control.cursorFocused) && control.tooltipText !== ""
        text: control.tooltipText
    }
}
