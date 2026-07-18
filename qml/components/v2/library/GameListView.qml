import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Window
import QtQml.Models
import QtQuick.Layouts 1.0
import QtNetwork
import QtMultimedia
import QtQuick.Effects
import Firelight 1.0

ListView {
    id: root

    property real initialContentY: contentY

    property string sortRole: "displayName"
    property bool sortAscending: true

    // Multi-select state owned by GameView and bound in
    property var selectedIds: ({})

    signal gameClicked(int entryId, int rowIndex, int modifiers)
    signal gameFocused(var data)
    signal requestAddToFolder(var entryIds)
    signal requestChangeArt(string contentHash, string displayName, int platformId)

    function focusSnapshot(m) {
        return {
            entryId: m.entryId, displayName: m.displayName, platformId: m.platformId,
            platformIconName: m.platformIconName, contentHash: m.contentHash,
            boxartFrontSourceUrl: m.boxartFrontSourceUrl, icon1x1SourceUrl: m.icon1x1SourceUrl,
            description: m.description, developer: m.developer, releaseYear: m.releaseYear,
            favorite: m.favorite, lastPlayedAt: m.lastPlayedAt, numSecondsPlayed: m.numSecondsPlayed,
            achievementsEarned: m.achievementsEarned, achievementsTotal: m.achievementsTotal,
            folderIds: m.folderIds, genres: m.genres
        };
    }

    function targetsFor(entryId) {
        if (root.selectedIds[entryId] === true) {
            var out = [];
            for (var k in root.selectedIds)
                if (root.selectedIds[k]) out.push(parseInt(k));
            if (out.length > 0) return out;
        }
        return [entryId];
    }

    function formatPlayTime(seconds) {
        if (!seconds || seconds <= 0) return "—";
        var h = Math.floor(seconds / 3600);
        var m = Math.round((seconds % 3600) / 60);
        if (h > 0) return h + "h " + m + "m";
        if (m > 0) return m + "m";
        return "<1m";
    }
    function formatLastPlayed(millis) {
        if (!millis || millis <= 0) return "Never";
        return Qt.formatDateTime(new Date(millis), "MMM d, yyyy");
    }
    function formatAchievements(earned, total) {
        if (!total || total <= 0) return "—";
        return earned + "/" + total;
    }

    property real titleColumnWidth: 340
    property real timePlayedColumnWidth: 128
    property real lastPlayedColumnWidth: 128
    property real achievementColumnWidth: 128

    Component.onCompleted: {
        initialContentY = contentY
    }

    ScrollBar.vertical: ScrollBar {
        anchors.left: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 8

    }
     boundsBehavior: Flickable.StopAtBounds

     headerPositioning: ListView.OverlayHeader
     header: Pane {
        z: 2
        padding: 0
        width: ListView.view.width
        background: Rectangle {
            color: Theme.surface
            topLeftRadius: 8
            topRightRadius: 8
            opacity: root.contentY > 0 ? 1 : 0
            width: parent.width + 48
            height: parent.height - 16
            anchors.horizontalCenter: parent.horizontalCenter

            Behavior on opacity {
                NumberAnimation {
                    duration: 160
                    easing.type: Easing.InOutQuad
                }
            }
        }
        contentItem: ColumnLayout {
             id: gameListHeader
             spacing: 8
             RowLayout {
                 Layout.fillWidth: true
                 Layout.topMargin: 8
                 Layout.leftMargin: 10
                 Layout.rightMargin: 10
                 Layout.minimumHeight: 32
                 Layout.maximumHeight: 32
                 spacing: 16

                 HoverHandler {
                     id: headerHoverHandler
                 }

                 Button {
                     padding: 0
                     implicitHeight: 24
                     implicitWidth: 24
                     Layout.alignment: Qt.AlignVCenter
                     icon.source: "qrc:/icons/favorite"
                     icon.width: 24
                     icon.height: 24
                     icon.color: Theme.textMuted
                     background: Item {}
                 }

                 Item {
                     implicitWidth: 48
                     implicitHeight: 1
                 }

                 SplitView {
                     Layout.fillWidth: true
                     Layout.fillHeight: true
                     padding: 0

                     handle: Item {
                         implicitWidth: 16
                         SplitView.fillHeight: true

                         Rectangle {
                             anchors.centerIn: parent
                             width: 1
                             height: parent.height - 8
                             color: Theme.textPrimary
                             opacity: headerHoverHandler.hovered ? 0.12 : 0
                         }
                     }

                     ListViewColumnHeader {
                         id: titleHeader
                         text: "Title"
                         selected: root.sortRole === "displayName"
                         sortAscending: root.sortAscending

                         SplitView.fillHeight: true
                         SplitView.minimumWidth: 256
                         SplitView.preferredWidth: 340
                         SplitView.fillWidth: true

                         onWidthChanged: {
                             root.titleColumnWidth = width
                         }

                         onTapped: {
                             if (root.sortRole === "displayName") {
                                 root.sortAscending = !root.sortAscending
                             } else {
                                 root.sortAscending = true
                                 root.sortRole = "displayName"
                             }
                         }
                     }

                     ListViewColumnHeader {
                         id: timePlayedHeader
                         text: "Time played"
                         selected: root.sortRole === "platformId"
                         sortAscending: root.sortAscending

                         SplitView.fillHeight: true
                         SplitView.minimumWidth: 128
                         SplitView.preferredWidth: 128

                         onWidthChanged: {
                             root.timePlayedColumnWidth = width
                         }

                         onTapped: {
                             if (root.sortRole === "platformId") {
                                 root.sortAscending = !root.sortAscending
                             } else {
                                 root.sortAscending = true
                                 root.sortRole = "platformId"
                             }
                         }
                     }

                     ListViewColumnHeader {
                         id: lastPlayedAtHeader
                         text: "Last played"
                         selected: root.sortRole === "lastPlayedAt"
                         sortAscending: root.sortAscending

                         SplitView.fillHeight: true
                         SplitView.minimumWidth: 128
                         SplitView.preferredWidth: 128

                         onWidthChanged: {
                             root.lastPlayedColumnWidth = width
                         }

                         onTapped: {
                             if (root.sortRole === "lastPlayedAt") {
                                 root.sortAscending = !root.sortAscending
                             } else {
                                 root.sortAscending = true
                                 root.sortRole = "lastPlayedAt"
                             }
                         }
                     }

                     Text {
                         id: achievementHeader
                         SplitView.fillHeight: true
                         SplitView.minimumWidth: 128
                         SplitView.preferredWidth: 128
                         onWidthChanged: {
                             gamesPanel.achievementColumnWidth = width
                         }
                         text: "Achievements"
                         font.pixelSize: AppStyle.fontSizeMedium
                         font.weight: Font.DemiBold
                         font.family: Constants.regularFontFamily
                         color: Theme.textMuted
                         horizontalAlignment: Text.AlignLeft
                         verticalAlignment: Text.AlignVCenter
                     }
                 }

                 Item {
                     implicitWidth: 32
                     implicitHeight: 1
                 }
             }

             Rectangle {
                 Layout.fillWidth: true
                 implicitHeight: 1
                 color: Theme.textPrimary
                 opacity: 0.12
             }
             Item {
                 implicitWidth: 1
                 implicitHeight: 8
             }
         }
     }
     delegate: Button {
        id: delegateButton
        required property var model
        required property int index
         readonly property int _art: Math.round(48 * AppStyle.scale)
         readonly property bool selected: root.selectedIds[delegateButton.model.entryId] === true
         height: Math.max(AppStyle.rowHeight, _art + AppStyle.spacingSm * 2)
         width: ListView.view.width
         padding: AppStyle.spacingSm
         hoverEnabled: true

         TapHandler {
             id: rowTap
             acceptedButtons: Qt.LeftButton
             // Default (DragThreshold) policy takes only a passive grab, so it
             // cooperates with the row's folder-drag DragHandler
             onSingleTapped: {
                 root.gameClicked(delegateButton.model.entryId, delegateButton.index, rowTap.point.modifiers)
                 root.gameFocused(root.focusSnapshot(delegateButton.model))
             }
             onDoubleTapped: EmulationService.loadEntry(delegateButton.model.entryId)
         }

         ContextMenu.menu: GameContextMenu {
             primaryEntryId: delegateButton.model.entryId
             primaryFavorite: delegateButton.model.favorite
             contentHash: delegateButton.model.contentHash
             displayName: delegateButton.model.displayName
             platformId: delegateButton.model.platformId
             targetIds: root.targetsFor(delegateButton.model.entryId)
             onRequestAddToFolder: (entryIds) => root.requestAddToFolder(entryIds)
             onRequestChangeArt: (h, n, p) => root.requestChangeArt(h, n, p)
         }

         Keys.onPressed: function (event) {
             if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter || event.key === Qt.Key_Select) {
                 EmulationService.loadEntry(delegateButton.model.entryId)
                 event.accepted = true
             }
         }

         Drag.dragType: Drag.Automatic
         Drag.supportedActions: Qt.CopyAction
         Drag.mimeData: {
             "text/plain": "Copied text"
         }

         DragHandler {
             id: dragHandler
             target: null
             snapMode: DragHandler.SnapAlways

             onActiveChanged:
                 if (active) {
                     parent.grabToImage(function(result) {
                         parent.Drag.imageSource = result.url
                         parent.Drag.hotSpot.x = centroid.pressPosition.x
                         parent.Drag.hotSpot.y = centroid.pressPosition.y
                         parent.Drag.mimeData = {
                             "text/plain": model.entryId
                         }
                         parent.Drag.active = true
                     })
                 } else {
                     parent.Drag.active = false
                 }
         }

         background: Rectangle {
             color: delegateButton.selected ? Theme.accent : Theme.textPrimary
             opacity: delegateButton.selected ? 0.16 : (delegateButton.hovered ? 0.08 : 0)
             radius: AppStyle.radiusSm
             border.color: Theme.accent
             border.width: delegateButton.selected ? Math.max(1, Math.round(AppStyle.scale)) : 0
         }
         contentItem: RowLayout {
             spacing: AppStyle.spacingLg

            GameTile {
                source: delegateButton.model.icon1x1SourceUrl
                size: delegateButton._art
            }

             ColumnLayout {
                 Layout.minimumWidth: root.titleColumnWidth
                 Layout.maximumWidth: root.titleColumnWidth
                 Layout.fillHeight: true
                 Text {
                     Layout.fillWidth: true
                     Layout.fillHeight: true
                     Layout.verticalStretchFactor: 1
                     text: model.displayName
                     font.pixelSize: AppStyle.fontSizeMedium
                     font.weight: Font.DemiBold
                     font.family: Constants.regularFontFamily
                     color: Theme.textPrimary
                     elide: Text.ElideRight
                 }
                 Text {
                     Layout.fillWidth: true
                     Layout.fillHeight: true
                     Layout.verticalStretchFactor: 1
                     text: model.platformId
                     font.pixelSize: AppStyle.fontSizeMedium
                     font.weight: Font.DemiBold
                     font.family: Constants.regularFontFamily
                     color: Theme.textMuted
                     elide: Text.ElideRight
                     }
             }

             Item {
                 width: 24
                 height: 24

                 Button {
                      padding: 0
                      anchors.fill: parent
                      icon.source: model.favorite ? "qrc:/icons/favorite" : "qrc:/icons/empty/favorite"
                      visible: model.favorite || delegateButton.hovered
                     icon.width: 24
                     icon.height: 24
                     icon.color: model.favorite ? Qt.darker("#e55aa2", 1.3) : "#9c9c9c"
                     background: Item {}
                     HoverHandler {
                          cursorShape: Qt.PointingHandCursor
                     }
                     onClicked: {
                        model.favorite = !model.favorite
                     }
                  }
             }

             Text {
                 Layout.fillHeight: true
                 Layout.minimumWidth: root.timePlayedColumnWidth
                 Layout.maximumWidth: root.timePlayedColumnWidth
                 text: root.formatPlayTime(delegateButton.model.numSecondsPlayed)
                 font.pixelSize: AppStyle.fontSizeMedium
                 font.weight: Font.Medium
                 font.family: Constants.regularFontFamily
                 color: Theme.textMuted
                 elide: Text.ElideRight
                 verticalAlignment: Text.AlignVCenter
             }
             Text {
                 Layout.fillHeight: true
                 Layout.minimumWidth: root.lastPlayedColumnWidth
                 Layout.maximumWidth: root.lastPlayedColumnWidth
                 text: root.formatLastPlayed(delegateButton.model.lastPlayedAt)
                 font.pixelSize: AppStyle.fontSizeMedium
                 font.weight: Font.Medium
                 font.family: Constants.regularFontFamily
                 color: Theme.textMuted
                 elide: Text.ElideRight
                 verticalAlignment: Text.AlignVCenter
             }

             Text {
                 Layout.fillHeight: true
                 Layout.minimumWidth: root.achievementColumnWidth
                 Layout.maximumWidth: root.achievementColumnWidth
                 text: root.formatAchievements(delegateButton.model.achievementsEarned, delegateButton.model.achievementsTotal)
                 font.pixelSize: AppStyle.fontSizeMedium
                 font.weight: Font.Medium
                 font.family: Constants.regularFontFamily
                 color: Theme.textMuted
                 elide: Text.ElideRight
                 verticalAlignment: Text.AlignVCenter
             }
             Button {
                 padding: 0
                 implicitHeight: 32
                 implicitWidth: 32
                 icon.source: "qrc:/icons/more-vertical"
                icon.width: 32
                icon.height: 32
                icon.color: Theme.textMuted
                background: Item {}
                visible: delegateButton.hovered

             }

             Item {
                 Layout.fillWidth: true
                 Layout.fillHeight: true
             }

        }
    }
 }
