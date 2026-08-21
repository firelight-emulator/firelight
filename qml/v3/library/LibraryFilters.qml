import QtQuick

Item {
    property bool pendingShowOnlyFavorites: false
    property bool showOnlyFavorites: false

    property list<int> pendingPlatformIds: []
    property list<int> platformIds: []

    signal pendingValuesChanged

    onPendingShowOnlyFavoritesChanged: pendingValuesChanged()
    onPendingPlatformIdsChanged: pendingValuesChanged()

    function commitAll() {
        showOnlyFavorites = pendingShowOnlyFavorites;
        platformIds = pendingPlatformIds;
    }

    function clearAll() {
    }
}
