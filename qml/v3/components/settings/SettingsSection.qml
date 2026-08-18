// TODO: NEEDS REVIEW
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// A titled group of setting rows rendered as one card. Put the rows in as
// children; they stack inside the card, separated by hairlines, with the section
// title above it
FocusScope {
    id: root

    property string title: ""
    property bool showHeader: true
    property bool showTopPadding: true
    property int surface: FLMenuItem.Surface.Page

    readonly property bool _onPage: root.surface === FLMenuItem.Surface.Page
    readonly property bool _showHeader: root.title !== "" && root.showHeader

    property bool _anyVisibleChildren: true

    focusPolicy: Qt.StrongFocus

    default property alias content: rows.data

    implicitWidth: layout.implicitWidth
    implicitHeight: layout.implicitHeight

    Behavior on implicitWidth {
        NumberAnimation {
            duration: AppStyle.durationFast
            easing.type: Easing.InOutQuad
        }
    }

    Behavior on implicitHeight {
        NumberAnimation {
            duration: AppStyle.durationFast
            easing.type: Easing.InOutQuad
        }
    }

    function updateRows() {
        const kids = rows.children;

        const visible = [];
        for (let i = 0; i < kids.length; i++) {
            if (kids[i].surface === undefined) {
                continue;
            }

            kids[i].surface = root.surface;

            if (kids[i].visible) {
                visible.push(kids[i]);
            } else {
                kids[i].showDivider = false;
            }
        }

        for (let i = 0; i < visible.length; i++) {
            const next = (i + 1 < visible.length) ? visible[i + 1] : null;
            const weldsToNext = next !== null && next.subItem === true;
            visible[i].showDivider = root._onPage && next !== null && !weldsToNext;

            visible[i].isFirstInSection = i === 0;
            visible[i].isLastInSection = next === null;
        }
    }

    // TODO
    // The column above this hands focus in through here, the way it does for any other row. Without
    // it the column forces focus on the FocusScope and lets Qt pick a descendant, which two of these
    // on screen at once turn into a race
    function enterFrom(step: int) {
        rows.enterFrom(step);
    }

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
            visible: root._showHeader
            topPadding: root.showTopPadding ? 22 : 2
            bottomPadding: AppStyle.spacingMd
            leftPadding: 2
            rightPadding: 0
            background: Item {}

            contentItem: Text {
                text: root.title
                color: Theme.textPrimary
                font.pixelSize: AppStyle.fontSizeMedium
                font.family: AppStyle.fontFamily
                font.weight: Font.Normal
                verticalAlignment: Text.AlignVCenter
            }
        }

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: rows.implicitHeight
            radius: AppStyle.radiusLg
            color: root._onPage ? Theme.surface : "transparent"
            border.width: root._onPage ? 1 : 0
            border.color: root._onPage ? Theme.border : "transparent"
            clip: true

            FLColumnLayout {
                id: rows
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                spacing: 0
            }
        }
    }
}
