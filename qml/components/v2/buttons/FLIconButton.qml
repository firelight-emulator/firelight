import QtQuick
import QtQuick.Controls
import Firelight 1.0

// Icon-only button. Glyph size from AppStyle (sm/md/lg, or an explicit
// iconSize), colors from Theme. The hit target is floored at minTarget
// (WCAG 2.5.8) even when the glyph is small. Checkable for toolbar toggles
//
//   FLIconButton { iconName: "settings"; tooltipText: "Settings" }
//   FLIconButton { iconName: "grid"; size: "sm"; checkable: true }
RoundButton {
    id: control

    required property string iconName
    // sm | md | lg
    property string size: "md"
    property int iconSize: size === "sm" ? AppStyle.iconSizeSm : size === "lg" ? AppStyle.iconSizeLg : AppStyle.iconSizeMd
    property string tooltipText: ""
    // Icon color when checked. Defaults to accent for toolbar toggles; set to
    // Theme.textPrimary for nav-style buttons that shouldn't tint on select
    property color checkedColor: Theme.accent
    property bool showGlobalCursor: true

    flat: true
    hoverEnabled: false
    implicitWidth: Math.max(AppStyle.minTarget, iconSize + AppStyle.spacingMd)
    implicitHeight: implicitWidth

    TapHandler {
        id: tapHandler
        // toggle() is a no-op unless checkable; clicked() always fires
        onTapped: {
            control.toggle();
            control.clicked();
        }
        onDoubleTapped: control.doubleClicked()
        margin: 8
    }
    HoverHandler {
        id: hoverHandler
        cursorShape: Qt.PointingHandCursor
        margin: 8
    }
    down: tapHandler.pressed

    contentItem: Icon {
        anchors.centerIn: parent
        name: control.iconName
        size: control.iconSize
        color: control.checked ? control.checkedColor : Theme.textPrimary
        opacity: control.enabled ? 1 : 0.5
    }

    // Focus is drawn by the app's global cursor overlay (showGlobalCursor), so
    // the button paints only the hover/press/checked wash — no second ring
    background: Rectangle {
        radius: width / 2
        color: Theme.textPrimary
        opacity: !control.enabled ? 0 : tapHandler.pressed ? 0.14 : (hoverHandler.hovered || control.checked) ? 0.10 : 0
    }

    FLToolTip {
        y: control.height / 2 - height / 2 - verticalPadding / 2
        x: control.width + AppStyle.spacingSm
        visible: (hoverHandler.hovered || (control.activeFocus && !InputMethodManager.usingMouse)) && control.tooltipText !== ""
        text: control.tooltipText
    }
}
