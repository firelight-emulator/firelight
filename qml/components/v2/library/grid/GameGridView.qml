import QtQuick
import QtQuick.Controls

Item {
    id: gridRoot
    property var model: null

    // Multi-select state owned by GameView and bound in
    property var selectedIds: ({})

    // Section grouping mode ("none" = ungrouped); when set, the grid swaps to a
    // sectioned layout (a list of group headers each over a flow of tiles)
    property string groupBy: "none"
    // Distinct group labels in display order, supplied by GameView
    property var groupKeys: []
    // key -> count, used to give each section a known height so the outer list
    // only realizes on-screen sections
    property var groupCounts: ({})

    signal gameClicked(int entryId, int rowIndex, int modifiers)
    signal gameFocused(var data)
    signal requestAddToFolder(var entryIds)
    signal requestChangeArt(string contentHash, string displayName, int platformId)
    signal requestEditGame(int entryId, string contentHash, int platformId)

    // A plain snapshot of a game's fields for the detail panel (cheaper than
    // mirroring every row's full data just to show one)
    function focusSnapshot(m) {
        return {
            entryId: m.entryId,
            displayName: m.displayName,
            platformId: m.platformId,
            platformIconName: m.platformIconName,
            contentHash: m.contentHash,
            boxartFrontSourceUrl: m.boxartFrontSourceUrl,
            icon1x1SourceUrl: m.icon1x1SourceUrl,
            description: m.description,
            developer: m.developer,
            releaseYear: m.releaseYear,
            favorite: m.favorite,
            lastPlayedAt: m.lastPlayedAt,
            numSecondsPlayed: m.numSecondsPlayed,
            achievementsEarned: m.achievementsEarned,
            achievementsTotal: m.achievementsTotal,
            folderIds: m.folderIds,
            genres: m.genres
        };
    }

    // The entries a context action targets: the whole selection when the clicked
    // game is part of it, otherwise just that one game
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

    // A remote URL passes through; a local file path (user-imported art) becomes
    // a file:// URL so Image can load it
    function iconSource(u) {
        if (!u) {
            return "";
        }
        if (u.indexOf("://") >= 0) {
            return u;
        }
        return "file:///" + u.replace(/\\/g, "/");
    }

    GridView {
        id: root

        visible: gridRoot.groupBy === "none"
        model: gridRoot.groupBy === "none" ? gridRoot.model : null

        property real initialContentY: contentY

        property string sortRole: "displayName"
        property bool sortAscending: true

        width: Math.max(cellWidth, Math.floor(parent.width / cellWidth) * cellWidth)
        height: parent.height
        x: Math.round((parent.width - width) / 2)

        // User-controlled tile size (Settings → System → Display)
        cellWidth: AppearanceSettings.libraryTileSize
        cellHeight: AppearanceSettings.libraryTileSize

        Component.onCompleted: {
            initialContentY = contentY;
        }

        ScrollBar.vertical: FLScrollBar {
            anchors.left: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: AppStyle.spacingSm
        }
        boundsBehavior: Flickable.StopAtBounds
        reuseItems: true

        delegate: tileComponent
    }

    // TODO
    // Shared tile — used by the plain grid above and the grouped flow below
    Component {
        id: tileComponent

        Item {
            id: gameDelegate
            required property var model
            required property int index
            width: AppearanceSettings.libraryTileSize
            height: AppearanceSettings.libraryTileSize

            readonly property bool selected: gridRoot.selectedIds[gameDelegate.model.entryId] === true

            Button {
                id: control
                property bool showGlobalCursor: true
                anchors.fill: parent
                anchors.margins: AppStyle.spacingXs
                padding: 0
                hoverEnabled: true

                TapHandler {
                    id: selectTap
                    acceptedButtons: Qt.LeftButton
                    onSingleTapped: {
                        gridRoot.gameClicked(gameDelegate.model.entryId, gameDelegate.index, selectTap.point.modifiers);
                        gridRoot.gameFocused(gridRoot.focusSnapshot(gameDelegate.model));
                    }
                    onDoubleTapped: EmulationService.loadEntry(gameDelegate.model.entryId)
                }

                Keys.onPressed: function (event) {
                    if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter || event.key === Qt.Key_Select) {
                        EmulationService.loadEntry(gameDelegate.model.entryId);
                        event.accepted = true;
                    }
                }

                ContextMenu.menu: GameContextMenu {
                    primaryEntryId: gameDelegate.model.entryId
                    primaryFavorite: gameDelegate.model.favorite
                    contentHash: gameDelegate.model.contentHash
                    displayName: gameDelegate.model.displayName
                    platformId: gameDelegate.model.platformId
                    targetIds: gridRoot.targetsFor(gameDelegate.model.entryId)
                    onRequestAddToFolder: entryIds => gridRoot.requestAddToFolder(entryIds)
                    onRequestChangeArt: (h, n, p) => gridRoot.requestChangeArt(h, n, p)
                    onRequestEditGame: (id, h, p) => gridRoot.requestEditGame(id, h, p)
                }

                transform: Translate {
                    y: control.hovered ? -2 : 0

                    Behavior on y {
                        NumberAnimation {
                            duration: AppStyle.durationFast
                            easing.type: Easing.InOutQuad
                        }
                    }
                }

                background: Item {}
                contentItem: GameTile {
                    source: gridRoot.iconSource(gameDelegate.model.icon1x1SourceUrl)
                    size: AppearanceSettings.libraryTileSize
                    platformId: gameDelegate.model.platformId
                    title: gameDelegate.model.displayName
                    titleVisible: control.hovered || control.activeFocus
                }

                // Status badges — always visible, so the wall of art carries
                // favorite / unplayed / achievement signal at a glance
                Item {
                    z: 2
                    anchors.fill: parent
                    anchors.margins: AppStyle.spacingXs

                    Row {
                        anchors.left: parent.left
                        anchors.top: parent.top
                        spacing: AppStyle.spacingXs

                        // Unplayed dot
                        Rectangle {
                            visible: gameDelegate.model.lastPlayedAt === 0
                            width: Math.round(9 * AppStyle.scale)
                            height: width
                            radius: width / 2
                            anchors.verticalCenter: parent.verticalCenter
                            color: Theme.accent
                            border.color: "#66000000"
                            border.width: 1
                        }

                        // Favorite heart chip
                        Rectangle {
                            visible: gameDelegate.model.favorite
                            width: Math.round(22 * AppStyle.scale)
                            height: width
                            radius: AppStyle.radiusSm
                            color: "#99000000"
                            Icon {
                                anchors.centerIn: parent
                                name: "favorite"
                                filled: true
                                size: Math.round(14 * AppStyle.scale)
                                color: Theme.favorite
                            }
                        }
                    }

                    // Achievement progress pill (top-right)
                    Rectangle {
                        id: achPill
                        readonly property bool done: gameDelegate.model.achievementsTotal > 0 && gameDelegate.model.achievementsEarned >= gameDelegate.model.achievementsTotal
                        visible: gameDelegate.model.achievementsTotal > 0
                        anchors.right: parent.right
                        anchors.top: parent.top
                        height: Math.round(18 * AppStyle.scale)
                        width: achLabel.implicitWidth + Math.round(10 * AppStyle.scale)
                        radius: height / 2
                        color: achPill.done ? Theme.gold : "#aa000000"
                        Text {
                            id: achLabel
                            anchors.centerIn: parent
                            text: gameDelegate.model.achievementsEarned + "/" + gameDelegate.model.achievementsTotal
                            color: achPill.done ? Theme.onAccent : "white"
                            font.family: AppStyle.fontFamily
                            font.pixelSize: AppStyle.fontSizeSmall
                            font.weight: Font.DemiBold
                        }
                    }
                }

                // Multi-select ring
                Rectangle {
                    z: 3
                    anchors.fill: parent
                    visible: gameDelegate.selected
                    color: "transparent"
                    radius: AppStyle.radiusLg
                    border.color: Theme.accent
                    border.width: Math.max(2, Math.round(2 * AppStyle.scale))
                }
            }
        }
    }

    // Grouped: a virtualized list of sections, each a header over a flow of tiles.
    // The outer list virtualizes groups; a per-group proxy feeds each flow
    ListView {
        id: groupedList
        anchors.fill: parent
        visible: gridRoot.groupBy !== "none"
        model: visible ? gridRoot.groupKeys : []
        clip: true
        spacing: AppStyle.spacingMd
        boundsBehavior: Flickable.StopAtBounds
        cacheBuffer: Math.round(AppearanceSettings.libraryTileSize * 2)

        ScrollBar.vertical: FLScrollBar {
            width: AppStyle.spacingSm
        }

        // A section with a KNOWN height (header + tile rows), so the outer list
        // virtualizes sections instead of realizing them all at once
        delegate: Item {
            id: groupCol
            required property string modelData
            readonly property int tileSize: AppearanceSettings.libraryTileSize
            readonly property int headerH: Math.round(34 * AppStyle.scale)
            readonly property int cols: Math.max(1, Math.floor(groupedList.width / tileSize))
            readonly property int count: gridRoot.groupCounts[modelData] || 0
            width: groupedList.width
            height: headerH + Math.ceil(count / cols) * tileSize

            Text {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                height: groupCol.headerH
                leftPadding: AppStyle.spacingSm
                verticalAlignment: Text.AlignVCenter
                text: groupCol.modelData
                color: Theme.textMuted
                font.family: AppStyle.fontFamily
                font.pixelSize: AppStyle.fontSizeSmall
                font.weight: Font.DemiBold
                font.capitalization: Font.AllUppercase
                font.letterSpacing: 0.5
            }

            SortFilterProxyModel {
                id: groupProxy
                model: gridRoot.model
                filters: ValueFilter {
                    roleName: "groupKey"
                    value: groupCol.modelData
                }
            }

            Flow {
                anchors.top: parent.top
                anchors.topMargin: groupCol.headerH
                anchors.left: parent.left
                anchors.right: parent.right
                Repeater {
                    model: groupProxy
                    delegate: tileComponent
                }
            }
        }
    }
}
