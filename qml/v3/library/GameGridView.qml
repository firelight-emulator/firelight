// TODO: NEEDS REVIEW
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

Item {
    id: gridRoot
    property var model: null
    required property string currentSortLabel
    required property bool sortAscending

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

    function positionViewAtBeginning() {
        root.positionViewAtBeginning();
    }

    // TODO
    // Room under the art for the title strip. Counted once, because the cell reserves it and the
    // strip fills it, and the two disagreeing takes the difference out of the gap between rows
    readonly property int labelHeight: Math.round(60 * AppStyle.scale)

    signal gameClicked(int entryId, int rowIndex, int modifiers)
    signal gameFocused(var data)
    signal requestAddToFolder(var entryIds)
    signal requestChangeArt(string contentHash, string displayName, int platformId)
    signal requestEditGame(int entryId, string contentHash, int platformId)
    signal requestLaunch(int entryId, string contentHash, int platformId, bool playable, string statusText)

    property alias header: root.header

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
            playable: m.playable,
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

    FLGridView {
        id: root

        visible: gridRoot.groupBy === "none"
        model: gridRoot.groupBy === "none" ? gridRoot.model : null

        displayMarginBeginning: Math.round(cellHeight / 2)

        property real initialContentY: contentY

        property string sortRole: "displayName"
        property bool sortAscending: true

        width: Math.max(1, Math.floor(parent.width / cellWidth)) * cellWidth
        height: parent.height
        x: Math.round((parent.width - width) / 2)

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

        Component.onCompleted: {
            initialContentY = contentY;
        }

        ScrollBar.vertical: FLScrollBar {
            anchors.right: root.right
            anchors.rightMargin: -32
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: AppStyle.spacingMd
        }
        boundsBehavior: Flickable.StopAtBounds
        reuseItems: true

        delegate: tileComponent
    }

    // TODO
    // Shared tile — used by the plain grid above and the grouped flow below
    Component {
        id: tileComponent

        FocusScope {
            id: gameDelegate
            required property var model
            required property int index

            width: GridView.view.cellWidth
            height: GridView.view.cellHeight

            readonly property bool selected: gridRoot.selectedIds[gameDelegate.model.entryId] === true

            Button {
                id: control
                anchors.fill: parent
                // TODO
                // Half the gap on each side, so two neighbours make one whole gap between them and
                // the tile itself keeps the size it was asked for whatever the spacing is
                anchors.margins: AppearanceSettings.libraryIconGridTileSpacing / 2
                padding: 0
                horizontalPadding: 0
                hoverEnabled: true
                focus: true

                FLFocus.showCursor: true
                FLFocus.spacing: 3
                FLFocus.fill: "black"
                FLFocus.focusSound: SoundEffects.gameTileFocus
                FLFocus.radius: gameTile.radius + Math.round(FLFocus.spacing / 2)

                TapHandler {
                    id: selectTap
                    acceptedButtons: Qt.LeftButton
                    onSingleTapped: {
                        gridRoot.gameClicked(gameDelegate.model.entryId, gameDelegate.index, selectTap.point.modifiers);
                        gridRoot.gameFocused(gridRoot.focusSnapshot(gameDelegate.model));
                    }
                    onDoubleTapped: EmulationService.loadEntry(gameDelegate.model.entryId)
                }

                ContextMenu.menu: FLMenu {
                    id: delegateContextMenu

                    FLMenuItem {
                        label: qsTr("Play")
                    }

                    FLMenuItem {
                        label: "Resume last session"
                    }

                    FLMenuSeparator {}

                    FLMenuItem {
                        label: "View details"
                    }

                    FLToggleMenuItem {
                        label: "Favorite"
                        checked: gameDelegate.model.favorite
                        onSelected: function (selected) {
                            gameDelegate.model.favorite = selected;
                        }
                    }

                    FLToggleMenuItem {
                        label: "Hidden"
                        checked: gameDelegate.model.hidden
                        onSelected: function (selected) {
                            gameDelegate.model.hidden = selected;
                        }
                    }

                    FLMenuItem {
                        label: "Manage game"
                    }
                }

                FLFocus.actions: [
                    FLAction {
                        keys: [Qt.Key_Select, Qt.Key_Return, Qt.Key_Enter, Qt.Key_Space]
                        label: qsTr("Play")
                        sound: SoundEffects.activateGame
                        onTriggered: activatedAnimation.restart()
                    },
                    FLAction {
                        keys: [Qt.Key_Menu]
                        label: qsTr("Menu")
                        onTriggered: delegateContextMenu.popupFor(control, control.width + AppStyle.spacingSm, 0)

                    }
                ]

                SequentialAnimation {
                    id: activatedAnimation
                    ParallelAnimation {
                        NumberAnimation {
                            target: control
                            property: "scale"
                            from: 1.0
                            to: 0.95
                            duration: AppStyle.durationVeryFast
                            easing.type: Easing.InOutQuad
                        }
                    }
                    NumberAnimation {
                        target: control
                        property: "scale"
                        from: 0.95
                        to: 1.0
                        duration: AppStyle.durationVeryFast
                        easing.type: Easing.InOutQuad
                    }
                    PauseAnimation {
                        duration: AppStyle.durationBase
                    }
                    ScriptAction {
                        script: {
                            gridRoot.requestLaunch(gameDelegate.model.entryId, gameDelegate.model.contentHash, gameDelegate.model.platformId, gameDelegate.model.playable, gameDelegate.model.statusText);
                        }
                    }
                }

                // ContextMenu.menu: GameContextMenu {
                //     primaryEntryId: gameDelegate.model.entryId
                //     primaryFavorite: gameDelegate.model.favorite
                //     contentHash: gameDelegate.model.contentHash
                //     displayName: gameDelegate.model.displayName
                //     platformId: gameDelegate.model.platformId
                //     targetIds: gridRoot.targetsFor(gameDelegate.model.entryId)
                //     onRequestAddToFolder: entryIds => gridRoot.requestAddToFolder(entryIds)
                //     onRequestChangeArt: (h, n, p) => gridRoot.requestChangeArt(h, n, p)
                //     onRequestEditGame: (id, h, p) => gridRoot.requestEditGame(id, h, p)
                // }

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
                contentItem: Column {
                    GameTile {
                        id: gameTile
                        source: FLUtil.toUrl(gameDelegate.model.icon1x1SourceUrl)
                        size: control.width
                        topLeftRadius: AppStyle.radiusMd
                        topRightRadius: AppStyle.radiusMd
                        bottomLeftRadius: titleBox.height > 0 ? 0 : AppStyle.radiusMd
                        bottomRightRadius: titleBox.height > 0 ? 0 : AppStyle.radiusMd
                        platformId: gameDelegate.model.platformId
                        title: gameDelegate.model.displayName
                        titleVisible: control.hovered || control.activeFocus
                    }

                    Rectangle {
                        id: titleBox
                        width: parent.width
                        height: root._showTitleBox ? gridRoot.labelHeight : 0

                        Behavior on height {
                            NumberAnimation {
                                duration: AppStyle.durationBase
                                easing.type: Easing.InOutQuad
                            }
                        }

                        color: Theme.surface
                        topLeftRadius: 0
                        topRightRadius: 0
                        bottomRightRadius: AppStyle.radiusMd
                        bottomLeftRadius: AppStyle.radiusMd

                        Icon {
                            id: favoriteIcon
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            anchors.margins: AppStyle.spacingSm
                            name: "favorite"
                            filled: true
                            visible: gameDelegate.model.favorite && AppearanceSettings.libraryIconGridShowFavoriteIcon
                            size: Math.round(16 * AppStyle.scale)
                            color: Theme.favorite
                        }

                        Text {
                            anchors.left: parent.left
                            anchors.right: favoriteIcon.visible ? favoriteIcon.left : parent.right
                            anchors.bottom: parent.bottom
                            anchors.top: parent.top
                            anchors.margins: AppStyle.spacingSm
                            text: gameDelegate.model.displayName
                            color: Theme.textPrimary
                            font.family: AppStyle.fontFamily
                            font.pixelSize: AppStyle.fontSizeSmall
                            font.weight: Font.DemiBold
                            elide: Text.ElideRight
                            wrapMode: Text.WordWrap
                            maximumLineCount: 2
                        }
                    }
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

                        // No core installed for this platform, so the game is here but
                        // cannot start
                        Rectangle {
                            visible: !gameDelegate.model.playable
                            width: Math.round(22 * AppStyle.scale)
                            height: width
                            radius: AppStyle.radiusSm
                            color: "#99000000"
                            Icon {
                                anchors.centerIn: parent
                                name: "cancel"
                                size: Math.round(14 * AppStyle.scale)
                                color: Theme.textMuted
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
                // Rectangle {
                //     z: 3
                //     anchors.fill: parent
                //     visible: gameDelegate.selected
                //     color: "transparent"
                //     radius: AppStyle.radiusLg
                //     border.color: Theme.accent
                //     border.width: Math.max(2, Math.round(2 * AppStyle.scale))
                // }
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
        // clip: true
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
