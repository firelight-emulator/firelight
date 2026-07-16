import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

// Groups dependent setting rows so they reveal and collapse together as one
// unit — bind `shown` to whatever they depend on. Its rows are marked as
// sub-items (indented, quieter label) and SettingsSection drops the divider
// above the group, so it reads as belonging to the setting it sits under.
FocusScope {
    id: root

    property bool shown: true
    default property alias content: col.data

    // Lets SettingsSection weld this group to the row above it.
    readonly property bool isSettingSubGroup: true

    Layout.fillWidth: true

    function updateRows() {
        const kids = col.children;
        for (let i = 0; i < kids.length; i++) {
            if (kids[i].subItem !== undefined) {
                kids[i].subItem = true;
            }
            // The group is one welded block: no lines between its rows, and
            // none after it — the indent and quieter labels do the work.
            if (kids[i].showDivider !== undefined) {
                kids[i].showDivider = false;
            }
        }
    }

    Component.onCompleted: updateRows()

    visible: shown
    implicitHeight: col.implicitHeight

    ColumnLayout {
        id: col
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 0
    }
}
