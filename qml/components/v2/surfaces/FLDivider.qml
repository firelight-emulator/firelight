import QtQuick
import QtQuick.Layouts
import Firelight 1.0

// Hairline separator. Horizontal by default; set `vertical: true` for a column
// divider. Fills its cross-axis inside a Layout.
Rectangle {
    property bool vertical: false

    implicitWidth: vertical ? Math.max(1, Math.round(AppStyle.scale)) : 0
    implicitHeight: vertical ? 0 : Math.max(1, Math.round(AppStyle.scale))
    Layout.fillWidth: !vertical
    Layout.fillHeight: vertical
    color: Theme.border
    opacity: 0.5
}
