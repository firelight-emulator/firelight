import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

// A titled group of setting rows rendered as one card. Put the rows in as
// children; they stack inside the card, separated by hairlines, with the section
// title above it.
FocusScope {
    id: root

    property string title: ""
    property bool showTopPadding: true

    default property alias content: rows.data

    implicitWidth: layout.implicitWidth
    implicitHeight: layout.implicitHeight

    function updateRows() {
        const kids = rows.children;
        // No line after the card's last row.
        for (let i = 0; i < kids.length; i++) {
            if (kids[i].showDivider !== undefined) {
                kids[i].showDivider = (i !== kids.length - 1);
            }
        }
        // A sub-group welds to the setting it depends on: drop the line between
        // that row and the group beneath it.
        for (let j = 1; j < kids.length; j++) {
            if (kids[j].isSettingSubGroup === true
                    && kids[j - 1].showDivider !== undefined) {
                kids[j - 1].showDivider = false;
            }
        }
    }

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
            bottomPadding: 10
            leftPadding: 2
            rightPadding: 0
            background: Item {}

            contentItem: Text {
                text: root.title
                color: Theme.textPrimary
                font.pixelSize: AppStyle.fontSizeLarge
                font.family: Constants.regularFontFamily
                font.weight: Font.DemiBold
                verticalAlignment: Text.AlignVCenter
            }
        }

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: rows.implicitHeight
            radius: 12
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
