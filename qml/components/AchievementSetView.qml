import QtQuick
import QtQuick.Controls
import Firelight 1.0

FocusScope {
    id: root

    required property var contentHash

    AchievementSet {
        id: achievementSet
        contentHash: root.contentHash
    }

    ListView {
        id: theList
        // anchors.left: gameNavList.right
        anchors.fill: parent
        focus: true
        // visible: achievements.numAchievements > 0
        highlightMoveDuration: 80
        highlightMoveVelocity: -1
        highlightRangeMode: InputMethodManager.usingMouse ? ListView.NoHighlightRange : ListView.ApplyRange
        preferredHighlightBegin: 64
        preferredHighlightEnd: height - 64
        model: achievementSet.achievements

        ScrollBar.vertical: ScrollBar { }

        spacing: 0

        delegate: Text {
            required property var model
            color: "white"
            text: model.name
        }
    }




}
