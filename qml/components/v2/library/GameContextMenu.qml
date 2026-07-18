import QtQuick
import QtQuick.Controls
import Firelight 1.0

// Right-click menu for a game tile / list row. Acts on `targetIds` (the whole
// multi-selection when the clicked game is part of it, otherwise just that one)
// Global actions (play, favorite, view media) are handled here; the two that
// need a shared host resource (the art picker, the add-to-folder dialog) are
// raised as signals
RightClickMenu {
    id: menu

    property var targetIds: []
    property int primaryEntryId: -1
    property bool primaryFavorite: false
    property string contentHash: ""
    property string displayName: ""
    property int platformId: -1

    signal requestAddToFolder(var entryIds)
    signal requestChangeArt(string contentHash, string displayName, int platformId)

    function _applyFavorite(fav) {
        for (var i = 0; i < menu.targetIds.length; i++)
            LibraryEntryModel.setEntryFavorite(menu.targetIds[i], fav);
    }

    RightClickMenuItem {
        text: menu.targetIds.length > 1 ? "Play first game" : "Play"
        onTriggered: EmulationService.loadEntry(menu.primaryEntryId)
    }
    RightClickMenuItem {
        text: menu.primaryFavorite ? "Remove from favorites" : "Add to favorites"
        onTriggered: menu._applyFavorite(!menu.primaryFavorite)
    }
    RightClickMenuItem {
        text: menu.targetIds.length > 1
              ? "Add " + menu.targetIds.length + " to folder…"
              : "Add to folder…"
        onTriggered: menu.requestAddToFolder(menu.targetIds)
    }
    RightClickMenuItem {
        text: "Change artwork"
        onTriggered: menu.requestChangeArt(menu.contentHash, menu.displayName, menu.platformId)
    }
    RightClickMenuItem {
        text: "View media"
        onTriggered: Router.navigateTo("/gallery/games/" + menu.contentHash)
    }
}
