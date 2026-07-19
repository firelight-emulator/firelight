import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

// Per-game "Change artwork" picker. Shows the stored candidates + current
// selection for a media type, lets the user search SteamGridDB for more, and
// import a local image. All actions go through GameArtService, which reprojects
// the entry's art and refreshes the library tile in place
Dialog {
    id: control

    property string contentHash: ""
    property string gameName: ""
    property int platformId: -1
    property int mediaType: 0 // C++ MediaType enum (Icon = 0)

    // The media types the picker can manage; `type` matches the C++ enum
    readonly property var mediaTypes: [
        {
            label: "Icon",
            type: 0
        },
        {
            label: "Box Art",
            type: 1
        },
        {
            label: "Logo",
            type: 3
        },
        {
            label: "Hero",
            type: 4
        }
    ]

    property var storedList: []
    property var onlineList: []
    // onlineList grouped by matched game, order preserved:
    // [{gameName, items:[...]}]. Each item keeps its flat `resultIndex`
    property var onlineGroups: control.groupResults(control.onlineList)

    // The term used for provider searches. Defaults to the game's name, but the
    // user can override it when the title doesn't turn up matches
    property string searchTerm: ""

    // Groups the flat result list by matched game name, preserving first-seen
    // (best-match) order so each game gets its own header in the picker
    function groupResults(list) {
        var groups = [];
        var indexByName = ({});
        for (var i = 0; i < list.length; i++) {
            var item = list[i];
            var name = (item.gameName && item.gameName.length > 0) ? item.gameName : "Other results";
            if (indexByName[name] === undefined) {
                indexByName[name] = groups.length;
                groups.push({
                    gameName: name,
                    items: []
                });
            }
            groups[indexByName[name]].items.push(item);
        }
        return groups;
    }

    function openFor(hash, name, platform) {
        control.contentHash = hash;
        control.gameName = name;
        control.platformId = platform;
        control.searchTerm = name;
        control.mediaType = 0;
        control.open();
    }

    function refresh() {
        if (!control.visible || control.contentHash === "") {
            return;
        }
        control.storedList = GameArtService.storedAssets(control.contentHash, control.mediaType);
        control.onlineList = GameArtService.searchResults(control.contentHash, control.mediaType);
    }

    function doSearch() {
        if (!GameArtService.providerConfigured) {
            return;
        }
        var term = control.searchTerm.trim() !== "" ? control.searchTerm : control.gameName;
        GameArtService.search(control.contentHash, term, control.platformId, control.mediaType);
    }

    // Load cached results when switching to a type; auto-search the first time
    function maybeAutoSearch() {
        if (GameArtService.providerConfigured && control.onlineList.length === 0) {
            control.doSearch();
        }
    }

    onMediaTypeChanged: {
        control.refresh();
        control.maybeAutoSearch();
    }

    onOpened: {
        control.refresh();
        control.maybeAutoSearch();
    }

    Connections {
        target: GameArtService
        function onSearchResultsReady(hash, type) {
            if (hash === control.contentHash && type === control.mediaType) {
                control.onlineList = GameArtService.searchResults(hash, type);
            }
        }
        function onAssetsChanged(hash, type) {
            if (hash === control.contentHash) {
                control.refresh();
            }
        }
    }

    modal: true
    parent: Overlay.overlay
    anchors.centerIn: parent
    padding: 0
    width: Math.min(parent ? parent.width - 80 : 760, 760)
    height: Math.min(parent ? parent.height - 80 : 640, 640)
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    background: Rectangle {
        color: ColorPalette.neutral900
        radius: AppStyle.radiusMd
        border.color: ColorPalette.neutral700
        border.width: 1
    }

    header: ColumnLayout {
        spacing: 2
        Text {
            Layout.fillWidth: true
            Layout.topMargin: 18
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            text: "Change artwork"
            color: Theme.textPrimary
            font.family: Constants.regularFontFamily
            font.pixelSize: AppStyle.fontSizeLarge
            elide: Text.ElideRight
        }
        Text {
            Layout.fillWidth: true
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            Layout.bottomMargin: 6
            text: control.gameName
            color: Theme.textMuted
            font.family: Constants.regularFontFamily
            font.pixelSize: AppStyle.fontSizeSmall
            elide: Text.ElideRight
        }
    }

    // Reused for both stored and online thumbnails. Self-contained so it can be
    // used as an inline component without reaching for outer ids
    component ArtThumb: Rectangle {
        id: thumb
        property string url: ""
        property bool isSelected: false
        signal picked

        // A remote URL passes through; a local file path (user import) becomes a
        // file:// URL so Image can load it
        function resolveSource(u) {
            if (!u) {
                return "";
            }
            if (u.indexOf("://") >= 0) {
                return u;
            }
            return "file:///" + u.replace(/\\/g, "/");
        }

        width: 104
        height: 104
        radius: AppStyle.radiusMd
        color: ColorPalette.neutral800
        border.color: isSelected ? ColorPalette.accentColor : (hover.hovered ? ColorPalette.neutral500 : ColorPalette.neutral700)
        border.width: isSelected ? 3 : 1

        Image {
            anchors.fill: parent
            anchors.margins: 6
            source: thumb.resolveSource(thumb.url)
            fillMode: Image.PreserveAspectFit
            sourceSize.width: 256
            sourceSize.height: 256
            smooth: true
            mipmap: true
            asynchronous: true
        }

        HoverHandler {
            id: hover
            cursorShape: Qt.PointingHandCursor
        }
        TapHandler {
            onTapped: thumb.picked()
        }
    }

    contentItem: ColumnLayout {
        spacing: AppStyle.spacingMd

        // Type selector
        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            Layout.topMargin: AppStyle.spacingSm
            spacing: AppStyle.spacingSm
            Repeater {
                model: control.mediaTypes
                delegate: Button {
                    required property var modelData
                    text: modelData.label
                    checkable: true
                    checked: control.mediaType === modelData.type
                    focusPolicy: Qt.NoFocus
                    onClicked: control.mediaType = modelData.type
                    contentItem: Text {
                        text: parent.text
                        color: parent.checked ? "white" : ColorPalette.neutral400
                        font.family: Constants.regularFontFamily
                        font.pixelSize: AppStyle.fontSizeSmall
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        radius: AppStyle.radiusMd
                        color: parent.checked ? ColorPalette.neutral700 : "transparent"
                        border.width: 1
                        border.color: ColorPalette.neutral700
                        implicitHeight: 32
                        implicitWidth: 84
                    }
                }
            }
            Item {
                Layout.fillWidth: true
            }
        }

        FLScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            contentWidth: availableWidth

            ColumnLayout {
                width: parent.width
                spacing: 10

                // ---- Your artwork -------------------------------------------
                Text {
                    text: "Your artwork"
                    color: Theme.textPrimary
                    font.family: Constants.regularFontFamily
                    font.pixelSize: AppStyle.fontSizeSmall
                }
                Text {
                    visible: control.storedList.length === 0
                    text: "No artwork yet for this game."
                    color: Theme.textMuted
                    font.family: Constants.regularFontFamily
                    font.pixelSize: AppStyle.fontSizeSmall
                }
                Flow {
                    Layout.fillWidth: true
                    spacing: 10
                    Repeater {
                        model: control.storedList
                        delegate: ArtThumb {
                            required property var modelData
                            url: modelData.url
                            isSelected: modelData.selected
                            onPicked: GameArtService.selectStored(control.contentHash, modelData.assetId)
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.topMargin: AppStyle.spacingXs
                    Layout.bottomMargin: AppStyle.spacingXs
                    implicitHeight: 1
                    color: ColorPalette.neutral800
                }

                // ---- SteamGridDB --------------------------------------------
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10
                    Text {
                        text: "SteamGridDB"
                        color: Theme.textPrimary
                        font.family: Constants.regularFontFamily
                        font.pixelSize: AppStyle.fontSizeSmall
                    }
                    BusyIndicator {
                        running: GameArtService.searching
                        visible: running
                        implicitWidth: 20
                        implicitHeight: 20
                    }
                    Item {
                        Layout.fillWidth: true
                    }
                }

                // Search term: pre-filled with the game's name, editable so the
                // user can try a different query when the title finds nothing
                RowLayout {
                    Layout.fillWidth: true
                    visible: GameArtService.providerConfigured
                    spacing: AppStyle.spacingSm
                    FLTextField {
                        id: searchField
                        Layout.fillWidth: true
                        text: control.searchTerm
                        placeholderText: "Search term"
                        onTextEdited: control.searchTerm = text
                        onAccepted: control.doSearch()
                    }
                    FirelightButton {
                        rounded: false
                        radius: AppStyle.radiusMd
                        label: "Search"
                        implicitWidth: 110
                        onClicked: control.doSearch()
                    }
                }

                // API-key entry when the provider isn't configured yet
                ColumnLayout {
                    Layout.fillWidth: true
                    visible: !GameArtService.providerConfigured
                    spacing: 6
                    Text {
                        Layout.fillWidth: true
                        text: "Add your SteamGridDB API key to search for artwork."
                        color: Theme.textMuted
                        font.family: Constants.regularFontFamily
                        font.pixelSize: AppStyle.fontSizeSmall
                        wrapMode: Text.WordWrap
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: AppStyle.spacingSm
                        FLTextField {
                            id: keyField
                            Layout.fillWidth: true
                            placeholderText: "SteamGridDB API key"
                            echoMode: TextInput.Password
                        }
                        FirelightButton {
                            rounded: false
                            radius: AppStyle.radiusMd
                            label: "Save key"
                            implicitWidth: 110
                            enabled: keyField.text.length > 0
                            onClicked: {
                                GameArtService.apiKey = keyField.text;
                                control.doSearch();
                            }
                        }
                    }
                    Text {
                        text: "Get a key at steamgriddb.com/profile/preferences/api"
                        color: ColorPalette.hyperlinkColor
                        font.family: Constants.regularFontFamily
                        font.pixelSize: AppStyle.fontSizeSmall
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: Qt.openUrlExternally("https://www.steamgriddb.com/profile/preferences/api")
                        }
                    }
                }

                Text {
                    visible: GameArtService.providerConfigured && control.onlineList.length === 0 && !GameArtService.searching
                    text: "No SteamGridDB results for this game and type."
                    color: ColorPalette.neutral500
                    font.family: Constants.regularFontFamily
                    font.pixelSize: AppStyle.fontSizeSmall
                }

                // Results grouped by matched game, each under its own header
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: AppStyle.spacingMd
                    Repeater {
                        model: control.onlineGroups
                        delegate: ColumnLayout {
                            id: groupDelegate
                            required property var modelData
                            Layout.fillWidth: true
                            spacing: 6
                            Text {
                                Layout.fillWidth: true
                                text: groupDelegate.modelData.gameName
                                color: ColorPalette.neutral400
                                font.family: Constants.regularFontFamily
                                font.pixelSize: AppStyle.fontSizeSmall
                                font.weight: Font.DemiBold
                                elide: Text.ElideRight
                            }
                            Flow {
                                Layout.fillWidth: true
                                spacing: 10
                                Repeater {
                                    model: groupDelegate.modelData.items
                                    delegate: ArtThumb {
                                        required property var modelData
                                        url: modelData.thumbUrl
                                        onPicked: GameArtService.applyOnline(control.contentHash, control.mediaType, modelData.resultIndex)
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    footer: RowLayout {
        spacing: AppStyle.spacingSm
        FirelightButton {
            Layout.leftMargin: 20
            Layout.bottomMargin: AppStyle.spacingLg
            rounded: false
            radius: AppStyle.radiusMd
            iconCode: ""
            label: "Import from file…"
            implicitWidth: 180
            onClicked: importDialog.open()
        }
        Item {
            Layout.fillWidth: true
        }
        FirelightButton {
            Layout.rightMargin: 20
            Layout.bottomMargin: AppStyle.spacingLg
            rounded: false
            radius: AppStyle.radiusMd
            label: "Close"
            implicitWidth: 100
            onClicked: control.close()
        }
    }

    FileDialog {
        id: importDialog
        title: "Choose an image"
        nameFilters: ["Images (*.png *.jpg *.jpeg *.webp *.bmp)"]
        onAccepted: GameArtService.importImage(control.contentHash, control.mediaType, selectedFile)
    }
}
