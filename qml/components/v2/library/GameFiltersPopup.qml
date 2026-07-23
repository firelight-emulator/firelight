import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// TODO
// The library's advanced-filter popup. Reads and writes the filter state on the
// GameView passed as `view`
Popup {
    id: root

    property var view

    width: Math.round(260 * AppStyle.scale)
    padding: AppStyle.spacingMd
    modal: false
    focus: true
    // Escape or the Filters button closes it; not press-outside, which the
    // nested combo popups would trip
    closePolicy: Popup.CloseOnEscape

    background: Rectangle {
        color: Theme.surfaceElevated
        radius: AppStyle.radiusMd
        border.color: Theme.border
        border.width: 1
    }

    ColumnLayout {
        width: parent.width
        spacing: AppStyle.spacingSm

        Text {
            text: "Refine"
            color: Theme.textPrimary
            font.family: AppStyle.fontFamily
            font.pixelSize: AppStyle.fontSizeMedium
            font.weight: Font.Bold
        }

        FLButton {
            Layout.fillWidth: true
            compact: true
            text: "Has achievements"
            variant: root.view.filterHasAchievements ? "primary" : "default"
            onClicked: root.view.filterHasAchievements = !root.view.filterHasAchievements
        }
        FLButton {
            Layout.fillWidth: true
            compact: true
            text: "Completed"
            variant: root.view.filterCompleted ? "primary" : "default"
            onClicked: root.view.filterCompleted = !root.view.filterCompleted
        }

        Text {
            text: "Play time"
            color: Theme.textMuted
            font.family: AppStyle.fontFamily
            font.pixelSize: AppStyle.fontSizeSmall
        }
        FLComboBox {
            Layout.fillWidth: true
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

        Text {
            text: "Decade"
            color: Theme.textMuted
            font.family: AppStyle.fontFamily
            font.pixelSize: AppStyle.fontSizeSmall
        }
        FLComboBox {
            Layout.fillWidth: true
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

        Text {
            text: "Genre contains"
            color: Theme.textMuted
            font.family: AppStyle.fontFamily
            font.pixelSize: AppStyle.fontSizeSmall
        }
        FLSearchField {
            id: genreField
            Layout.fillWidth: true
            placeholder: "e.g. RPG"
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
            Layout.fillWidth: true
            compact: true
            variant: "subtle"
            text: "Clear these filters"
            visible: root.view.anyAdvancedFilter
            onClicked: root.view.clearAdvancedFilters()
        }
    }
}
