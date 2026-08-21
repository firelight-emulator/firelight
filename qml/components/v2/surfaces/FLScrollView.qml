import QtQuick
import QtQuick.Controls

// ScrollView with the app scrollbar baked in, so callers stop pairing a bespoke
// ListView/Flickable with an inline FLScrollBar. Put scrollable content inside
//
// Vertical only by default. Set horizontalPolicy to turn the other axis on —
// attaching it unconditionally gives a vertical list a scroll axis it never wanted
ScrollView {
    id: control

    property int horizontalPolicy: ScrollBar.AlwaysOff

    clip: true
    ScrollBar.vertical: FLScrollBar {}
    ScrollBar.horizontal: FLScrollBar {
        policy: control.horizontalPolicy
    }
}
