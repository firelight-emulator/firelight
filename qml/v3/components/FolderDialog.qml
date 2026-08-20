import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

// TODO
// Unified create/edit dialog for library folders. Manual folders edit
// name/icon/color/description; smart folders add source + filter criteria.
// Persists field writes through the model's setData (model.<role> = value),
// not per-field invokables. Create: leave editFolderId at -1, set `smart` +
// `targetParentId`, open(). Edit: call loadForEdit(...) then open()
FirelightDialog {
    id: control

    // -1 = creating; otherwise the folder id being edited
    property int editFolderId: -1
    // true = smart folder (shows the criteria sections)
    property bool smart: false
    // Parent scope for a newly created folder (-1 = root)
    property int targetParentId: -1

    // The row currently written through setData (the edit target, or the newly
    // created folder once its id is known)
    property int activeFolderId: editFolderId

    property alias folderName: nameInput.text

    headerText: (editFolderId === -1 ? "New " : "Edit ") + (smart ? "smart folder" : "folder")
    acceptText: "Save"

    // Appearance
    property string selectedColor: ""
    property string selectedIcon: ""
    property string selectedSortRole: ""
    property bool selectedSortAscending: true

    // Smart-criteria selection state (arrays of ids)
    property var selectedDirIds: []
    property var selectedPlatformIds: []

    readonly property var sortOptions: [
        {
            label: "Default (name)",
            role: ""
        },
        {
            label: "Name",
            role: "displayName"
        },
        {
            label: "Recently played",
            role: "lastPlayedAt"
        },
        {
            label: "Most played",
            role: "numSecondsPlayed"
        },
        {
            label: "Release year",
            role: "releaseYear"
        }
    ]

    // Preset folder glyphs (stored as qrc:/icons/<name>, rendered by the row)
    readonly property var iconChoices: ["folder", "star", "bookmark-star", "favorite", "controller", "trophy", "book", "history", "kid-star", "play-circle", "home", "photo-library"]

    signal saved

    function toggleId(arr, id) {
        var copy = arr.slice();
        var idx = copy.indexOf(id);
        if (idx === -1) {
            copy.push(id);
        } else {
            copy.splice(idx, 1);
        }
        return copy;
    }

    function resetFields() {
        nameInput.text = "";
        descriptionInput.text = "";
        selectedColor = "";
        selectedIcon = "";
        selectedSortRole = "";
        selectedSortAscending = true;
        selectedDirIds = [];
        selectedPlatformIds = [];
        pathContainsInput.text = "";
        developerInput.text = "";
        publisherInput.text = "";
        genreInput.text = "";
        yearMinInput.text = "";
        yearMaxInput.text = "";
        minMinutesInput.text = "";
        lastDaysInput.text = "";
        favoriteOnly.checked = false;
        unplayedOnly.checked = false;
    }

    // Opens the dialog to edit an existing folder
    function loadForEdit(folderId, isSmart, name, color, icon, description, filterJson, sortRole, sortAscending) {
        resetFields();
        control.editFolderId = folderId;
        control.activeFolderId = folderId;
        control.smart = isSmart;
        nameInput.text = name || "";
        descriptionInput.text = description || "";
        control.selectedColor = color || "";
        control.selectedIcon = icon || "";
        control.selectedSortRole = sortRole || "";
        control.selectedSortAscending = sortAscending !== false;
        if (isSmart) {
            loadCriteria(filterJson);
        }
        control.open();
    }

    // Opens the dialog to create a new folder under `parentId`
    function openForCreate(isSmart, parentId) {
        resetFields();
        control.editFolderId = -1;
        control.activeFolderId = -1;
        control.smart = isSmart;
        control.targetParentId = parentId;
        control.open();
    }

    function loadCriteria(jsonStr) {
        if (!jsonStr || jsonStr.length === 0) {
            return;
        }
        var c = {};
        try {
            c = JSON.parse(jsonStr);
        } catch (e) {
            return;
        }
        pathContainsInput.text = c.pathContains || "";
        developerInput.text = c.developer || "";
        publisherInput.text = c.publisher || "";
        genreInput.text = (c.genres && c.genres.length > 0) ? c.genres[0] : "";
        yearMinInput.text = c.yearMin !== undefined ? String(c.yearMin) : "";
        yearMaxInput.text = c.yearMax !== undefined ? String(c.yearMax) : "";
        minMinutesInput.text = c.minSecondsPlayed !== undefined ? String(Math.round(c.minSecondsPlayed / 60)) : "";
        lastDaysInput.text = c.playedWithinDays !== undefined ? String(c.playedWithinDays) : "";
        favoriteOnly.checked = c.favorite === true;
        unplayedOnly.checked = c.unplayed === true;
        selectedDirIds = c.contentDirectoryIds ? c.contentDirectoryIds.slice() : [];
        selectedPlatformIds = c.platformIds ? c.platformIds.slice() : [];
    }

    function buildFilterJson() {
        var c = {};
        if (selectedDirIds.length > 0) {
            c.contentDirectoryIds = selectedDirIds;
        }
        if (pathContainsInput.text.trim().length > 0) {
            c.pathContains = pathContainsInput.text.trim();
        }
        if (selectedPlatformIds.length > 0) {
            c.platformIds = selectedPlatformIds;
        }
        if (favoriteOnly.checked) {
            c.favorite = true;
        }
        if (genreInput.text.trim().length > 0) {
            c.genres = [genreInput.text.trim()];
        }
        if (developerInput.text.trim().length > 0) {
            c.developer = developerInput.text.trim();
        }
        if (publisherInput.text.trim().length > 0) {
            c.publisher = publisherInput.text.trim();
        }
        if (yearMinInput.text.trim().length > 0) {
            c.yearMin = parseInt(yearMinInput.text);
        }
        if (yearMaxInput.text.trim().length > 0) {
            c.yearMax = parseInt(yearMaxInput.text);
        }
        if (minMinutesInput.text.trim().length > 0) {
            c.minSecondsPlayed = parseInt(minMinutesInput.text) * 60;
        }
        if (lastDaysInput.text.trim().length > 0) {
            c.playedWithinDays = parseInt(lastDaysInput.text);
        }
        if (unplayedOnly.checked) {
            c.unplayed = true;
        }
        return JSON.stringify(c);
    }

    // Writes the appearance fields to the active folder row through setData
    function persistAppearance() {
        if (rowMirror.count > 0) {
            rowMirror.itemAt(0).apply(nameInput.text.trim(), control.selectedColor, control.selectedIcon, descriptionInput.text, control.selectedSortRole, control.selectedSortAscending);
        }
    }

    onAccepted: {
        var name = nameInput.text.trim();
        var json = control.smart ? buildFilterJson() : "";
        if (control.editFolderId === -1) {
            var id = control.smart ? LibraryFolderModel.addSmartFolder(name, json) : LibraryFolderModel.createFolder(name, control.targetParentId);
            if (id !== -1) {
                // The new row exists now; point the mirror at it and write its
                // appearance once the proxy has picked it up
                control.activeFolderId = id;
                Qt.callLater(control.persistAppearance);
            }
        } else {
            if (control.smart) {
                LibraryFolderModel.updateSmartFolder(control.editFolderId, json);
            }
            control.persistAppearance();
        }
        LibraryEntryModel.invalidateSmartFolderCache();
        control.saved();
    }

    // Hidden mirror of the active folder row, so appearance edits go through the
    // model's setData rather than bespoke setter invokables
    SortFilterProxyModel {
        id: rowProxy
        model: LibraryFolderModel
        filters: ValueFilter {
            roleName: "folderId"
            value: control.activeFolderId
            enabled: control.activeFolderId !== -1
        }
    }
    Item {
        visible: false
        Repeater {
            id: rowMirror
            model: rowProxy
            delegate: Item {
                required property var model
                function apply(name, color, icon, description, sortRole, sortAscending) {
                    model.displayName = name;
                    model.color = color;
                    model.icon1x1SourceUrl = icon;
                    model.description = description;
                    model.sortRole = sortRole;
                    model.sortAscending = sortAscending;
                }
            }
        }
    }

    FileDialog {
        id: iconFileDialog
        title: "Choose folder icon"
        nameFilters: ["Images (*.png *.jpg *.jpeg *.svg *.webp)"]
        onAccepted: control.selectedIcon = selectedFile
    }

    component FieldLabel: Text {
        color: Theme.textMuted
        font.family: AppStyle.fontFamily
        font.pixelSize: AppStyle.fontSizeSmall
    }

    component SectionLabel: Text {
        Layout.fillWidth: true
        Layout.topMargin: AppStyle.spacingSm
        color: Theme.textPrimary
        font.family: AppStyle.fontFamily
        font.pixelSize: AppStyle.fontSizeSmall
        font.weight: Font.DemiBold
    }

    contentItem: ScrollView {
        implicitWidth: Math.round(460 * AppStyle.scale)
        implicitHeight: Math.round(440 * AppStyle.scale)
        clip: true
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        ColumnLayout {
            width: control.availableWidth - AppStyle.spacingLg
            spacing: AppStyle.spacingSm

            // TODO
            // Folder kind — choosable only when creating; the rest of the form
            // shows/hides the smart-criteria sections off `control.smart`
            FLSegmentedControl {
                Layout.fillWidth: true
                visible: control.editFolderId === -1
                compact: false
                segments: [
                    {
                        label: "Manual",
                        value: "manual"
                    },
                    {
                        label: "Smart",
                        value: "smart"
                    }
                ]
                currentValue: control.smart ? "smart" : "manual"
                onActivated: value => control.smart = (value === "smart")
            }
            FieldLabel {
                Layout.fillWidth: true
                visible: control.editFolderId === -1
                wrapMode: Text.WordWrap
                text: control.smart ? "Fills itself automatically from the rules below." : "Holds the games you add to it."
            }

            FieldLabel {
                text: "Name"
            }
            FLTextField {
                id: nameInput
                Layout.fillWidth: true
                placeholderText: control.smart ? "Smart folder name" : "Folder name"
            }

            FieldLabel {
                text: "Description"
            }
            FLTextField {
                id: descriptionInput
                Layout.fillWidth: true
                placeholderText: "Optional"
            }

            // --- Icon ---
            SectionLabel {
                text: "Icon"
            }
            Flow {
                Layout.fillWidth: true
                spacing: AppStyle.spacingSm

                // "None" — clears the icon
                Rectangle {
                    width: AppStyle.iconChipSize
                    height: AppStyle.iconChipSize
                    radius: AppStyle.radiusSm
                    color: "transparent"
                    border.width: control.selectedIcon === "" ? 2 : 1
                    border.color: control.selectedIcon === "" ? Theme.accent : Theme.border
                    Icon {
                        anchors.centerIn: parent
                        name: "close"
                        size: AppStyle.iconSizeSm
                        color: Theme.textMuted
                    }
                    TapHandler {
                        onTapped: control.selectedIcon = ""
                    }
                    HoverHandler {
                        cursorShape: Qt.PointingHandCursor
                    }
                }

                Repeater {
                    model: control.iconChoices
                    delegate: Rectangle {
                        required property string modelData
                        readonly property string iconUrl: "qrc:/icons/" + modelData
                        width: AppStyle.iconChipSize
                        height: AppStyle.iconChipSize
                        radius: AppStyle.radiusSm
                        color: control.selectedIcon === iconUrl ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.18) : "transparent"
                        border.width: control.selectedIcon === iconUrl ? 2 : 1
                        border.color: control.selectedIcon === iconUrl ? Theme.accent : Theme.border
                        Icon {
                            anchors.centerIn: parent
                            name: parent.modelData
                            size: AppStyle.iconSizeSm
                            color: control.selectedColor !== "" ? control.selectedColor : Theme.textPrimary
                        }
                        TapHandler {
                            onTapped: control.selectedIcon = parent.iconUrl
                        }
                        HoverHandler {
                            cursorShape: Qt.PointingHandCursor
                        }
                    }
                }
            }
            RowLayout {
                Layout.fillWidth: true
                spacing: AppStyle.spacingSm

                // TODO
                // Preview of the current icon (preset or a chosen file), so a
                // file upload gives immediate feedback
                Rectangle {
                    Layout.preferredWidth: AppStyle.iconChipSize
                    Layout.preferredHeight: AppStyle.iconChipSize
                    radius: AppStyle.radiusSm
                    color: Theme.surfaceElevated
                    visible: control.selectedIcon !== ""
                    Image {
                        anchors.centerIn: parent
                        width: AppStyle.iconSizeMd
                        height: AppStyle.iconSizeMd
                        fillMode: Image.PreserveAspectFit
                        sourceSize: Qt.size(width * 2, height * 2)
                        source: control.selectedIcon
                    }
                }
                FLButton {
                    compact: true
                    variant: "default"
                    text: "Choose file…"
                    onClicked: iconFileDialog.open()
                }
                Item {
                    Layout.fillWidth: true
                }
            }

            // --- Color ---
            SectionLabel {
                text: "Accent color"
            }
            FLColorPicker {
                Layout.fillWidth: true
                selectedColor: control.selectedColor
                onPicked: color => control.selectedColor = color
            }

            // --- Default sort ---
            SectionLabel {
                text: "Default game sort"
            }
            RowLayout {
                Layout.fillWidth: true
                spacing: AppStyle.spacingSm
                FLComboBox {
                    Layout.fillWidth: true
                    model: control.sortOptions
                    textRole: "label"
                    currentIndex: {
                        for (var i = 0; i < control.sortOptions.length; i++) {
                            if (control.sortOptions[i].role === control.selectedSortRole) {
                                return i;
                            }
                        }
                        return 0;
                    }
                    onActivated: control.selectedSortRole = control.sortOptions[currentIndex].role
                }
                FLButton {
                    variant: "default"
                    enabled: control.selectedSortRole !== ""
                    text: control.selectedSortAscending ? "Ascending" : "Descending"
                    onClicked: control.selectedSortAscending = !control.selectedSortAscending
                }
            }

            // --- Smart criteria (smart folders only) ---
            SectionLabel {
                text: "Source"
                visible: control.smart
            }
            FieldLabel {
                text: "Leave everything below empty to draw from the whole library."
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                visible: control.smart
            }
            FieldLabel {
                text: "Path contains"
                visible: control.smart
            }
            FLTextField {
                id: pathContainsInput
                Layout.fillWidth: true
                visible: control.smart
                placeholderText: "e.g. snes"
            }
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2
                visible: control.smart
                Repeater {
                    model: ContentDirectoryModel
                    delegate: CheckDelegate {
                        required property var model
                        Layout.fillWidth: true
                        padding: AppStyle.spacingXs
                        checked: control.selectedDirIds.indexOf(model.directory_id) !== -1
                        contentItem: Text {
                            leftPadding: Math.round(28 * AppStyle.scale)
                            text: model.path
                            color: Theme.textPrimary
                            elide: Text.ElideMiddle
                            font.family: AppStyle.fontFamily
                            font.pixelSize: AppStyle.fontSizeSmall
                            verticalAlignment: Text.AlignVCenter
                        }
                        onToggled: control.selectedDirIds = control.toggleId(control.selectedDirIds, model.directory_id)
                    }
                }
            }

            SectionLabel {
                text: "Filters"
                visible: control.smart
            }
            CheckDelegate {
                id: favoriteOnly
                Layout.fillWidth: true
                padding: AppStyle.spacingXs
                visible: control.smart
                contentItem: Text {
                    leftPadding: Math.round(28 * AppStyle.scale)
                    text: "Only favorites"
                    color: Theme.textPrimary
                    font.family: AppStyle.fontFamily
                    font.pixelSize: AppStyle.fontSizeSmall
                    verticalAlignment: Text.AlignVCenter
                }
            }
            CheckDelegate {
                id: unplayedOnly
                Layout.fillWidth: true
                padding: AppStyle.spacingXs
                visible: control.smart
                contentItem: Text {
                    leftPadding: Math.round(28 * AppStyle.scale)
                    text: "Only unplayed (never launched)"
                    color: Theme.textPrimary
                    font.family: AppStyle.fontFamily
                    font.pixelSize: AppStyle.fontSizeSmall
                    verticalAlignment: Text.AlignVCenter
                }
            }
            FieldLabel {
                text: "Platforms"
                visible: control.smart && platformRepeater.count > 0
            }
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2
                visible: control.smart
                Repeater {
                    id: platformRepeater
                    model: PlatformModel
                    delegate: CheckDelegate {
                        required property var model
                        Layout.fillWidth: true
                        padding: AppStyle.spacingXs
                        checked: control.selectedPlatformIds.indexOf(model.platformId) !== -1
                        contentItem: Text {
                            leftPadding: Math.round(28 * AppStyle.scale)
                            text: model.displayName
                            color: Theme.textPrimary
                            font.family: AppStyle.fontFamily
                            font.pixelSize: AppStyle.fontSizeSmall
                            verticalAlignment: Text.AlignVCenter
                        }
                        onToggled: control.selectedPlatformIds = control.toggleId(control.selectedPlatformIds, model.platformId)
                    }
                }
            }
            FieldLabel {
                text: "Genre contains"
                visible: control.smart
            }
            FLTextField {
                id: genreInput
                Layout.fillWidth: true
                visible: control.smart
                placeholderText: "e.g. RPG"
            }
            RowLayout {
                Layout.fillWidth: true
                spacing: AppStyle.spacingSm
                visible: control.smart
                ColumnLayout {
                    Layout.fillWidth: true
                    FieldLabel {
                        text: "Developer contains"
                    }
                    FLTextField {
                        id: developerInput
                        Layout.fillWidth: true
                    }
                }
                ColumnLayout {
                    Layout.fillWidth: true
                    FieldLabel {
                        text: "Publisher contains"
                    }
                    FLTextField {
                        id: publisherInput
                        Layout.fillWidth: true
                    }
                }
            }
            RowLayout {
                Layout.fillWidth: true
                spacing: AppStyle.spacingSm
                visible: control.smart
                ColumnLayout {
                    Layout.fillWidth: true
                    FieldLabel {
                        text: "Year from"
                    }
                    FLTextField {
                        id: yearMinInput
                        Layout.fillWidth: true
                        placeholderText: "1990"
                        inputMethodHints: Qt.ImhDigitsOnly
                    }
                }
                ColumnLayout {
                    Layout.fillWidth: true
                    FieldLabel {
                        text: "Year to"
                    }
                    FLTextField {
                        id: yearMaxInput
                        Layout.fillWidth: true
                        placeholderText: "1999"
                        inputMethodHints: Qt.ImhDigitsOnly
                    }
                }
            }
            RowLayout {
                Layout.fillWidth: true
                spacing: AppStyle.spacingSm
                visible: control.smart
                ColumnLayout {
                    Layout.fillWidth: true
                    FieldLabel {
                        text: "Min. minutes played"
                    }
                    FLTextField {
                        id: minMinutesInput
                        Layout.fillWidth: true
                        placeholderText: "60"
                        inputMethodHints: Qt.ImhDigitsOnly
                    }
                }
                ColumnLayout {
                    Layout.fillWidth: true
                    FieldLabel {
                        text: "Played in last N days"
                    }
                    FLTextField {
                        id: lastDaysInput
                        Layout.fillWidth: true
                        placeholderText: "30"
                        inputMethodHints: Qt.ImhDigitsOnly
                    }
                }
            }
        }
    }
}
