import QtQuick
import QtQuick.Layouts

// TODO
// A row of stars: `value` of `count` are filled. Interactive by default —
// clicking a star emits `edited` with the new value (clicking the current value
// clears to 0). Set `interactive: false` for a read-only display
//
//   FLStarRating { value: 3 }                                  // display
//   FLStarRating { value: entry.rating; onEdited: v => entry.rating = v }
RowLayout {
    id: control

    property int count: 5
    property int value: 0
    property bool interactive: true
    property int starSize: AppStyle.iconSizeMd
    property color filledColor: Theme.gold
    property color emptyColor: Theme.textMuted

    spacing: 0

    Repeater {
        model: control.count
        delegate: Item {
            id: star
            required property int index
            readonly property bool lit: control.value > index
            Layout.preferredWidth: control.starSize
            Layout.preferredHeight: control.starSize

            Icon {
                anchors.centerIn: parent
                name: "star"
                weight: Font.ExtraLight
                filled: star.lit
                size: control.starSize
                color: star.lit ? control.filledColor : control.emptyColor
            }

            TapHandler {
                enabled: control.interactive
                onTapped: control.value = (control.value === star.index + 1 ? 0 : star.index + 1)
            }

            HoverHandler {
                enabled: control.interactive
                cursorShape: Qt.PointingHandCursor
            }
        }
    }
}
