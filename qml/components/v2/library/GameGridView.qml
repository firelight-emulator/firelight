import QtQuick
import QtQuick.Controls
import QtQuick.Window
import Firelight 1.0

Item {
    id: gridRoot
    property alias model: root.model

    // Multi-select state owned by GameView and bound in
    property var selectedIds: ({})

    signal gameClicked(int entryId, int rowIndex, int modifiers)
    signal gameFocused(var data)
    signal requestAddToFolder(var entryIds)
    signal requestChangeArt(string contentHash, string displayName, int platformId)

    // A plain snapshot of a game's fields for the detail panel (cheaper than
    // mirroring every row's full data just to show one)
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

    // The entries a context action targets: the whole selection when the clicked
    // game is part of it, otherwise just that one game
    function targetsFor(entryId) {
        if (gridRoot.selectedIds[entryId] === true) {
            var out = [];
            for (var k in gridRoot.selectedIds)
                if (gridRoot.selectedIds[k]) out.push(parseInt(k));
            if (out.length > 0) return out;
        }
        return [entryId];
    }

    // A remote URL passes through; a local file path (user-imported art) becomes
    // a file:// URL so Image can load it
    function iconSource(u) {
        if (!u)
            return ""
        if (u.indexOf("://") >= 0)
            return u
        return "file:///" + u.replace(/\\/g, "/")
    }

    GridView {
        id: root

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
            initialContentY = contentY
        }

        ScrollBar.vertical: ScrollBar {
            anchors.left: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 8

        }
         boundsBehavior: Flickable.StopAtBounds

        delegate: Item {
            id: gameDelegate
            required property var model
            required property int index
            width: root.cellWidth
            height: root.cellHeight

            readonly property bool selected: gridRoot.selectedIds[gameDelegate.model.entryId] === true

            Button {
            id: control
                anchors.fill: parent
                anchors.margins: 4
                padding: 0
                hoverEnabled: true

                TapHandler {
                    id: selectTap
                    acceptedButtons: Qt.LeftButton
                    onSingleTapped: {
                        gridRoot.gameClicked(gameDelegate.model.entryId, gameDelegate.index, selectTap.point.modifiers)
                        gridRoot.gameFocused(gridRoot.focusSnapshot(gameDelegate.model))
                    }
                    onDoubleTapped: EmulationService.loadEntry(gameDelegate.model.entryId)
                }

                Keys.onPressed: function (event) {
                    if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter || event.key === Qt.Key_Select) {
                        EmulationService.loadEntry(gameDelegate.model.entryId)
                        event.accepted = true
                    }
                }

                ContextMenu.menu: GameContextMenu {
                    primaryEntryId: gameDelegate.model.entryId
                    primaryFavorite: gameDelegate.model.favorite
                    contentHash: gameDelegate.model.contentHash
                    displayName: gameDelegate.model.displayName
                    platformId: gameDelegate.model.platformId
                    targetIds: gridRoot.targetsFor(gameDelegate.model.entryId)
                    onRequestAddToFolder: (entryIds) => gridRoot.requestAddToFolder(entryIds)
                    onRequestChangeArt: (h, n, p) => gridRoot.requestChangeArt(h, n, p)
                }

                scale: hovered ? 1.05 : 1
                Behavior on scale {
                    NumberAnimation {
                        duration: 120
                        easing.type: Easing.InOutQuad
                    }
                }

                background: Rectangle {
                    color: control.down ? "#FFFFFF" : "transparent"
                    opacity: control.down ? 0.05 : 0
                    radius: 4
                }
                contentItem: GameTile {
                    source: gridRoot.iconSource(gameDelegate.model.icon1x1SourceUrl)
                    size: root.cellWidth
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
                                color: "#e55aa2"
                            }
                        }
                    }

                    // Achievement progress pill (top-right)
                    Rectangle {
                        id: achPill
                        readonly property bool done: gameDelegate.model.achievementsTotal > 0
                                                     && gameDelegate.model.achievementsEarned >= gameDelegate.model.achievementsTotal
                        visible: gameDelegate.model.achievementsTotal > 0
                        anchors.right: parent.right
                        anchors.top: parent.top
                        height: Math.round(18 * AppStyle.scale)
                        width: achLabel.implicitWidth + Math.round(10 * AppStyle.scale)
                        radius: height / 2
                        color: achPill.done ? "#cc9a6b12" : "#aa000000"
                        Text {
                            id: achLabel
                            anchors.centerIn: parent
                            text: gameDelegate.model.achievementsEarned + "/" + gameDelegate.model.achievementsTotal
                            color: "white"
                            font.family: Constants.regularFontFamily
                            font.pixelSize: AppStyle.fontSizeSmall
                            font.weight: Font.DemiBold
                        }
                    }
                }

                // Title reveal: pristine art at rest, a scrim + name on hover/focus
                Rectangle {
                    z: 2
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    height: Math.round(parent.height * 0.55)
                    // Uniform radius, not per-corner: a Rectangle ignores its
                    // gradient when per-corner radii are set. The top corners are
                    // transparent in the gradient, so rounding them is invisible
                    radius: AppStyle.radiusMd
                    opacity: control.hovered || control.activeFocus ? 1 : 0
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "transparent" }
                        GradientStop { position: 1.0; color: "#dd000000" }
                    }
                    Behavior on opacity { NumberAnimation { duration: 120; easing.type: Easing.OutQuad } }

                    Text {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        anchors.margins: AppStyle.spacingSm
                        text: gameDelegate.model.displayName
                        color: "white"
                        font.family: Constants.regularFontFamily
                        font.pixelSize: AppStyle.fontSizeSmall
                        font.weight: Font.DemiBold
                        elide: Text.ElideRight
                        maximumLineCount: 2
                        wrapMode: Text.WordWrap
                    }
                }

                // Multi-select ring
                Rectangle {
                    z: 3
                    anchors.fill: parent
                    visible: gameDelegate.selected
                    color: "transparent"
                    radius: AppStyle.radiusMd
                    border.color: Theme.accent
                    border.width: Math.max(2, Math.round(2 * AppStyle.scale))
                }
            }
        }

     }
 }
