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

    FLButton {
        id: filtersButton
        compact: false
        iconName: "filter"
        text: "Filters"
        variant: (root.view.filtersExpanded || root.view.anyPopupFilter) ? "primary" : "default"
        onClicked: root.view.filtersExpanded = !root.view.filtersExpanded
    }

    Item {
        Layout.fillWidth: true
    }

    FLButton {
        id: displayButton
        compact: false
        iconName: root.view.viewMode === "grid" ? "grid_view" : "view_list"
        text: root.view.viewMode === "grid" ? "Grid" : "List"
        // trailingIconName: root.view.sortAscending ? "arrow-up" : "arrow-down"
        onClicked: displayPopup.opened ? displayPopup.close() : displayPopup.open()

        GameDisplayTypePopup {
            id: displayPopup
            x: displayButton.width - width
            y: displayButton.height + AppStyle.spacingXs
            view: root.view
        }
    }

    // FLButton {
    //     compact: true
    //     text: "Details"
    //     variant: root.view.detailOpen ? "primary" : "default"
    //     onClicked: root.view.detailOpen = !root.view.detailOpen
    // }
}
