import QtQuick
import QtQml
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

// TODO
// The per-game detail screen (route /library/entries/:entryId). A hero (art +
// title + play/favorite) over a tab bar; each tab scopes its data to this game.
// Entry fields come from a single-row mirror of LibraryEntryModel so no bespoke
// C++ item is needed
Item {
    id: root

    required property var entryId

    readonly property int _eid: parseInt(root.entryId)

    // The one library row for this entry (every field, including joined play
    // stats + achievement counts), live and writable through the shared model
    readonly property var entry: entryView.entry

    LibraryEntryView {
        id: entryView
        entryId: root._eid
    }

    GameActivity {
        id: gameActivity
        contentHash: root.entry ? root.entry.contentHash : ""
    }

    FLGameEditDialog {
        id: editDialog
    }

    property string activeTab: "details"

    function fmtPlayTime(seconds) {
        if (!seconds || seconds <= 0) {
            return "Never played";
        }
        var h = Math.floor(seconds / 3600);
        var m = Math.round((seconds % 3600) / 60);
        if (h > 0) {
            return h + "h " + m + "m";
        }
        return m + "m";
    }
    function fmtDate(millis) {
        if (!millis || millis <= 0) {
            return "—";
        }
        return Qt.formatDateTime(new Date(millis), "MMM d, yyyy");
    }
    function fmtEpochSecs(secs) {
        if (!secs || secs <= 0) {
            return "—";
        }
        return Qt.formatDateTime(new Date(secs * 1000), "MMM d, yyyy h:mm ap");
    }

    Flickable {
        id: scroll
        anchors.fill: parent
        contentWidth: width
        contentHeight: contentColumn.implicitHeight
        boundsBehavior: Flickable.StopAtBounds
        clip: true

        ScrollBar.vertical: FLScrollBar {
            width: AppStyle.spacingSm
        }

        ColumnLayout {
            id: contentColumn
            width: scroll.width
            spacing: AppStyle.spacingLg

            //****************
            // hero
            //****************
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: heroRow.implicitHeight + AppStyle.spacingLg * 3

                // Soft accent wash behind the hero
                Rectangle {
                    anchors.fill: parent
                    gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop {
                            position: 0.0
                            color: Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.10)
                        }
                        GradientStop {
                            position: 1.0
                            color: "transparent"
                        }
                    }
                }

                RowLayout {
                    id: heroRow
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.leftMargin: AppStyle.spacingLg
                    anchors.rightMargin: AppStyle.spacingLg
                    spacing: AppStyle.spacingLg

                    // Square game art, platform-logo fallback
                    Rectangle {
                        Layout.alignment: Qt.AlignVCenter
                        Layout.preferredWidth: Math.round(180 * AppStyle.scale)
                        Layout.preferredHeight: Math.round(180 * AppStyle.scale)
                        radius: AppStyle.radiusMd
                        color: Theme.surfaceElevated
                        clip: true

                        FLPlatformIcon {
                            anchors.centerIn: parent
                            width: parent.width * 0.5
                            height: width
                            visible: heroArt.status !== Image.Ready
                            platformId: root.entry ? root.entry.platformId : -1
                        }
                        FLRoundedImage {
                            id: heroArt
                            anchors.fill: parent
                            visible: status === Image.Ready
                            radius: AppStyle.radiusMd
                            fillMode: Image.PreserveAspectCrop
                            background: "transparent"
                            source: root.entry ? FLUtil.toUrl(root.entry.icon1x1SourceUrl || root.entry.boxartFrontSourceUrl) : ""
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignVCenter
                        spacing: AppStyle.spacingSm

                        Text {
                            text: root.entry ? PlatformModel.getPlatformDisplayName(root.entry.platformId).toUpperCase() : ""
                            color: Theme.accent
                            font.family: AppStyle.fontFamily
                            font.pixelSize: AppStyle.fontSizeSmall
                            font.weight: Font.DemiBold
                            font.letterSpacing: 1
                        }
                        Text {
                            Layout.fillWidth: true
                            text: root.entry ? root.entry.displayName : ""
                            color: Theme.textPrimary
                            font.family: AppStyle.fontFamily
                            font.pixelSize: AppStyle.fontSizeXLarge
                            font.weight: Font.Bold
                            wrapMode: Text.WordWrap
                            maximumLineCount: 2
                            elide: Text.ElideRight
                        }
                        Text {
                            Layout.fillWidth: true
                            text: {
                                if (!root.entry) {
                                    return "";
                                }
                                var parts = [];
                                if (root.entry.releaseYear > 0) {
                                    parts.push(root.entry.releaseYear);
                                }
                                if (root.entry.developer && root.entry.developer !== "") {
                                    parts.push(root.entry.developer);
                                }
                                if (root.entry.genres && root.entry.genres !== "") {
                                    parts.push(root.entry.genres);
                                }
                                return parts.join("   ·   ");
                            }
                            color: Theme.textMuted
                            font.family: AppStyle.fontFamily
                            font.pixelSize: AppStyle.fontSizeMedium
                            elide: Text.ElideRight
                        }

                        FLStarRating {
                            value: root.entry ? root.entry.rating : 0

                            onValueChanged: {
                                if (root.entry) {
                                    root.entry.rating = value;
                                }
                            }
                        }

                        RowLayout {
                            Layout.topMargin: AppStyle.spacingSm
                            spacing: AppStyle.spacingSm

                            FLButton {
                                variant: "primary"
                                iconName: "play-circle"
                                text: (root.entry && root.entry.numSecondsPlayed > 0) ? ("Continue · " + root.fmtPlayTime(root.entry.numSecondsPlayed)) : "Play"
                                onClicked: EmulationService.loadEntry(root._eid)
                            }
                            FLIconButton {
                                iconName: "favorite"
                                tooltipText: (root.entry && root.entry.favorite) ? "Unfavorite" : "Favorite"
                                onClicked: {
                                    if (root.entry) {
                                        LibraryEntryModel.setEntryFavorite(root._eid, !root.entry.favorite);
                                    }
                                }
                            }
                            FLIconButton {
                                iconName: "edit"
                                tooltipText: "Edit game"
                                onClicked: if (root.entry) {
                                    editDialog.loadAndOpen(root._eid, root.entry.contentHash, root.entry.platformId);
                                }
                            }
                            FLIconButton {
                                iconName: "settings"
                                tooltipText: "Game settings"
                                onClicked: root.activeTab = "settings"
                            }
                        }
                    }
                }
            }

            //****************
            // tab bar
            //****************
            ColumnLayout {
                Layout.fillWidth: true
                Layout.leftMargin: AppStyle.spacingLg
                Layout.rightMargin: AppStyle.spacingLg
                spacing: 0

                RowLayout {
                    Layout.fillWidth: true
                    spacing: AppStyle.spacingLg

                    Repeater {
                        model: [
                            {
                                id: "details",
                                label: "Details"
                            },
                            {
                                id: "achievements",
                                label: "Achievements"
                            },
                            {
                                id: "activity",
                                label: "Activity"
                            },
                            {
                                id: "gallery",
                                label: "Gallery"
                            },
                            {
                                id: "settings",
                                label: "Settings"
                            }
                        ]
                        delegate: Item {
                            required property var modelData
                            readonly property bool current: root.activeTab === modelData.id
                            Layout.preferredHeight: Math.round(40 * AppStyle.scale)
                            Layout.preferredWidth: tabLabel.implicitWidth

                            Text {
                                id: tabLabel
                                anchors.centerIn: parent
                                text: parent.modelData.label
                                color: parent.current ? Theme.textPrimary : Theme.textMuted
                                font.family: AppStyle.fontFamily
                                font.pixelSize: AppStyle.fontSizeMedium
                                font.weight: parent.current ? Font.DemiBold : Font.Normal
                            }
                            Rectangle {
                                anchors.bottom: parent.bottom
                                anchors.left: parent.left
                                anchors.right: parent.right
                                height: 2
                                radius: 1
                                color: Theme.accent
                                visible: parent.current
                            }
                            TapHandler {
                                onTapped: root.activeTab = parent.modelData.id
                            }
                            HoverHandler {
                                cursorShape: Qt.PointingHandCursor
                            }
                        }
                    }
                    Item {
                        Layout.fillWidth: true
                    }
                }
                FLDivider {
                    Layout.fillWidth: true
                }
            }

            //****************
            // tab content
            //****************
            Loader {
                Layout.fillWidth: true
                Layout.leftMargin: AppStyle.spacingLg
                Layout.rightMargin: AppStyle.spacingLg
                Layout.bottomMargin: AppStyle.spacingLg
                sourceComponent: {
                    switch (root.activeTab) {
                    case "activity":
                        return activityTab;
                    case "achievements":
                        return achievementsTab;
                    case "gallery":
                        return galleryTab;
                    case "settings":
                        return settingsTab;
                    default:
                        return detailsTab;
                    }
                }
            }
        }
    }

    //****************
    // details (overview)
    //****************
    Component {
        id: detailsTab
        RowLayout {
            id: detailsRoot
            spacing: AppStyle.spacingLg

            // Per-slot save info; recomputed when the game or its active slot
            // changes (a fresh save while off this screen won't be reflected
            // until it reopens, which is fine)
            readonly property var saveFiles: (root.entry && root.entry.contentHash !== "") ? SaveManager.getSaveFiles(root.entry.contentHash) : []
            readonly property int activeSlot: root.entry ? root.entry.activeSaveSlot : 1

            // Left: save states + description
            ColumnLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignTop
                spacing: AppStyle.spacingMd

                DetailSectionLabel {
                    text: "SAVE DATA"
                }

                // Active-slot picker
                RowLayout {
                    Layout.fillWidth: true
                    spacing: AppStyle.spacingSm

                    Text {
                        text: "Active slot"
                        color: Theme.textMuted
                        font.family: AppStyle.fontFamily
                        font.pixelSize: AppStyle.fontSizeSmall
                    }
                    Repeater {
                        model: 8
                        delegate: Rectangle {
                            required property int index
                            readonly property int slotNum: index + 1
                            readonly property bool isActive: detailsRoot.activeSlot === slotNum
                            readonly property bool hasData: {
                                for (var i = 0; i < detailsRoot.saveFiles.length; i++) {
                                    if (detailsRoot.saveFiles[i].slotNumber === slotNum && detailsRoot.saveFiles[i].hasData) {
                                        return true;
                                    }
                                }
                                return false;
                            }
                            Layout.preferredWidth: Math.round(32 * AppStyle.scale)
                            Layout.preferredHeight: Math.round(32 * AppStyle.scale)
                            radius: AppStyle.radiusSm
                            color: isActive ? Theme.accent : Theme.backgroundInset
                            border.width: hasData && !isActive ? 1 : 0
                            border.color: Theme.accent
                            Text {
                                anchors.centerIn: parent
                                text: parent.slotNum
                                color: parent.isActive ? Theme.onAccent : (parent.hasData ? Theme.textPrimary : Theme.textMuted)
                                font.family: AppStyle.fontFamily
                                font.pixelSize: AppStyle.fontSizeSmall
                                font.weight: Font.DemiBold
                            }
                            TapHandler {
                                onTapped: if (root.entry) {
                                    root.entry.activeSaveSlot = parent.slotNum;
                                }
                            }
                            HoverHandler {
                                cursorShape: Qt.PointingHandCursor
                            }
                        }
                    }
                }

                // Per-slot save data
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: AppStyle.spacingXs

                    Repeater {
                        model: detailsRoot.saveFiles
                        delegate: Rectangle {
                            id: slotRow
                            required property var modelData
                            readonly property bool isActive: detailsRoot.activeSlot === slotRow.modelData.slotNumber
                            Layout.fillWidth: true
                            Layout.preferredHeight: Math.round(52 * AppStyle.scale)
                            radius: AppStyle.radiusMd
                            color: Theme.backgroundInset
                            border.width: slotRow.isActive ? 1 : 0
                            border.color: Theme.accent

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: AppStyle.spacingMd
                                anchors.rightMargin: AppStyle.spacingMd
                                spacing: AppStyle.spacingMd

                                Icon {
                                    name: "save"
                                    size: AppStyle.iconSizeMd
                                    color: slotRow.isActive ? Theme.accent : Theme.textMuted
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 0
                                    Text {
                                        text: "Slot " + slotRow.modelData.slotNumber
                                        color: Theme.textPrimary
                                        font.family: AppStyle.fontFamily
                                        font.pixelSize: AppStyle.fontSizeSmall
                                        font.weight: Font.DemiBold
                                    }
                                    Text {
                                        text: "Saved " + root.fmtEpochSecs(slotRow.modelData.lastModified)
                                        color: Theme.textMuted
                                        font.family: AppStyle.fontFamily
                                        font.pixelSize: AppStyle.fontSizeXSmall
                                    }
                                }
                                Text {
                                    visible: slotRow.isActive
                                    text: "Active"
                                    color: Theme.accent
                                    font.family: AppStyle.fontFamily
                                    font.pixelSize: AppStyle.fontSizeXSmall
                                    font.weight: Font.DemiBold
                                }
                            }
                            TapHandler {
                                onTapped: if (root.entry) {
                                    root.entry.activeSaveSlot = slotRow.modelData.slotNumber;
                                }
                            }
                        }
                    }
                    Text {
                        visible: detailsRoot.saveFiles.length === 0
                        text: "No save data yet — it appears here once you play and the game saves."
                        color: Theme.textMuted
                        font.family: AppStyle.fontFamily
                        font.pixelSize: AppStyle.fontSizeSmall
                    }
                }

                DetailSectionLabel {
                    text: "ABOUT"
                    Layout.topMargin: AppStyle.spacingMd
                }
                Text {
                    Layout.fillWidth: true
                    text: (root.entry && root.entry.description && root.entry.description !== "") ? root.entry.description : "No description available for this game."
                    color: Theme.textPrimary
                    font.family: AppStyle.fontFamily
                    font.pixelSize: AppStyle.fontSizeMedium
                    wrapMode: Text.WordWrap
                    lineHeight: 1.35
                }
            }

            // Right rail: achievements mini + info
            ColumnLayout {
                Layout.preferredWidth: Math.round(300 * AppStyle.scale)
                Layout.alignment: Qt.AlignTop
                spacing: AppStyle.spacingMd

                // Achievements summary
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: achColumn.implicitHeight + AppStyle.spacingLg * 2
                    radius: AppStyle.radiusMd
                    color: Theme.backgroundInset
                    visible: root.entry && root.entry.achievementsTotal > 0

                    ColumnLayout {
                        id: achColumn
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.leftMargin: AppStyle.spacingLg
                        anchors.rightMargin: AppStyle.spacingLg
                        spacing: AppStyle.spacingSm

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: AppStyle.spacingSm
                            Icon {
                                name: "trophy"
                                filled: true
                                size: AppStyle.iconSizeSm
                                color: Theme.gold
                            }
                            Text {
                                Layout.fillWidth: true
                                text: "ACHIEVEMENTS"
                                color: Theme.textMuted
                                font.family: AppStyle.fontFamily
                                font.pixelSize: AppStyle.fontSizeXSmall
                                font.weight: Font.DemiBold
                                font.letterSpacing: 1
                            }
                            Text {
                                text: root.entry ? (root.entry.achievementsEarned + "/" + root.entry.achievementsTotal) : ""
                                color: Theme.accent
                                font.family: AppStyle.fontFamily
                                font.pixelSize: AppStyle.fontSizeMedium
                                font.weight: Font.DemiBold
                            }
                        }
                        Rectangle {
                            Layout.fillWidth: true
                            height: Math.round(6 * AppStyle.scale)
                            radius: height / 2
                            color: Theme.surface
                            Rectangle {
                                width: parent.width * (root.entry && root.entry.achievementsTotal > 0 ? root.entry.achievementsEarned / root.entry.achievementsTotal : 0)
                                height: parent.height
                                radius: height / 2
                                color: Theme.gold
                            }
                        }
                    }
                }

                // Info
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: infoColumn.implicitHeight + AppStyle.spacingLg * 2
                    radius: AppStyle.radiusMd
                    color: Theme.backgroundInset

                    ColumnLayout {
                        id: infoColumn
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.leftMargin: AppStyle.spacingLg
                        anchors.rightMargin: AppStyle.spacingLg
                        anchors.topMargin: AppStyle.spacingLg
                        spacing: AppStyle.spacingSm

                        InfoRow {
                            label: "Play time"
                            value: root.fmtPlayTime(root.entry ? root.entry.numSecondsPlayed : 0)
                        }
                        InfoRow {
                            label: "Last played"
                            value: root.entry ? root.fmtDate(root.entry.lastPlayedAt) : "—"
                        }
                        InfoRow {
                            label: "Times played"
                            value: gameActivity.timesPlayed > 0 ? ("" + gameActivity.timesPlayed) : "—"
                        }
                        InfoRow {
                            label: "Added"
                            value: root.entry ? root.fmtDate(root.entry.createdAt) : "—"
                        }
                    }
                }
            }
        }
    }

    //****************
    // activity
    //****************
    Component {
        id: activityTab
        FLGameActivityPage {
            implicitHeight: Math.round(420 * AppStyle.scale)
            activity: gameActivity
        }
    }

    //****************
    // placeholder tabs (wired in a follow-up)
    //****************
    Component {
        id: achievementsTab
        ColumnLayout {
            spacing: AppStyle.spacingMd

            RetroAchievementsGame {
                id: raGame
                contentHash: root.entry ? root.entry.contentHash : ""
            }

            Text {
                visible: raGame.achievementSets.length === 0
                Layout.topMargin: AppStyle.spacingLg
                Layout.alignment: Qt.AlignHCenter
                text: "This game has no achievements."
                color: Theme.textMuted
                font.family: AppStyle.fontFamily
                font.pixelSize: AppStyle.fontSizeMedium
            }

            Repeater {
                model: raGame.achievementSets
                delegate: ColumnLayout {
                    id: setBlock
                    required property var modelData
                    Layout.fillWidth: true
                    spacing: AppStyle.spacingSm

                    // Set header — shown when a game has more than one set
                    RowLayout {
                        Layout.fillWidth: true
                        Layout.topMargin: AppStyle.spacingSm
                        visible: raGame.achievementSets.length > 1
                        spacing: AppStyle.spacingSm
                        Text {
                            Layout.fillWidth: true
                            text: setBlock.modelData.name
                            color: Theme.textPrimary
                            font.family: AppStyle.fontFamily
                            font.pixelSize: AppStyle.fontSizeMedium
                            font.weight: Font.DemiBold
                        }
                        Text {
                            text: setBlock.modelData.numEarned + "/" + setBlock.modelData.numAchievements
                            color: Theme.textMuted
                            font.family: AppStyle.fontFamily
                            font.pixelSize: AppStyle.fontSizeSmall
                            font.weight: Font.DemiBold
                        }
                    }

                    Repeater {
                        model: setBlock.modelData.achievements
                        delegate: Rectangle {
                            id: achRow
                            required property var model
                            Layout.fillWidth: true
                            Layout.preferredHeight: Math.round(60 * AppStyle.scale)
                            radius: AppStyle.radiusMd
                            color: Theme.backgroundInset

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: AppStyle.spacingMd
                                anchors.rightMargin: AppStyle.spacingMd
                                spacing: AppStyle.spacingMd

                                Image {
                                    Layout.preferredWidth: Math.round(40 * AppStyle.scale)
                                    Layout.preferredHeight: Math.round(40 * AppStyle.scale)
                                    source: achRow.model.iconUrl
                                    fillMode: Image.PreserveAspectFit
                                    sourceSize: Qt.size(80, 80)
                                    opacity: achRow.model.earned ? 1 : 0.4
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 0
                                    Text {
                                        Layout.fillWidth: true
                                        text: achRow.model.name
                                        color: Theme.textPrimary
                                        font.family: AppStyle.fontFamily
                                        font.pixelSize: AppStyle.fontSizeSmall
                                        font.weight: Font.DemiBold
                                        elide: Text.ElideRight
                                    }
                                    Text {
                                        Layout.fillWidth: true
                                        text: achRow.model.description
                                        color: Theme.textMuted
                                        font.family: AppStyle.fontFamily
                                        font.pixelSize: AppStyle.fontSizeXSmall
                                        elide: Text.ElideRight
                                    }
                                }
                                Text {
                                    text: achRow.model.points + " pts"
                                    color: Theme.textMuted
                                    font.family: AppStyle.fontFamily
                                    font.pixelSize: AppStyle.fontSizeXSmall
                                    font.weight: Font.DemiBold
                                }
                                Icon {
                                    name: "trophy"
                                    filled: true
                                    size: AppStyle.iconSizeSm
                                    visible: achRow.model.earned
                                    color: Theme.gold
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    Component {
        id: galleryTab
        GalleryPage {
            implicitHeight: Math.round(520 * AppStyle.scale)
            gameContentHash: root.entry ? root.entry.contentHash : ""
        }
    }
    Component {
        id: settingsTab
        SettingsPage {
            page: "emulation"
            level: SettingsLevel.Game
            platformId: root.entry ? root.entry.platformId : -1
            contentHash: root.entry ? root.entry.contentHash : ""
        }
    }

    //****************
    // small building blocks
    //****************
    component DetailSectionLabel: Text {
        color: Theme.textMuted
        font.family: AppStyle.fontFamily
        font.pixelSize: AppStyle.fontSizeXSmall
        font.weight: Font.DemiBold
        font.letterSpacing: 1
    }

    component InfoRow: RowLayout {
        property string label: ""
        property string value: ""
        Layout.fillWidth: true
        spacing: AppStyle.spacingSm
        Text {
            text: parent.label
            color: Theme.textMuted
            font.family: AppStyle.fontFamily
            font.pixelSize: AppStyle.fontSizeSmall
        }
        Item {
            Layout.fillWidth: true
        }
        Text {
            text: parent.value
            color: Theme.textPrimary
            font.family: AppStyle.fontFamily
            font.pixelSize: AppStyle.fontSizeSmall
            font.weight: Font.DemiBold
        }
    }

    component TabPlaceholder: ColumnLayout {
        property string icon: "info"
        property string text: ""
        Layout.fillWidth: true
        spacing: AppStyle.spacingMd
        Item {
            Layout.preferredHeight: AppStyle.spacingLg * 2
        }
        Icon {
            Layout.alignment: Qt.AlignHCenter
            name: parent.icon
            size: AppStyle.iconSizeLg
            color: Theme.textMuted
        }
        Text {
            Layout.alignment: Qt.AlignHCenter
            Layout.maximumWidth: Math.round(420 * AppStyle.scale)
            text: parent.text
            color: Theme.textMuted
            font.family: AppStyle.fontFamily
            font.pixelSize: AppStyle.fontSizeMedium
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }
    }
}
