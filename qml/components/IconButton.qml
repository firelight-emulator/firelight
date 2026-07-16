import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import QtQuick.VectorImage

RoundButton {
    id: control
    // height: 50
    // width: 50
    //
    // icon.width: 32
    // icon.height: 32
    // icon.color: "white"

    required property string iconName
    property string tooltipText: ""

    property real size: 24

    property real xOffset: 0

    hoverEnabled: true

    property bool showGlobalCursor: true
    property int globalCursorSpacing: 1

    flat: true

    contentItem: Icon {
        anchors.centerIn: parent
        name: control.iconName
        size: control.size
        opacity: control.enabled ? 1 : 0.5
    }

    background: Rectangle {
        color: control.pressed || control.hovered ? ColorPalette.neutral100 : "transparent"
        radius: control.width / 2
        opacity: !control.enabled ? 0 : control.pressed ? 0.14 : 0.23
    }

    FLToolTip {
        y: control.height / 2 - height / 2 - verticalPadding / 2
        x: control.width + 8
        visible: (control.hovered || (!InputMethodManager.usingMouse && activeFocus)) && control.tooltipText !== ""
        text: control.tooltipText
    }
}
