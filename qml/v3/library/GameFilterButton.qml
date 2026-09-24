// TODO: NEEDS REVIEW
import QtQuick
import QtQuick.Layouts
import Firelight 1.0

FLIconButton {
    id: control

    required property LibraryEntrySortFilterModel entryModel
    property bool scopeIsSmart: false

    signal saveRequested
    signal clearRequested

    Layout.alignment: Qt.AlignHCenter
    iconName: "filter-alt"
    tooltipText: "Filter"
    filled: false
    compact: false
    iconColor: filterPopup.visible || control.entryModel.anyFiltersActive ? Theme.switch2Color : Theme.textPrimary
    onClicked: filterPopup.opened ? filterPopup.close() : filterPopup.open()

    Rectangle {
        color: Theme.switch2Color
        height: 6
        width: 6
        radius: width / 2
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.rightMargin: 8
        anchors.bottomMargin: 8
        visible: control.entryModel.anyFiltersActive
    }

    FLMenu {
        id: filterPopup
        x: control.width + AppStyle.spacingXs
        minWidth: 300

        FLButton {
            id: saveFiltersButton
            visible: control.scopeIsSmart
            text: "Save current filters"
            Layout.fillWidth: true
            Layout.leftMargin: AppStyle.spacingSm
            Layout.rightMargin: AppStyle.spacingSm
            Layout.topMargin: AppStyle.spacingSm
            canInteract: control.entryModel.filter.dirty

            FLFocus.focusSound: SoundEffects.menuNavigate
            FLFocus.actions: [
                FLAction {
                    keys: [Qt.Key_Select, Qt.Key_Return, Qt.Key_Enter, Qt.Key_Space]
                    label: qsTr("Select")
                    sound: saveFiltersButton.canInteract ? SoundEffects.openPopup : SoundEffects.cursorBump
                    onTriggered: control.saveRequested()
                }
            ]

            onClicked: control.saveRequested()
        }

        FLButton {
            id: clearButton
            text: control.scopeIsSmart ? "Reset to saved filters" : "Clear all filters"
            Layout.fillWidth: true
            Layout.leftMargin: AppStyle.spacingSm
            Layout.rightMargin: AppStyle.spacingSm
            Layout.topMargin: AppStyle.spacingSm
            Layout.bottomMargin: AppStyle.spacingSm
            canInteract: control.entryModel.anyFiltersActive

            FLFocus.focusSound: SoundEffects.menuNavigate
            FLFocus.actions: [
                FLAction {
                    keys: [Qt.Key_Select, Qt.Key_Return, Qt.Key_Enter, Qt.Key_Space]
                    label: qsTr("Select")
                    sound: clearButton.canInteract ? SoundEffects.openPopup : SoundEffects.cursorBump
                    onTriggered: control.clearRequested()
                }
            ]

            onClicked: {
                control.clearRequested();
            }
        }

        FLCheckboxMenuItem {
            label: "Favorites"
            checked: control.entryModel.filter.favorite === LibraryFilter.Yes
            onSelected: function (selected) {
                control.entryModel.filter.favorite = selected ? LibraryFilter.Yes : LibraryFilter.Unset;
            }
        }

        FLSubmenuItem {
            label: "Platform"
            model: PlatformModel
            textRole: "displayName"
            valueRole: "platformId"

            selectionIsExternal: true
            currentValues: control.entryModel.filter.platformIds

            onCleared: control.entryModel.filter.platformIds = []

            onOptionToggled: (value, selected) => {
                const ids = control.entryModel.filter.platformIds.filter(id => id !== value);

                if (selected) {
                    ids.push(value);
                }

                control.entryModel.filter.platformIds = ids;
            }
        }

        FLSubmenuItem {
            id: playtimeItem
            label: "Time played"
            model: [
                {
                    "text": "Over 1 hour",
                    "value": 60
                },
                {
                    "text": "Over 5 hours",
                    "value": 300
                },
                {
                    "text": "Over 10 hours",
                    "value": 600
                },
                {
                    "text": "Over 25 hours",
                    "value": 1500
                }
            ]

            onCurrentValuesChanged: {
                control.entryModel.filter.minMinutesPlayed = playtimeItem.currentValues.length > 0 ? Math.min(...playtimeItem.currentValues) : -1;
            }
        }

        FLCheckboxMenuItem {
            label: "Never played"
            checked: control.entryModel.filter.unplayed === LibraryFilter.Yes
            onSelected: function (selected) {
                control.entryModel.filter.unplayed = selected ? LibraryFilter.Yes : LibraryFilter.Unset;
            }
        }

        FLSubmenuItem {
            label: "Developer"
            model: []
        }

        FLSubmenuItem {
            label: "Publisher"
            model: []
        }

        FLSubmenuItem {
            label: "Genre"
            model: []
        }

        FLSubmenuItem {
            label: "Tags"
            model: []
        }

        FLCheckboxMenuItem {
            label: "Hide unplayable"
            checked: control.entryModel.filter.playable === LibraryFilter.Yes
            onSelected: function (selected) {
                control.entryModel.filter.playable = selected ? LibraryFilter.Yes : LibraryFilter.Unset;
            }
        }
    }
}
