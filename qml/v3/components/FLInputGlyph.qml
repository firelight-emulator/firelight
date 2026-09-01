import QtQuick
import Firelight 1.0

Image {
    id: root

    required property int input
    property int size: AppStyle.gamepadGlyphSize
    property real enabledOpacity: 1.0

    source: InputService.currentGamepadButtonIcons[input]
    sourceSize.width: root.size
    sourceSize.height: root.size
    fillMode: Image.PreserveAspectFit
    opacity: root.enabled ? root.enabledOpacity : 0.35
}