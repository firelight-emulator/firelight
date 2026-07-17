import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import QtQuick.VectorImage

RoundButton {
    id: control

    required property string iconName
    required property real iconSize

    property string tooltipText: ""


    property real xOffset: 0

    hoverEnabled: false

    property bool showGlobalCursor: true
    property int globalCursorSpacing: 1

    TapHandler {
        id: tapHandler
        onTapped: control.clicked()
        onDoubleTapped: control.doubleClicked()
        margin: 8
    }

    HoverHandler {
        id: hoverHandler
        cursorShape: Qt.PointingHandCursor
        margin: 8
    }

    down: tapHandler.pressed

    flat: true

    contentItem: Icon {
        anchors.centerIn: parent
        name: control.iconName
        size: control.iconSize
        opacity: control.enabled ? 1 : 0.5
    }

    background: Rectangle {
        color: tapHandler.pressed || hoverHandler.hovered ? ColorPalette.neutral100 : "transparent"
        radius: control.width / 2
        opacity: !control.enabled ? 0 : tapHandler.pressed ? 0.14 : 0.23
    }

    FLToolTip {
        y: control.height / 2 - height / 2 - verticalPadding / 2
        x: control.width + 8
        visible: (hoverHandler.hovered || (!InputMethodManager.usingMouse && activeFocus)) && control.tooltipText !== ""
        text: control.tooltipText
    }
}
