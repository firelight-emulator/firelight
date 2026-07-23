import QtQuick
import QtQuick.Controls

// Themed single-line text input. Height derives from content (floored at
// minTarget); colors from Theme, metrics from AppStyle
TextField {
    id: control

    objectName: "FLTextField|" + placeholderText

    // sm | md
    property string size: "md"
    readonly property int _font: size === "sm" ? AppStyle.fontSizeSmall : AppStyle.fontSizeMedium

    color: Theme.textPrimary
    placeholderTextColor: Theme.textMuted
    font.pixelSize: _font
    font.family: AppStyle.fontFamily
    selectByMouse: true
    leftPadding: AppStyle.spacingMd
    rightPadding: AppStyle.spacingMd
    topPadding: AppStyle.spacingSm
    bottomPadding: AppStyle.spacingSm
    implicitHeight: Math.max(AppStyle.minTarget, contentHeight + topPadding + bottomPadding)

    background: Rectangle {
        radius: AppStyle.radiusMd
        color: control.activeFocus ? Theme.surfaceHover : Theme.surfaceElevated
        border.width: control.activeFocus ? 2 : 1
        border.color: control.activeFocus ? Theme.accent : Theme.border
    }
}
