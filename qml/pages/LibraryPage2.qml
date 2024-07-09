pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

FocusScope {
    id: page
    property alias model: listView.model

    signal openDetails(entryId: int)
    signal startGame(entryId: int)

    GridView {
        id: listView
        objectName: "GridView"
        clip: true
        width: Math.min(Math.floor(parent.width / cellContentWidth), 7) * cellContentWidth
        anchors.horizontalCenter: parent.horizontalCenter
        height: parent.height

        preferredHighlightBegin: 0.33 * listView.height
        preferredHighlightEnd: 0.66 * listView.height
        highlightRangeMode: GridView.ApplyRange

        contentY: 0

        header: Pane {
            id: header

            width: parent.width
            height: 120
            background: Rectangle {
                color: "transparent"
                // border.color: "red"
            }

            horizontalPadding: 8

            Text {
                anchors.fill: parent
                text: "All Games"
                color: "white"
                font.pointSize: 26
                font.family: Constants.regularFontFamily
                font.weight: Font.DemiBold
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
            }

            Rectangle {
                width: 100
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.right: parent.right
            }

        }

        readonly property int cellSpacing: 12
        readonly property int cellContentWidth: 180 + cellSpacing
        readonly property int cellContentHeight: 260 + cellSpacing

        cellWidth: cellContentWidth
        cellHeight: cellContentHeight
        focus: true

        currentIndex: 0
        boundsBehavior: Flickable.StopAtBounds

        delegate: GameGridItemDelegate {
            cellSpacing: listView.cellSpacing
            cellWidth: listView.cellContentWidth
            cellHeight: listView.cellContentHeight

            Component.onCompleted: {
                openDetails.connect(page.openDetails)
                startGame.connect(page.startGame)
            }
        }
    }
}

