import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import Firelight 1.0

RoundButton {
    id: control
    height: 50
    width: 50

    icon.width: 32
    icon.height: 32

    property bool showGlobalCursor: true
    property int globalCursorSpacing: 1

    flat: true
}
