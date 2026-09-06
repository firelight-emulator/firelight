// TODO: NEEDS REVIEW
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

FocusScope {
    id: gridRoot

    focus: true
    property var model: null
    required property string currentSortLabel
    required property bool sortAscending

    // Multi-select state owned by GameView and bound in
    property var selectedIds: ({})

    function positionViewAtBeginning() {
        root.positionViewAtBeginning();
    }

    function focusCurrentItem() {
        root.focusCurrentItem();
    }

    readonly property int labelHeight: Math.round(60 * AppStyle.scale)

    signal gameClicked(int entryId, int rowIndex, int modifiers)
    signal requestAddToFolder(var entryIds)
    signal requestChangeArt(string contentHash, string displayName, int platformId)
    signal requestEditGame(int entryId, string contentHash, int platformId)
    signal requestLaunch(int entryId, string contentHash, int platformId, bool playable, string statusText)

    property alias header: root.header
    property alias headerItem: root.headerItem

    property alias footer: root.footer

    property alias contentY: root.contentY
    readonly property alias currentIndex: root.currentIndex

    function targetsFor(entryId) {
        if (gridRoot.selectedIds[entryId] === true) {
            var out = [];
            for (var k in gridRoot.selectedIds) {
                if (gridRoot.selectedIds[k]) {
                    out.push(parseInt(k));
                }
            }
            if (out.length > 0) {
                return out;
            }
        }
        return [entryId];
    }

    function focusFirstItem() {
        root.focusFirstItem();
    }

    function jumpToIndex(index: int) {
        root.jumpTo(index);
    }

    function resetCursor() {
        root.resetCursor();
    }

    FLGridView {
        id: root

        model: gridRoot.model

        displayMarginBeginning: Math.round(cellHeight / 2)

        cacheBuffer: Math.round(height + cellHeight * 8)

        property real initialContentY: 0

        property string sortRole: "displayName"
        property bool sortAscending: true

        width: Math.max(1, Math.floor(parent.width / cellWidth)) * cellWidth
        height: parent.height

        readonly property bool _showTitleBox: AppearanceSettings.libraryIconGridShowTitleBox
        property real _titleBoxHeight: _showTitleBox ? gridRoot.labelHeight : 0

        cellWidth: AppearanceSettings.libraryIconGridTileSize + Math.round(AppearanceSettings.libraryIconGridTileSpacing)
        cellHeight: cellWidth + _titleBoxHeight

        Behavior on _titleBoxHeight {
            NumberAnimation {
                duration: AppStyle.durationBase
                easing.type: Easing.InOutQuad
            }
        }

        readonly property int jumpRows: Math.max(1, Math.floor(height / Math.max(1, cellHeight)))
        readonly property int jumpSize: root.columns * root.jumpRows

        // x position of the top of the row containing index, relative to the contentY of the view. Used for jumping
        function rowTop(index: int): real {
            return root.originY + Math.floor(index / root.columns) * root.cellHeight;
        }

        function jumpTo(index: int) {
            FocusCursor.alignNextToTop(root.rowTop(index));
            root.currentIndex = index;
            root.focusCurrentItem();
        }

        FLFocus.actions: [
            FLAction {
                keys: [Qt.Key_PageDown]
                label: qsTr("Jump down")
                enabled: root.currentIndex < root.count - 1
                onTriggered: {
                    root.adoptFocusedIndex();
                    root.jumpTo(Math.min(root.currentIndex + root.jumpSize, root.count - 1));
                }
            },
            FLAction {
                keys: [Qt.Key_PageUp]
                label: qsTr("Jump up")
                enabled: root.currentIndex > 0
                onTriggered: {
                    root.adoptFocusedIndex();
                    root.jumpTo(Math.max(root.currentIndex - root.jumpSize, 0));
                }
            }
        ]

        Component.onCompleted: {
            initialContentY = contentY;
        }

        ScrollBar.vertical: FLScrollBar {
            parent: gridRoot.Window.window.contentItem
            anchors.right: parent.right
            anchors.rightMargin: (AppStyle.windowPadding / 2) - width / 2
            y: gridRoot.mapToItem(parent, Qt.point(0, 0)).y
            height: gridRoot.height
        }
        boundsBehavior: Flickable.StopAtBounds

        delegate: tileComponent
    }

    Component {
        id: tileComponent

        GameGridViewItem {
            id: gameDelegate

            required property var model

            width: GridView.view.cellWidth
            height: GridView.view.cellHeight

            titleBoxHeight: gridRoot.labelHeight

            onClicked: (tapPoint) =>{
                gameDelegate.GridView.view.currentIndex = gameDelegate.index;
                gridRoot.gameClicked(gameDelegate.model.entryId, gameDelegate.index, tapPoint.modifiers);
            }

            onLaunchRequested: (entryId, contentHash, platformId, playable, statusText) => {
                gridRoot.requestLaunch(entryId, contentHash, platformId, playable, statusText);
            }
        }
    }
}
