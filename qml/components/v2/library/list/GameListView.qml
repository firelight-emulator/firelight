import QtQuick
import QtQuick.Controls
import QtQuick.Layouts 1.0

// TODO
// The library list view. No column header — each cell is self-describing (the
// sort lives in the Display menu). Columns: art + name/platform, an achievement
// trophy+bar+count, and a play-activity summary
ListView {
    id: root

    property real initialContentY: contentY

    property string sortRole: "displayName"
    property bool sortAscending: true

    // TODO
    // Section grouping mode ("none" = ungrouped); the model's groupKey role
    // supplies the section value
    property string groupBy: "none"

    section.property: root.groupBy !== "none" ? "groupKey" : ""
    section.criteria: ViewSection.FullString
    section.delegate: Rectangle {
        required property string section
        width: ListView.view.width
        height: Math.round(38 * AppStyle.scale)
        color: Theme.backgroundInset

        Text {
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: AppStyle.spacingSm
            anchors.rightMargin: AppStyle.spacingSm
            text: parent.section
            color: Theme.textMuted
            font.family: AppStyle.fontFamily
            font.pixelSize: AppStyle.fontSizeSmall
            font.weight: Font.DemiBold
            font.capitalization: Font.AllUppercase
            font.letterSpacing: 0.5
            elide: Text.ElideRight
        }
    }

    // TODO
    // Multi-select state owned by GameView and bound in
    property var selectedIds: ({})

    signal gameClicked(int entryId, int rowIndex, int modifiers)
    signal gameFocused(var data)
    signal requestAddToFolder(var entryIds)
    signal requestChangeArt(string contentHash, string displayName, int platformId)
    signal requestEditGame(int entryId, string contentHash, int platformId)

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

    function targetsFor(entryId) {
        if (root.selectedIds[entryId] === true) {
            var out = [];
            for (var k in root.selectedIds) {
                if (root.selectedIds[k]) {
                    out.push(parseInt(k));
                }
            }
            if (out.length > 0) {
                return out;
            }
        }
        return [entryId];
    }

    readonly property int activityColumnWidth: Math.round(180 * AppStyle.scale)
    readonly property int achievementsColumnWidth: Math.round(150 * AppStyle.scale)
    readonly property int ratingColumnWidth: Math.round(100 * AppStyle.scale)

    Component.onCompleted: {
        initialContentY = contentY;
    }

    ScrollBar.vertical: FLScrollBar {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: AppStyle.spacingLg
    }
    boundsBehavior: Flickable.StopAtBounds

    delegate: GameListViewItem {
        width: ListView.view.width
        selectedIds: root.selectedIds
        achievementsColumnWidth: root.achievementsColumnWidth
        activityColumnWidth: root.activityColumnWidth
        ratingColumnWidth: root.ratingColumnWidth

        onSingleTapped: function (entryId, index, modifiers) {
            root.gameClicked(entryId, index, modifiers);
            root.gameFocused(root.focusSnapshot(model));
        }

        onDoubleTapped: EmulationService.loadEntry(entryId)
        onRequestAddToFolder: entryIds => root.requestAddToFolder(entryIds)
        onRequestChangeArt: (h, n, p) => root.requestChangeArt(h, n, p)
        onRequestEditGame: (id, h, p) => root.requestEditGame(id, h, p)
    }

    // Button {
    //     id: delegateButton
    //     required property var model
    //     required property int index
    //     readonly property int _art: Math.round(48 * AppStyle.scale)
    //     readonly property bool selected: root.selectedIds[delegateButton.model.entryId] === true
    //     readonly property bool _hasAchievements: delegateButton.model.achievementsTotal > 0
    //     readonly property bool _achievementsDone: _hasAchievements && delegateButton.model.achievementsEarned >= delegateButton.model.achievementsTotal
    //     property bool showGlobalCursor: true
    //     height: Math.max(AppStyle.rowHeight, _art + AppStyle.spacingSm * 2)
    //     width: ListView.view.width
    //     padding: AppStyle.spacingSm
    //     hoverEnabled: true
    //
    //     TapHandler {
    //         id: rowTap
    //         acceptedButtons: Qt.LeftButton
    //         // TODO
    //         // Default (DragThreshold) policy takes only a passive grab, so it
    //         // cooperates with the row's folder-drag DragHandler
    //         onSingleTapped: {
    //             root.gameClicked(delegateButton.model.entryId, delegateButton.index, rowTap.point.modifiers);
    //             root.gameFocused(root.focusSnapshot(delegateButton.model));
    //         }
    //         onDoubleTapped: EmulationService.loadEntry(delegateButton.model.entryId)
    //     }
    //
    //     ContextMenu.menu: GameContextMenu {
    //         primaryEntryId: delegateButton.model.entryId
    //         primaryFavorite: delegateButton.model.favorite
    //         contentHash: delegateButton.model.contentHash
    //         displayName: delegateButton.model.displayName
    //         platformId: delegateButton.model.platformId
    //         targetIds: root.targetsFor(delegateButton.model.entryId)
    //         onRequestAddToFolder: entryIds => root.requestAddToFolder(entryIds)
    //         onRequestChangeArt: (h, n, p) => root.requestChangeArt(h, n, p)
    //         onRequestEditGame: (id, h, p) => root.requestEditGame(id, h, p)
    //     }
    //
    //     Keys.onPressed: function (event) {
    //         if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter || event.key === Qt.Key_Select) {
    //             EmulationService.loadEntry(delegateButton.model.entryId);
    //             event.accepted = true;
    //         }
    //     }
    //
    //     Drag.dragType: Drag.Automatic
    //     Drag.supportedActions: Qt.CopyAction
    //
    //     DragHandler {
    //         id: dragHandler
    //         target: null
    //         snapMode: DragHandler.SnapAlways
    //
    //         onActiveChanged: if (active) {
    //             parent.grabToImage(function (result) {
    //                 parent.Drag.imageSource = result.url;
    //                 parent.Drag.hotSpot.x = centroid.pressPosition.x;
    //                 parent.Drag.hotSpot.y = centroid.pressPosition.y;
    //                 parent.Drag.mimeData = {
    //                     "text/plain": model.entryId
    //                 };
    //                 parent.Drag.active = true;
    //             });
    //         } else {
    //             parent.Drag.active = false;
    //         }
    //     }
    //
    //     background: Rectangle {
    //         color: delegateButton.selected ? Theme.accent : Theme.textPrimary
    //         opacity: delegateButton.selected ? 0.16 : (delegateButton.hovered ? 0.08 : 0)
    //         radius: AppStyle.radiusSm
    //         border.color: Theme.accent
    //         border.width: delegateButton.selected ? Math.max(1, Math.round(AppStyle.scale)) : 0
    //     }
    //     RowLayout {
    //         anchors.fill: parent
    //         anchors.leftMargin: AppStyle.spacingSm
    //         anchors.rightMargin: AppStyle.spacingSm
    //         spacing: AppStyle.spacingLg
    //
    //         GameTile {
    //             source: root.iconSource(delegateButton.model.icon1x1SourceUrl)
    //             platformId: delegateButton.model.platformId
    //             size: delegateButton._art
    //         }
    //
    //         // TODO
    //         // Name + platform
    //         ColumnLayout {
    //             Layout.fillWidth: true
    //             Layout.fillHeight: true
    //             spacing: 0
    //             Text {
    //                 Layout.fillWidth: true
    //                 Layout.fillHeight: true
    //                 Layout.verticalStretchFactor: 1
    //                 text: delegateButton.model.displayName
    //                 font.pixelSize: AppStyle.fontSizeMedium
    //                 font.weight: Font.DemiBold
    //                 font.family: AppStyle.fontFamily
    //                 color: Theme.textPrimary
    //                 elide: Text.ElideRight
    //                 verticalAlignment: Text.AlignVCenter
    //             }
    //             Text {
    //                 Layout.fillWidth: true
    //                 Layout.fillHeight: true
    //                 Layout.verticalStretchFactor: 1
    //                 text: PlatformModel.getPlatformDisplayName(delegateButton.model.platformId)
    //                 font.pixelSize: AppStyle.fontSizeSmall
    //                 font.family: AppStyle.fontFamily
    //                 color: Theme.textMuted
    //                 elide: Text.ElideRight
    //                 verticalAlignment: Text.AlignVCenter
    //             }
    //         }
    //
    //         // TODO
    //         // Achievements: trophy + progress bar + earned/total. Fixed width and
    //         // always present (content hidden when the game has none) so the
    //         // columns line up down the list
    //         Item {
    //             Layout.preferredWidth: root.achievementsColumnWidth
    //             Layout.fillHeight: true
    //
    //             RowLayout {
    //                 anchors.verticalCenter: parent.verticalCenter
    //                 width: parent.width
    //                 spacing: AppStyle.spacingSm
    //                 visible: delegateButton._hasAchievements
    //
    //                 Icon {
    //                     name: "trophy"
    //                     filled: true
    //                     size: AppStyle.iconSizeSm
    //                     color: delegateButton._achievementsDone ? Theme.gold : Theme.textMuted
    //                     Layout.alignment: Qt.AlignVCenter
    //                 }
    //                 Rectangle {
    //                     Layout.fillWidth: true
    //                     Layout.alignment: Qt.AlignVCenter
    //                     height: Math.round(4 * AppStyle.scale)
    //                     radius: height / 2
    //                     color: Theme.backgroundInset
    //                     Rectangle {
    //                         width: parent.width * (delegateButton._hasAchievements ? delegateButton.model.achievementsEarned / delegateButton.model.achievementsTotal : 0)
    //                         height: parent.height
    //                         radius: height / 2
    //                         color: delegateButton._achievementsDone ? Theme.gold : Theme.accent
    //                     }
    //                 }
    //                 Text {
    //                     text: delegateButton.model.achievementsEarned + "/" + delegateButton.model.achievementsTotal + (delegateButton.model.achievementSetCount > 1 ? " (+" + (delegateButton.model.achievementSetCount - 1) + ")" : "")
    //                     font.pixelSize: AppStyle.fontSizeSmall
    //                     font.family: AppStyle.fontFamily
    //                     color: Theme.textMuted
    //                     Layout.alignment: Qt.AlignVCenter
    //                 }
    //             }
    //         }
    //
    //         // TODO
    //         // Play activity — self-describing (no header). fillWidth is forced
    //         // off: a ColumnLayout otherwise inherits it from its fillWidth Text
    //         // child and steals slack from the name column, misaligning the rows
    //         ColumnLayout {
    //             Layout.preferredWidth: root.activityColumnWidth
    //             Layout.fillWidth: false
    //             Layout.fillHeight: true
    //             spacing: 0
    //             Text {
    //                 Layout.fillWidth: true
    //                 Layout.fillHeight: true
    //                 Layout.verticalStretchFactor: 1
    //                 text: delegateButton.model.numSecondsPlayed > 0 ? root.formatPlayTime(delegateButton.model.numSecondsPlayed) : "Never played"
    //                 font.pixelSize: AppStyle.fontSizeSmall
    //                 font.weight: Font.Medium
    //                 font.family: AppStyle.fontFamily
    //                 color: Theme.textPrimary
    //                 elide: Text.ElideRight
    //                 verticalAlignment: Text.AlignVCenter
    //             }
    //             Text {
    //                 Layout.fillWidth: true
    //                 Layout.fillHeight: true
    //                 Layout.verticalStretchFactor: 1
    //                 visible: text !== ""
    //                 text: root.formatLastPlayed(delegateButton.model.lastPlayedAt)
    //                 font.pixelSize: AppStyle.fontSizeSmall
    //                 font.family: AppStyle.fontFamily
    //                 color: Theme.textMuted
    //                 elide: Text.ElideRight
    //                 verticalAlignment: Text.AlignVCenter
    //             }
    //         }
    //
    //         Item {
    //             Layout.preferredWidth: AppStyle.iconSizeMd
    //             Layout.preferredHeight: AppStyle.iconSizeMd
    //             Layout.alignment: Qt.AlignVCenter
    //             Icon {
    //                 anchors.centerIn: parent
    //                 name: "favorite"
    //                 filled: delegateButton.model.favorite
    //                 size: AppStyle.iconSizeMd
    //                 color: delegateButton.model.favorite ? Theme.favorite : Theme.textMuted
    //                 visible: delegateButton.model.favorite || delegateButton.hovered
    //             }
    //             TapHandler {
    //                 enabled: delegateButton.model.favorite || delegateButton.hovered
    //                 onTapped: delegateButton.model.favorite = !delegateButton.model.favorite
    //             }
    //             HoverHandler {
    //                 cursorShape: Qt.PointingHandCursor
    //             }
    //         }
    //         Item {
    //             Layout.preferredWidth: AppStyle.iconSizeMd
    //             Layout.preferredHeight: AppStyle.iconSizeMd
    //             Layout.alignment: Qt.AlignVCenter
    //             Icon {
    //                 anchors.centerIn: parent
    //                 name: "more-vertical"
    //                 size: AppStyle.iconSizeMd
    //                 color: Theme.textMuted
    //                 visible: delegateButton.hovered
    //             }
    //             TapHandler {
    //                 enabled: delegateButton.hovered
    //                 onTapped: delegateButton.ContextMenu.menu.popup()
    //             }
    //             HoverHandler {
    //                 cursorShape: Qt.PointingHandCursor
    //             }
    //         }
    //     }
    // }
}
