import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// TODO
// The library view's toolbar: search, quick filters, the Filters/Display popups,
// and the details toggle. Reads/writes the filter + view state on the GameView
// passed as `view`
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
        compact: true
        iconName: "favorite"
        text: "Favorites"
        variant: root.view.showOnlyFavorites ? "primary" : "default"
        onClicked: root.view.showOnlyFavorites = !root.view.showOnlyFavorites
    }
    FLButton {
        compact: true
        text: "Unplayed"
        variant: root.view.showOnlyUnplayed ? "primary" : "default"
        onClicked: root.view.showOnlyUnplayed = !root.view.showOnlyUnplayed
    }

    FLButton {
        id: filtersButton
        compact: true
        text: "Filters"
        variant: root.view.anyAdvancedFilter ? "primary" : "default"
        onClicked: filtersPopup.opened ? filtersPopup.close() : filtersPopup.open()

        GameFiltersPopup {
            id: filtersPopup
            y: filtersButton.height + AppStyle.spacingXs
            view: root.view
        }
    }

    FLButton {
        id: displayButton
        compact: true
        text: "Display"
        iconName: "tune"
        onClicked: displayPopup.opened ? displayPopup.close() : displayPopup.open()

        GameDisplayPopup {
            id: displayPopup
            x: displayButton.width - width
            y: displayButton.height + AppStyle.spacingXs
            view: root.view
        }
    }

    Item {
        Layout.fillWidth: true
    }

    FLButton {
        compact: true
        text: "Details"
        variant: root.view.detailOpen ? "primary" : "default"
        onClicked: root.view.detailOpen = !root.view.detailOpen
    }
}
