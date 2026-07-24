import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// TODO
// Inline advanced-filter bar that expands under the toolbar (replacing the old
// popup + chips). Holds the quick toggles plus play-time/decade/genre and reads
// and writes the filter state on the GameView passed as `view`
Rectangle {
    id: root

    property var view
    property bool expanded: false

    Layout.fillWidth: true
    Layout.preferredHeight: expanded ? content.implicitHeight + AppStyle.spacingMd * 2 : 0
    clip: true
    visible: Layout.preferredHeight > 0
    radius: AppStyle.radiusMd
    color: Theme.backgroundInset

    Behavior on Layout.preferredHeight {
        NumberAnimation {
            duration: AppStyle.durationFast
            easing.type: Easing.InOutQuad
        }
    }

    Flow {
        id: content
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: AppStyle.spacingMd
        spacing: AppStyle.spacingSm

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
            compact: true
            text: "Has achievements"
            variant: root.view.filterHasAchievements ? "primary" : "default"
            onClicked: root.view.filterHasAchievements = !root.view.filterHasAchievements
        }
        FLButton {
            compact: true
            text: "Completed"
            variant: root.view.filterCompleted ? "primary" : "default"
            onClicked: root.view.filterCompleted = !root.view.filterCompleted
        }

        FLComboBox {
            model: root.view.playTimeOptions
            textRole: "label"
            currentIndex: {
                for (var i = 0; i < root.view.playTimeOptions.length; i++) {
                    if (root.view.playTimeOptions[i].value === root.view.filterPlayTime) {
                        return i;
                    }
                }
                return 0;
            }
            onActivated: function (index) {
                root.view.filterPlayTime = root.view.playTimeOptions[index].value;
            }
        }
        FLComboBox {
            model: root.view.decadeOptions
            textRole: "label"
            currentIndex: {
                for (var i = 0; i < root.view.decadeOptions.length; i++) {
                    if (root.view.decadeOptions[i].value === root.view.filterDecade) {
                        return i;
                    }
                }
                return 0;
            }
            onActivated: function (index) {
                root.view.filterDecade = root.view.decadeOptions[index].value;
            }
        }
        FLSearchField {
            id: genreField
            placeholder: "Genre…"
            onTextChanged: {
                if (root.view.filterGenre !== text) {
                    root.view.filterGenre = text;
                }
            }
            Connections {
                target: root.view
                function onFilterGenreChanged() {
                    if (genreField.text !== root.view.filterGenre) {
                        genreField.text = root.view.filterGenre;
                    }
                }
            }
        }

        FLButton {
            compact: true
            variant: "subtle"
            text: "Clear filters"
            visible: root.view.anyPopupFilter
            onClicked: root.view.clearPopupFilters()
        }
    }
}
