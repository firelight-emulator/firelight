import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// A titled group of setting rows rendered as one card. Put the rows in as
// children; they stack inside the card, separated by hairlines, with the section
// title above it
FocusScope {
    id: root

    property string title: ""
    property bool showTopPadding: true

    default property alias content: rows.data

    implicitWidth: layout.implicitWidth
    implicitHeight: layout.implicitHeight

    function updateRows() {
        const kids = rows.children;

        // The visible rows that can carry a divider, in order. Hidden rows are
        // skipped: a line either side of nothing is still a line
        const visible = [];
        for (let i = 0; i < kids.length; i++) {
            if (kids[i].showDivider !== undefined && kids[i].visible) {
                visible.push(kids[i]);
            } else if (kids[i].showDivider !== undefined) {
                kids[i].showDivider = false;
            }
        }

        for (let i = 0; i < visible.length; i++) {
            const next = (i + 1 < visible.length) ? visible[i + 1] : null;
            // No line after the card's last row, and none between a row and
            // whatever depends on it: a sub-setting welds to the setting above
            // it, so they read as one block
            const weldsToNext = next !== null && next.subItem === true;
            visible[i].showDivider = (next !== null) && !weldsToNext;
        }
    }

    // Reading every row's `visible` makes this re-evaluate whenever rows are
    // added, removed, shown or hidden — which is the trigger for updateRows()
    // Rows arrive from a Repeater long after Component.onCompleted, and
    // dependent rows come and go, so the layout can't be decided just once
    readonly property var rowVisibility: {
        const out = [];
        for (let i = 0; i < rows.children.length; i++) {
            out.push(rows.children[i].visible === true);
        }
        return out;
    }
    onRowVisibilityChanged: updateRows()

    Component.onCompleted: updateRows()

    ColumnLayout {
        id: layout
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 0

        Pane {
            Layout.fillWidth: true
            visible: root.title !== ""
            topPadding: root.showTopPadding ? 22 : 2
            bottomPadding: AppStyle.spacingMd
            leftPadding: 2
            rightPadding: 0
            background: Item {}

            contentItem: Text {
                text: root.title
                color: Theme.textPrimary
                font.pixelSize: AppStyle.fontSizeLarge
                font.family: AppStyle.fontFamily
                font.weight: Font.DemiBold
                verticalAlignment: Text.AlignVCenter
            }
        }

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: rows.implicitHeight
            radius: AppStyle.radiusLg
            color: Theme.surface
            border.width: 1
            border.color: Theme.border
            clip: true

            ColumnLayout {
                id: rows
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                spacing: 0
            }
        }
    }
}
