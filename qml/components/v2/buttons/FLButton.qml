import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Text (+ optional leading icon) button. Colors from Theme, metrics from
// AppStyle. Height is derived from content and floored at minTarget, so it
// grows with the UI scale instead of clipping
//
//   FLButton { text: "Save"; variant: "primary" }
//   FLButton { text: "Delete"; variant: "danger"; iconName: "delete" }
Button {
    id: control

    // primary | secondary | ghost | danger
    property string variant: "secondary"
    // sm | md | lg
    property string size: "md"
    property string iconName: ""
    property bool showGlobalCursor: true

    readonly property int _font: size === "sm" ? AppStyle.fontSizeSmall : size === "lg" ? AppStyle.fontSizeLarge : AppStyle.fontSizeMedium
    readonly property int _vpad: size === "sm" ? AppStyle.spacingXs : size === "lg" ? AppStyle.spacingMd : AppStyle.spacingSm
    readonly property int _hpad: size === "sm" ? AppStyle.spacingMd : size === "lg" ? AppStyle.spacingXl : AppStyle.spacingLg

    readonly property color _fill: variant === "primary" ? Theme.accent : variant === "danger" ? Theme.danger : variant === "ghost" ? "transparent" : Theme.surfaceElevated
    readonly property color _fg: variant === "primary" ? Theme.onAccent : variant === "danger" ? "white" : Theme.textPrimary
    readonly property bool _focusRing: activeFocus && !InputMethodManager.usingMouse

    topPadding: _vpad
    bottomPadding: _vpad
    leftPadding: _hpad
    rightPadding: _hpad
    implicitHeight: Math.max(AppStyle.minTarget, contentItem.implicitHeight + topPadding + bottomPadding)
    implicitWidth: contentItem.implicitWidth + leftPadding + rightPadding

    HoverHandler {
        id: hover
        cursorShape: Qt.PointingHandCursor
    }

    background: Rectangle {
        radius: AppStyle.radiusMd
        color: control._fill
        opacity: control.enabled ? 1 : 0.4
        border.width: control._focusRing ? 2 : (control.variant === "secondary" ? 1 : 0)
        border.color: control._focusRing ? Theme.accent : Theme.border

        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: Theme.textPrimary
            opacity: !control.enabled ? 0 : control.pressed ? 0.12 : hover.hovered ? 0.07 : 0
            Behavior on opacity {
                NumberAnimation {
                    duration: 64
                }
            }
        }
    }

    contentItem: RowLayout {
        spacing: AppStyle.spacingSm
        Icon {
            visible: control.iconName !== ""
            Layout.alignment: Qt.AlignVCenter
            name: control.iconName
            size: control._font
            color: control._fg
        }
        Text {
            visible: control.text !== ""
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            text: control.text
            color: control._fg
            font.pixelSize: control._font
            font.family: Constants.regularFontFamily
            font.weight: Font.DemiBold
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }
}
