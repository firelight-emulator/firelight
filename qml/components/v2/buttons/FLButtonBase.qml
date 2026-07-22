import QtQuick
import QtQuick.Controls

Button {
    id: control

    // primary | default | danger | subtle
    property string variant: "default"

    property bool compact: false
    property string tooltipText: ""
    // TODO
    // Icon/label tint when checked; subtle toggles only
    property color checkedColor: Theme.textPrimary
    property bool showGlobalCursor: true

    readonly property bool _subtle: variant === "subtle"
    readonly property color _fill: variant === "primary" ? Theme.accent : variant === "danger" ? Theme.danger : Theme.surfaceElevated
    readonly property color _fg: control.checked && control._subtle ? checkedColor : variant === "primary" ? Theme.onAccent : variant === "danger" ? "white" : Theme.textPrimary

    hoverEnabled: true
    opacity: enabled ? 1 : 0.5
    implicitHeight: compact ? AppStyle.buttonHeightCompact : AppStyle.buttonHeight

    HoverHandler {
        id: hover
        cursorShape: Qt.PointingHandCursor
    }

    background: Rectangle {
        radius: AppStyle.radiusMd
        color: control._subtle ? "transparent" : control._fill
        border.width: control.variant === "default" ? 1 : 0
        border.color: Theme.border

        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: Theme.textPrimary
            opacity: !control.enabled ? 0 : control._subtle ? (control.pressed ? 0.14 : (hover.hovered || control.checked) ? 0.10 : 0) : (control.pressed ? 0.12 : hover.hovered ? 0.07 : 0)
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
        visible: (hover.hovered || (control.activeFocus && !InputMethodManager.usingMouse)) && control.tooltipText !== ""
        text: control.tooltipText
    }
}
