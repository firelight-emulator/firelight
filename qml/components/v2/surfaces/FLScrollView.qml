import QtQuick
import QtQuick.Controls
import Firelight 1.0

// ScrollView with the app scrollbar baked in, so callers stop pairing a bespoke
// ListView/Flickable with an inline FLScrollBar. Put scrollable content inside
ScrollView {
    id: control

    clip: true
    ScrollBar.vertical: FLScrollBar {}
    ScrollBar.horizontal: FLScrollBar {}
}
