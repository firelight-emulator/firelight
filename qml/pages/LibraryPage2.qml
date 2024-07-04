pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

FocusScope {
    property alias model: listView.model

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

        // snapMode: GridView.SnapToRow

        // cacheBuffer: 100
        // reuseItems: true
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

        // height: contentHeight

        readonly property int cellSpacing: 12
        readonly property int cellContentWidth: 180 + cellSpacing
        readonly property int cellContentHeight: 260 + cellSpacing

        // readonly property int cellSpacing: 4
        // readonly property int cellContentWidth: 120 + cellSpacing
        // readonly property int cellContentHeight: 120 + cellSpacing

        cellWidth: cellContentWidth
        cellHeight: cellContentHeight
        focus: true

        currentIndex: 0

        // model: ListModel {
        //     ListElement {
        //         thing: "074224"
        //     }
        //     ListElement {
        //         thing: "066393"
        //     }
        //     ListElement {
        //         thing: "069326"
        //     }
        //     ListElement {
        //         thing: "047942"
        //     }
        //     ListElement {
        //         thing: "091553"
        //     }
        //     ListElement {
        //         thing: "091555"
        //     }
        //     ListElement {
        //         thing: "029672"
        //     }
        //     ListElement {
        //         thing: "040625"
        //     }
        //     ListElement {
        //         thing: "027488"
        //     }
        //     ListElement {
        //         thing: "074224"
        //     }
        //     ListElement {
        //         thing: "066393"
        //     }
        //     ListElement {
        //         thing: "069326"
        //     }
        //     ListElement {
        //         thing: "047942"
        //     }
        //     ListElement {
        //         thing: "091553"
        //     }
        //     ListElement {
        //         thing: "091555"
        //     }
        //     ListElement {
        //         thing: "029672"
        //     }
        //     ListElement {
        //         thing: "040625"
        //     }
        //     ListElement {
        //         thing: "027488"
        //     }
        //     ListElement {
        //         thing: "066393"
        //     }
        //     ListElement {
        //         thing: "069326"
        //     }
        //     ListElement {
        //         thing: "047942"
        //     }
        //     ListElement {
        //         thing: "040155"
        //     }
        //     ListElement {
        //         thing: "091553"
        //     }
        //     ListElement {
        //         thing: "091555"
        //     }
        //     ListElement {
        //         thing: "040625"
        //     }
        //     ListElement {
        //         thing: "027488"
        //     }
        // }
        boundsBehavior: Flickable.StopAtBounds

        delegate: GameGridItemDelegate {
            cellSpacing: listView.cellSpacing
            cellWidth: listView.cellContentWidth
            cellHeight: listView.cellContentHeight
        }
    }
}

