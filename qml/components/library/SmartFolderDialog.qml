import QtQuick
import QtQuick.Controls
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Effects
import Firelight 1.0

// Editor for a smart folder's name + criteria. Produces the SmartFolderCriteria
// JSON consumed by the C++ evaluator (keys must match smart_folder.cpp's parse).
// Create: leave editFolderId at -1, open(). Edit: set editFolderId + call
// loadFromJson(existingJson) + set folderName, then open().
FirelightDialog {
    id: control

    // -1 = creating a new folder; otherwise the id being edited.
    property int editFolderId: -1
    property alias folderName: nameInput.text

    headerText: editFolderId === -1 ? "New smart folder" : "Edit smart folder"
    acceptText: "Save"

    // Selection state for the multi-select lists (arrays of ids).
    property var selectedDirIds: []
    property var selectedPlatformIds: []

    // Appearance (persisted separately from the criteria JSON).
    property string selectedColor: ""
    property string selectedSortRole: ""
    property bool selectedSortAscending: true

    // The available folder accent colors ("" = none).
    readonly property var swatchColors: ["", "#e5484d", "#f76b15", "#f5d90a",
        "#46a758", "#0091ff", "#8e4ec6", "#e93d82"]

    // Game-sort options for the folder's remembered sort (role names must match
    // EntryListModel's roles).
    readonly property var sortOptions: [
        { label: "Default (name)", role: "" },
        { label: "Name", role: "displayName" },
        { label: "Recently played", role: "lastPlayedAt" },
        { label: "Most played", role: "numSecondsPlayed" },
        { label: "Release year", role: "releaseYear" }
    ]

    // Lets a future edit entry point prefill the appearance controls.
    function loadAppearance(color, sortRole, ascending) {
        selectedColor = color || "";
        selectedSortRole = sortRole || "";
        selectedSortAscending = ascending !== false;
    }

    signal saved()

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
        selectedDirIds = [];
        selectedPlatformIds = [];
        selectedColor = "";
        selectedSortRole = "";
        selectedSortAscending = true;
    }

    function loadFromJson(jsonStr) {
        resetFields();
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
        favoriteOnly.checked = c.favorite === true;
        unplayedOnly.checked = c.unplayed === true;
        selectedDirIds = c.contentDirectoryIds ? c.contentDirectoryIds.slice() : [];
        selectedPlatformIds = c.platformIds ? c.platformIds.slice() : [];
        // The rolling "last N days" window round-trips now (it's stored as a day
        // count, not an absolute timestamp).
        lastDaysInput.text = c.playedWithinDays !== undefined ? String(c.playedWithinDays) : "";
    }

    // Prefill the fields for a common smart folder. Keeps a name the user has
    // already typed; otherwise names the folder after the template.
    function applyTemplate(kind) {
        var priorName = nameInput.text;
        resetFields();
        nameInput.text = priorName;
        if (kind === "recent") {
            lastDaysInput.text = "14";
            if (!priorName.trim()) nameInput.text = "Recently played";
        } else if (kind === "never") {
            unplayedOnly.checked = true;
            if (!priorName.trim()) nameInput.text = "Never played";
        } else if (kind === "favorites") {
            favoriteOnly.checked = true;
            if (!priorName.trim()) nameInput.text = "Favorites";
        } else if (kind === "played") {
            minMinutesInput.text = "60";
            if (!priorName.trim()) nameInput.text = "Sunk hours in";
        }
    }

    function buildFilterJson() {
        var c = {};
        if (selectedDirIds.length > 0)
            c.contentDirectoryIds = selectedDirIds;
        if (pathContainsInput.text.trim().length > 0)
            c.pathContains = pathContainsInput.text.trim();
        if (selectedPlatformIds.length > 0)
            c.platformIds = selectedPlatformIds;
        if (favoriteOnly.checked)
            c.favorite = true;
        if (genreInput.text.trim().length > 0)
            c.genres = [genreInput.text.trim()];
        if (developerInput.text.trim().length > 0)
            c.developer = developerInput.text.trim();
        if (publisherInput.text.trim().length > 0)
            c.publisher = publisherInput.text.trim();
        if (yearMinInput.text.trim().length > 0)
            c.yearMin = parseInt(yearMinInput.text);
        if (yearMaxInput.text.trim().length > 0)
            c.yearMax = parseInt(yearMaxInput.text);
        if (minMinutesInput.text.trim().length > 0)
            c.minSecondsPlayed = parseInt(minMinutesInput.text) * 60;
        if (lastDaysInput.text.trim().length > 0)
            c.playedWithinDays = parseInt(lastDaysInput.text);
        if (unplayedOnly.checked)
            c.unplayed = true;
        return JSON.stringify(c);
    }

    onAboutToShow: {
        if (editFolderId === -1) {
            resetFields();
        }
    }

    onAccepted: {
        var json = buildFilterJson();
        var id = editFolderId;
        if (editFolderId === -1) {
            id = LibraryFolderModel.addSmartFolder(nameInput.text.trim(), json);
        } else {
            LibraryFolderModel.updateSmartFolder(editFolderId, json);
        }
        if (id !== -1) {
            LibraryFolderModel.setFolderColor(id, selectedColor);
            LibraryFolderModel.setFolderSort(id, selectedSortRole, selectedSortAscending);
        }
        LibraryEntryModel.invalidateSmartFolderCache();
        control.saved();
    }

    // A dark themed single-line text field.
    component ThemedField: Pane {
        id: fieldPane
        property alias text: fieldInput.text
        property string placeholder: ""
        property int inputHints: Qt.ImhNone
        Layout.fillWidth: true
        implicitHeight: 40
        padding: 0
        background: Rectangle {
            color: Theme.surface
            radius: 4
        }
        contentItem: Item {
            Text {
                anchors.fill: parent
                anchors.leftMargin: 10
                font.pixelSize: AppStyle.fontSizeSmall
                font.family: Constants.regularFontFamily
                color: Theme.textMuted
                text: fieldPane.placeholder
                verticalAlignment: Text.AlignVCenter
                visible: fieldInput.length === 0
            }
            TextInput {
                id: fieldInput
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.rightMargin: 10
                clip: true
                inputMethodHints: fieldPane.inputHints
                font.family: Constants.regularFontFamily
                font.pixelSize: AppStyle.fontSizeSmall
                color: Theme.textPrimary
                verticalAlignment: Text.AlignVCenter
            }
        }
    }

    component SectionLabel: Text {
        Layout.fillWidth: true
        Layout.topMargin: 8
        color: Theme.textPrimary
        font.family: Constants.regularFontFamily
        font.pixelSize: AppStyle.fontSizeSmall
        font.weight: Font.DemiBold
    }

    component FieldLabel: Text {
        color: Theme.textMuted
        font.family: Constants.regularFontFamily
        font.pixelSize: AppStyle.fontSizeSmall
    }

    contentItem: ScrollView {
        implicitWidth: 460
        implicitHeight: 420
        clip: true
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        ColumnLayout {
            width: control.availableWidth - 16
            spacing: 8

            FieldLabel { text: "Name" }
            ThemedField {
                id: nameInput
                placeholder: "Smart folder name"
            }

            FieldLabel {
                text: "Start from a template"
                visible: control.editFolderId === -1
            }
            Flow {
                Layout.fillWidth: true
                visible: control.editFolderId === -1
                spacing: 6
                Repeater {
                    model: [
                        { label: "Recently played", kind: "recent" },
                        { label: "Never played", kind: "never" },
                        { label: "Favorites", kind: "favorites" },
                        { label: "Sunk hours in", kind: "played" }
                    ]
                    delegate: FLButton {
                        required property var modelData
                        size: "sm"
                        variant: "secondary"
                        text: modelData.label
                        onClicked: control.applyTemplate(modelData.kind)
                    }
                }
            }

            // --- Source ---
            SectionLabel { text: "Source" }
            FieldLabel {
                text: "Leave everything below empty to draw from the whole library."
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }

            FieldLabel { text: "Path contains" }
            ThemedField {
                id: pathContainsInput
                placeholder: "e.g. snes"
            }

            FieldLabel {
                text: "Content directories"
                visible: dirRepeater.count > 0
            }
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2
                Repeater {
                    id: dirRepeater
                    model: ContentDirectoryModel
                    delegate: CheckDelegate {
                        required property var model
                        Layout.fillWidth: true
                        padding: 4
                        checked: control.selectedDirIds.indexOf(model.directory_id) !== -1
                        contentItem: Text {
                            leftPadding: 28
                            text: model.path
                            color: Theme.textPrimary
                            elide: Text.ElideMiddle
                            font.family: Constants.regularFontFamily
                            font.pixelSize: AppStyle.fontSizeSmall
                            verticalAlignment: Text.AlignVCenter
                        }
                        onToggled: control.selectedDirIds = control.toggleId(control.selectedDirIds, model.directory_id)
                    }
                }
            }

            // --- Filters ---
            SectionLabel { text: "Filters" }

            CheckDelegate {
                id: favoriteOnly
                Layout.fillWidth: true
                padding: 4
                contentItem: Text {
                    leftPadding: 28
                    text: "Only favorites"
                    color: Theme.textPrimary
                    font.family: Constants.regularFontFamily
                    font.pixelSize: AppStyle.fontSizeSmall
                    verticalAlignment: Text.AlignVCenter
                }
            }

            CheckDelegate {
                id: unplayedOnly
                Layout.fillWidth: true
                padding: 4
                contentItem: Text {
                    leftPadding: 28
                    text: "Only unplayed (never launched)"
                    color: Theme.textPrimary
                    font.family: Constants.regularFontFamily
                    font.pixelSize: AppStyle.fontSizeSmall
                    verticalAlignment: Text.AlignVCenter
                }
            }

            FieldLabel {
                text: "Platforms"
                visible: platformRepeater.count > 0
            }
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2
                Repeater {
                    id: platformRepeater
                    model: PlatformModel
                    delegate: CheckDelegate {
                        required property var model
                        Layout.fillWidth: true
                        padding: 4
                        checked: control.selectedPlatformIds.indexOf(model.platformId) !== -1
                        contentItem: Text {
                            leftPadding: 28
                            text: model.displayName
                            color: Theme.textPrimary
                            font.family: Constants.regularFontFamily
                            font.pixelSize: AppStyle.fontSizeSmall
                            verticalAlignment: Text.AlignVCenter
                        }
                        onToggled: control.selectedPlatformIds = control.toggleId(control.selectedPlatformIds, model.platformId)
                    }
                }
            }

            FieldLabel { text: "Genre contains" }
            ThemedField {
                id: genreInput
                placeholder: "e.g. RPG"
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 8
                ColumnLayout {
                    Layout.fillWidth: true
                    FieldLabel { text: "Developer contains" }
                    ThemedField { id: developerInput }
                }
                ColumnLayout {
                    Layout.fillWidth: true
                    FieldLabel { text: "Publisher contains" }
                    ThemedField { id: publisherInput }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 8
                ColumnLayout {
                    Layout.fillWidth: true
                    FieldLabel { text: "Year from" }
                    ThemedField {
                        id: yearMinInput
                        placeholder: "1990"
                        inputHints: Qt.ImhDigitsOnly
                    }
                }
                ColumnLayout {
                    Layout.fillWidth: true
                    FieldLabel { text: "Year to" }
                    ThemedField {
                        id: yearMaxInput
                        placeholder: "1999"
                        inputHints: Qt.ImhDigitsOnly
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 8
                ColumnLayout {
                    Layout.fillWidth: true
                    FieldLabel { text: "Min. minutes played" }
                    ThemedField {
                        id: minMinutesInput
                        placeholder: "60"
                        inputHints: Qt.ImhDigitsOnly
                    }
                }
                ColumnLayout {
                    Layout.fillWidth: true
                    FieldLabel { text: "Played in last N days" }
                    ThemedField {
                        id: lastDaysInput
                        placeholder: "30"
                        inputHints: Qt.ImhDigitsOnly
                    }
                }
            }

            // --- Appearance ---
            SectionLabel { text: "Appearance" }

            FieldLabel { text: "Accent color" }
            Flow {
                Layout.fillWidth: true
                spacing: 8
                Repeater {
                    model: control.swatchColors
                    delegate: Rectangle {
                        required property string modelData
                        width: 26
                        height: 26
                        radius: 13
                        color: modelData === "" ? "transparent" : modelData
                        border.width: control.selectedColor === modelData ? 3 : 1
                        border.color: control.selectedColor === modelData
                            ? "white"
                            : (modelData === "" ? ColorPalette.neutral500 : "#00000040")

                        // "None" swatch shows a slash.
                        Rectangle {
                            visible: parent.modelData === ""
                            width: parent.width - 8
                            height: 2
                            radius: 1
                            color: Theme.textMuted
                            anchors.centerIn: parent
                            rotation: 45
                        }

                        TapHandler {
                            onTapped: control.selectedColor = parent.modelData
                        }
                    }
                }
            }

            FieldLabel { text: "Default game sort" }
            RowLayout {
                Layout.fillWidth: true
                spacing: 8
                FLComboBox {
                    id: sortCombo
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
                Button {
                    text: control.selectedSortAscending ? "Ascending" : "Descending"
                    enabled: control.selectedSortRole !== ""
                    onClicked: control.selectedSortAscending = !control.selectedSortAscending
                }
            }
        }
    }
}
