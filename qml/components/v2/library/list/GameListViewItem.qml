import QtQuick
import QtQuick.Controls
import QtQuick.Layouts 1.0

Button {
    id: control
    required property var model
    required property int index

    required property var selectedIds
    required property int achievementsColumnWidth
    required property int activityColumnWidth
    required property int ratingColumnWidth

    readonly property int _art: Math.round(48 * AppStyle.scale)
    readonly property bool selected: control.selectedIds[control.model.entryId] === true
    readonly property bool _hasAchievements: control.model.achievementsTotal > 0
    readonly property bool _achievementsDone: _hasAchievements && control.model.achievementsEarned >= control.model.achievementsTotal
    property bool showGlobalCursor: true
    height: Math.max(AppStyle.rowHeight, _art + AppStyle.spacingSm * 2)
    padding: AppStyle.spacingSm
    hoverEnabled: true

    signal singleTapped(string entryId, int index, int modifiers)
    signal doubleTapped(string entryId)

    signal requestAddToFolder(var entryIds)
    signal requestChangeArt(string contentHash, string displayName, int platformId)
    signal requestEditGame(string entryId, string contentHash, int platformId)

    TapHandler {
        id: rowTap
        acceptedButtons: Qt.LeftButton
        onSingleTapped: control.singleTapped(control.model.entryId, control.index, rowTap.point.modifiers)
        onDoubleTapped: control.doubleTapped(control.model.entryId)
    }

    ContextMenu.menu: GameContextMenu {
        primaryEntryId: control.model.entryId
        primaryFavorite: control.model.favorite
        contentHash: control.model.contentHash
        displayName: control.model.displayName
        platformId: control.model.platformId
        targetIds: control.selectedIds

        onRequestAddToFolder: control.requestAddToFolder
        onRequestChangeArt: control.requestChangeArt
        onRequestEditGame: control.requestEditGame
    }

    Keys.onPressed: function (event) {
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter || event.key === Qt.Key_Select) {
            control.doubleClicked(control.model.entryId)
            event.accepted = true;
        }
    }

    Drag.dragType: Drag.Automatic
    Drag.supportedActions: Qt.CopyAction

    DragHandler {
        id: dragHandler
        target: null
        snapMode: DragHandler.SnapAlways

        onActiveChanged: if (active) {
            parent.grabToImage(function (result) {
                parent.Drag.imageSource = result.url;
                parent.Drag.hotSpot.x = centroid.pressPosition.x;
                parent.Drag.hotSpot.y = centroid.pressPosition.y;
                parent.Drag.mimeData = {
                    "text/plain": model.entryId
                };
                parent.Drag.active = true;
            });
        } else {
            parent.Drag.active = false;
        }
    }

    background: Rectangle {
        color: control.selected ? Theme.accent : Theme.textPrimary
        opacity: control.selected ? 0.16 : (control.hovered ? 0.08 : 0)
        radius: AppStyle.radiusSm
        border.color: Theme.accent
        border.width: control.selected ? Math.max(1, Math.round(AppStyle.scale)) : 0
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: AppStyle.spacingSm
        anchors.rightMargin: AppStyle.spacingSm
        spacing: AppStyle.spacingLg

        GameTile {
            source: FLUtil.toUrl(control.model.icon1x1SourceUrl)
            platformId: control.model.platformId
            size: control._art
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.topMargin: AppStyle.spacingSm
            Layout.bottomMargin: AppStyle.spacingSm

            spacing: 0
            Text {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.verticalStretchFactor: 1
                text: control.model.displayName
                font.pixelSize: AppStyle.fontSizeMedium
                font.weight: Font.Medium
                font.family: AppStyle.fontFamily
                color: Theme.textPrimary
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
            }
            Text {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.verticalStretchFactor: 1
                text: PlatformModel.getPlatformDisplayName(control.model.platformId)
                font.pixelSize: AppStyle.fontSizeSmall
                font.family: AppStyle.fontFamily
                font.weight: Font.Light
                color: Theme.textMuted
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
            }
        }

        Item {
            Layout.preferredWidth: AppStyle.iconSizeSm
            Layout.preferredHeight: AppStyle.iconSizeSm
            Layout.alignment: Qt.AlignVCenter
            Layout.rightMargin: AppStyle.spacingSm
            Icon {
                anchors.centerIn: parent
                name: "favorite"
                filled: control.model.favorite
                size: AppStyle.iconSizeSm
                color: control.model.favorite ? Theme.favorite : Theme.textMuted
                visible: control.model.favorite || control.hovered
            }
            TapHandler {
                enabled: control.model.favorite || control.hovered
                onTapped: control.model.favorite = !control.model.favorite
            }
            HoverHandler {
                cursorShape: Qt.PointingHandCursor
            }
        }

        ColumnLayout {
            Layout.preferredWidth: control.activityColumnWidth
            Layout.fillWidth: false
            Layout.fillHeight: true
            Layout.topMargin: AppStyle.spacingSm
            Layout.bottomMargin: AppStyle.spacingSm

            spacing: 0
            Text {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.verticalStretchFactor: 1
                text: control.model.numSecondsPlayed > 0 ? FLUtil.formatPlayTime(control.model.numSecondsPlayed) : "Never played"
                font.pixelSize: AppStyle.fontSizeSmall
                font.weight: control.model.numSecondsPlayed > 0 ? Font.Normal : Font.Light
                font.family: AppStyle.fontFamily
                color: control.model.numSecondsPlayed > 0 ? Theme.textPrimary : Theme.textMuted
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
            }
            Text {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.verticalStretchFactor: 1
                visible: text !== ""
                text: FLUtil.formatLastPlayed(control.model.lastPlayedAt)
                font.pixelSize: AppStyle.fontSizeSmall
                font.family: AppStyle.fontFamily
                font.weight: Font.Light
                color: Theme.textMuted
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
            }
        }

        Item {
            Layout.preferredWidth: control.achievementsColumnWidth
            Layout.fillHeight: true

            Text {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                visible: !control._hasAchievements
                text: "No achievements"
                font.pixelSize: AppStyle.fontSizeSmall
                font.weight: Font.Light
                font.family: AppStyle.fontFamily
                color: Theme.textMuted
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
            }

            RowLayout {
                height: parent.height
                width: parent.width
                spacing: AppStyle.spacingSm
                visible: control._hasAchievements

                Icon {
                    name: "trophy"
                    filled: true
                    size: AppStyle.iconSizeMd
                    color: control._achievementsDone ? Theme.gold : Theme.textPrimary
                    Layout.alignment: Qt.AlignVCenter
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.topMargin: AppStyle.spacingMd
                    Layout.bottomMargin: AppStyle.spacingMd
                    spacing: AppStyle.spacingSm

                    Text {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.verticalStretchFactor: 1

                        verticalAlignment: Text.AlignBottom
                        text: control.model.achievementsEarned + "/" + control.model.achievementsTotal + (control.model.achievementSetCount > 1 ? " (+" + (control.model.achievementSetCount - 1) + ")" : "")
                        font.pixelSize: AppStyle.fontSizeSmall
                        font.family: AppStyle.fontFamily
                        color: Theme.textPrimary
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.verticalStretchFactor: 1

                        Rectangle {
                            anchors.top: parent.top
                            width: parent.width
                            height: Math.round(4 * AppStyle.scale)
                            radius: height / 2
                            color: Theme.backgroundInset
                            Rectangle {
                                width: parent.width * (control._hasAchievements ? control.model.achievementsEarned / control.model.achievementsTotal : 0)
                                height: parent.height
                                radius: height / 2
                                color: control._achievementsDone ? Theme.gold : Theme.accent
                            }
                        }
                    }

                }

            }
        }

        Item {
            Layout.preferredWidth: control.ratingColumnWidth
            Layout.fillHeight: true
            Layout.rightMargin: AppStyle.spacingSm

            FLStarRating {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                // starSize: AppStyle.iconSizeSm

                Component.onCompleted: {
                    value = control.model.rating
                }

                onValueChanged: {
                    control.model.rating = value
                }
            }
        }

        Item {
            Layout.preferredWidth: AppStyle.iconSizeLg
            Layout.preferredHeight: AppStyle.iconSizeLg
            Layout.alignment: Qt.AlignVCenter
            Icon {
                anchors.centerIn: parent
                name: "more-vertical"
                size: AppStyle.iconSizeMd
                color: Theme.textMuted
                visible: control.hovered
            }
            TapHandler {
                enabled: control.hovered
                onTapped: control.ContextMenu.menu.popup()
            }
            HoverHandler {
                cursorShape: Qt.PointingHandCursor
            }
        }
    }
}