import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// TODO
// The library view's toolbar: search on the left, then the Filters and Display
// popups and the details toggle on the right. Reads/writes the filter + view state
// on the GameView passed as `view`
RowLayout {
    id: root

    property var view

    spacing: AppStyle.spacingSm

    FLSearchField {
        id: searchField
        Layout.preferredWidth: Math.round(240 * AppStyle.scale)
        placeholder: qsTr("Search this view")
        onTextChanged: {
            if (root.view.filterText !== text) {
                root.view.filterText = text;
            }
        }
        Connections {
            target: root.view
            function onFilterTextChanged() {
                if (searchField.text !== root.view.filterText) {
                    searchField.text = root.view.filterText;
                }
            }
        }
    }

    Item {
        Layout.fillWidth: true
    }

    FLButton {
        id: filtersButton
        compact: true
        text: "Filters"
        variant: root.view.anyPopupFilter ? "primary" : "default"
        onClicked: filtersPopup.opened ? filtersPopup.close() : filtersPopup.open()

        GameFiltersPopup {
            id: filtersPopup
            x: filtersButton.width - width
            y: filtersButton.height + AppStyle.spacingXs
            view: root.view
        }
    }

    FLButton {
        id: displayButton
        compact: true
        iconName: root.view.viewMode === "grid" ? "grid_view" : "view_list"
        text: root.view.currentSortLabel
        trailingIconName: root.view.sortAscending ? "arrow-up" : "arrow-down"
        onClicked: displayPopup.opened ? displayPopup.close() : displayPopup.open()

        GameDisplayPopup {
            id: displayPopup
            x: displayButton.width - width
            y: displayButton.height + AppStyle.spacingXs
            view: root.view
        }
    }

    FLButton {
        compact: true
        text: "Details"
        variant: root.view.detailOpen ? "primary" : "default"
        onClicked: root.view.detailOpen = !root.view.detailOpen
    }
}
