import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Right-dock details for the last-clicked game. `data` is a plain snapshot
// object (see GameView.focusSnapshot); null shows an empty state
Rectangle {
    id: panel

    property var gameData: null

    signal requestPlay(int entryId)
    signal requestFavorite(int entryId, bool favorite)
    signal requestAddToFolder(int entryId)

    color: Theme.surface

    function iconSource(u) {
        if (!u) {
            return "";
        }
        if (u.indexOf("://") >= 0) {
            return u;
        }
        return "file:///" + u.replace(/\\/g, "/");
    }
    function fmtTime(seconds) {
        if (!seconds || seconds <= 0) {
            return "—";
        }
        var h = Math.floor(seconds / 3600);
        var m = Math.round((seconds % 3600) / 60);
        return h > 0 ? (h + "h " + m + "m") : (m > 0 ? (m + "m") : "<1m");
    }
    function fmtDate(ms) {
        return (!ms || ms <= 0) ? "Never" : Qt.formatDateTime(new Date(ms), "MMM d, yyyy");
    }

    // Left hairline separating the dock from the grid
    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 1
        color: Theme.textPrimary
        opacity: 0.12
    }

    Text {
        anchors.centerIn: parent
        visible: !panel.gameData
        width: parent.width - AppStyle.spacingLg * 2
        text: "Select a game to see its details"
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
        color: Theme.textMuted
        font.family: Constants.regularFontFamily
        font.pixelSize: AppStyle.fontSizeMedium
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: AppStyle.spacingLg
        anchors.leftMargin: AppStyle.spacingLg + 1
        spacing: AppStyle.spacingSm
        visible: !!panel.gameData

        Image {
            Layout.fillWidth: true
            Layout.preferredHeight: Math.round(220 * AppStyle.scale)
            fillMode: Image.PreserveAspectFit
            horizontalAlignment: Image.AlignHCenter
            source: panel.gameData ? panel.iconSource(panel.gameData.boxartFrontSourceUrl !== "" ? panel.gameData.boxartFrontSourceUrl : panel.gameData.icon1x1SourceUrl) : ""
            sourceSize.width: 512
            smooth: true
        }

        Text {
            Layout.fillWidth: true
            text: panel.gameData ? panel.gameData.displayName : ""
            color: Theme.textPrimary
            font.family: Constants.regularFontFamily
            font.pixelSize: AppStyle.fontSizeLarge
            font.weight: Font.Bold
            wrapMode: Text.WordWrap
            maximumLineCount: 3
            elide: Text.ElideRight
        }
        Text {
            Layout.fillWidth: true
            text: panel.gameData ? (panel.gameData.platformId + (panel.gameData.releaseYear > 0 ? "  ·  " + panel.gameData.releaseYear : "") + (panel.gameData.developer !== "" ? "  ·  " + panel.gameData.developer : "")) : ""
            color: Theme.textMuted
            font.family: Constants.regularFontFamily
            font.pixelSize: AppStyle.fontSizeSmall
            elide: Text.ElideRight
        }

        FLDivider {}

        // Stat rows
        GridLayout {
            Layout.fillWidth: true
            columns: 2
            columnSpacing: AppStyle.spacingSm
            rowSpacing: AppStyle.spacingXs

            Text {
                text: "Last played"
                color: Theme.textMuted
                font.family: Constants.regularFontFamily
                font.pixelSize: AppStyle.fontSizeSmall
            }
            Text {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignRight
                text: panel.gameData ? panel.fmtDate(panel.gameData.lastPlayedAt) : ""
                color: Theme.textPrimary
                font.family: Constants.regularFontFamily
                font.pixelSize: AppStyle.fontSizeSmall
                font.weight: Font.DemiBold
            }
            Text {
                text: "Play time"
                color: Theme.textMuted
                font.family: Constants.regularFontFamily
                font.pixelSize: AppStyle.fontSizeSmall
            }
            Text {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignRight
                text: panel.gameData ? panel.fmtTime(panel.gameData.numSecondsPlayed) : ""
                color: Theme.textPrimary
                font.family: Constants.regularFontFamily
                font.pixelSize: AppStyle.fontSizeSmall
                font.weight: Font.DemiBold
            }
        }

        // Achievement progress
        ColumnLayout {
            Layout.fillWidth: true
            Layout.topMargin: AppStyle.spacingXs
            spacing: AppStyle.spacingXs
            visible: panel.gameData && panel.gameData.achievementsTotal > 0

            RowLayout {
                Layout.fillWidth: true
                Text {
                    text: "Achievements"
                    color: Theme.textMuted
                    font.family: Constants.regularFontFamily
                    font.pixelSize: AppStyle.fontSizeSmall
                }
                Item {
                    Layout.fillWidth: true
                }
                Text {
                    text: panel.gameData ? (panel.gameData.achievementsEarned + " / " + panel.gameData.achievementsTotal) : ""
                    color: Theme.textPrimary
                    font.family: Constants.regularFontFamily
                    font.pixelSize: AppStyle.fontSizeSmall
                    font.weight: Font.DemiBold
                }
            }
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: Math.round(6 * AppStyle.scale)
                radius: implicitHeight / 2
                color: Theme.surfaceElevated
                Rectangle {
                    height: parent.height
                    radius: parent.radius
                    width: parent.width * (panel.gameData && panel.gameData.achievementsTotal > 0 ? Math.min(1, panel.gameData.achievementsEarned / panel.gameData.achievementsTotal) : 0)
                    color: Theme.accent
                    Behavior on width {
                        NumberAnimation {
                            duration: 160
                        }
                    }
                }
            }
        }

        // Description (the scrollable bulk)
        FLScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.topMargin: AppStyle.spacingXs

            Text {
                width: panel.width - AppStyle.spacingLg * 2 - 1
                text: panel.gameData && panel.gameData.description !== "" ? panel.gameData.description : "No description."
                color: Theme.textMuted
                font.family: Constants.regularFontFamily
                font.pixelSize: AppStyle.fontSizeSmall
                wrapMode: Text.WordWrap
                lineHeight: 1.25
            }
        }

        // Actions (pinned)
        RowLayout {
            Layout.fillWidth: true
            spacing: AppStyle.spacingSm

            FLButton {
                Layout.fillWidth: true
                compact: true
                variant: "primary"
                text: "Play"
                onClicked: if (panel.gameData)
                    panel.requestPlay(panel.gameData.entryId)
            }
            FLIconButton {
                compact: true
                iconName: "favorite"
                checked: panel.gameData ? panel.gameData.favorite : false
                tooltipText: (panel.gameData && panel.gameData.favorite) ? "Remove from favorites" : "Add to favorites"
                onClicked: if (panel.gameData)
                    panel.requestFavorite(panel.gameData.entryId, !panel.gameData.favorite)
            }
            FLIconButton {
                compact: true
                iconName: "folder"
                tooltipText: "Add to folder"
                onClicked: if (panel.gameData)
                    panel.requestAddToFolder(panel.gameData.entryId)
            }
        }
    }
}
