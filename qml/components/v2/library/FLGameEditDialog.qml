import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// TODO
// Edit a library game's name + metadata + artwork. Fields load from a single-row
// mirror of the entry and write back through the model's setData (model.<role> =
// value): the name marks itself user-set, the rest persist as metadata. Open with
// loadAndOpen(entryId, contentHash, platformId)
FirelightDialog {
    id: control

    property int entryId: -1
    property string contentHash: ""
    property int platformId: -1

    headerText: "Edit game"
    acceptText: "Save"

    function loadAndOpen(id, hash, platform) {
        control.entryId = id;
        control.contentHash = hash;
        control.platformId = platform;
        control.open();
        Qt.callLater(control._loadFields);
    }

    function _loadFields() {
        if (rowMirror.count === 0) {
            return;
        }
        const m = rowMirror.itemAt(0).model;
        nameInput.text = m.displayName || "";
        descriptionInput.text = m.description || "";
        developerInput.text = m.developer || "";
        publisherInput.text = m.publisher || "";
        yearInput.text = (m.releaseYear && m.releaseYear > 0) ? String(m.releaseYear) : "";
        genreInput.text = m.genres || "";
    }

    onAccepted: {
        if (rowMirror.count > 0) {
            rowMirror.itemAt(0).apply(nameInput.text.trim(), descriptionInput.text, developerInput.text.trim(), publisherInput.text.trim(), parseInt(yearInput.text) || 0, genreInput.text.trim());
        }
    }

    // Mirror of this entry's row so field writes go through the model's setData
    // rather than bespoke setter invokables
    SortFilterProxyModel {
        id: rowProxy
        model: LibraryEntryModel
        filters: ValueFilter {
            roleName: "entryId"
            value: control.entryId
            enabled: control.entryId !== -1
        }
    }
    Item {
        visible: false
        Repeater {
            id: rowMirror
            model: rowProxy
            delegate: Item {
                required property var model
                function apply(name, description, developer, publisher, year, genres) {
                    model.displayName = name;
                    model.description = description;
                    model.developer = developer;
                    model.publisher = publisher;
                    model.releaseYear = year;
                    model.genres = genres;
                }
            }
        }
    }

    GameArtPickerDialog {
        id: artPicker
    }

    component FieldLabel: Text {
        color: Theme.textMuted
        font.family: AppStyle.fontFamily
        font.pixelSize: AppStyle.fontSizeSmall
    }

    contentItem: ScrollView {
        implicitWidth: Math.round(460 * AppStyle.scale)
        implicitHeight: Math.round(400 * AppStyle.scale)
        clip: true
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        ColumnLayout {
            width: control.availableWidth - AppStyle.spacingLg
            spacing: AppStyle.spacingSm

            FieldLabel {
                text: "Name"
            }
            FLTextField {
                id: nameInput
                Layout.fillWidth: true
                placeholderText: "Game name"
            }

            FLButton {
                Layout.topMargin: AppStyle.spacingXs
                compact: true
                variant: "default"
                text: "Change artwork…"
                onClicked: artPicker.openFor(control.contentHash, nameInput.text, control.platformId)
            }

            FieldLabel {
                text: "Description"
                Layout.topMargin: AppStyle.spacingSm
            }
            FLTextField {
                id: descriptionInput
                Layout.fillWidth: true
                placeholderText: "Optional"
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: AppStyle.spacingSm
                ColumnLayout {
                    Layout.fillWidth: true
                    FieldLabel {
                        text: "Developer"
                    }
                    FLTextField {
                        id: developerInput
                        Layout.fillWidth: true
                    }
                }
                ColumnLayout {
                    Layout.fillWidth: true
                    FieldLabel {
                        text: "Publisher"
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
                ColumnLayout {
                    Layout.fillWidth: true
                    FieldLabel {
                        text: "Release year"
                    }
                    FLTextField {
                        id: yearInput
                        Layout.fillWidth: true
                        placeholderText: "1995"
                        inputMethodHints: Qt.ImhDigitsOnly
                    }
                }
                ColumnLayout {
                    Layout.fillWidth: true
                    FieldLabel {
                        text: "Genre"
                    }
                    FLTextField {
                        id: genreInput
                        Layout.fillWidth: true
                    }
                }
            }
        }
    }
}
