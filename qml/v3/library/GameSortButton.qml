// TODO: NEEDS REVIEW
import QtQuick
import QtQuick.Layouts
import Firelight 1.0

FLIconButton {
    id: control

    required property LibraryEntrySortFilterModel entryModel
    property bool showsPinRow: false

    Layout.alignment: Qt.AlignHCenter
    iconName: "list-arrow"
    tooltipText: "Sort"
    compact: false
    iconColor: gameSortPopup.visible ? Theme.switch2Color : Theme.textPrimary
    onClicked: gameSortPopup.opened ? gameSortPopup.close() : gameSortPopup.open()

    FLRadioMenu {
        id: gameSortPopup
        x: control.width + AppStyle.spacingXs

        model: control.entryModel.sortOptions
        currentValue: control.entryModel.sortRole

        onActivated: value => {
            control.entryModel.sortRole = value;
        }

        FLMenuSeparator {
            visible: control.showsPinRow
        }

        FLMenuItem {
            label: control.entryModel.sortPinnedToOpenFolder ? "Reset to default sort" : "Using the default sort"
            enabled: control.entryModel.sortPinnedToOpenFolder
            visible: control.showsPinRow
            onClicked: control.entryModel.sortPinnedToOpenFolder = false
        }
    }
}
