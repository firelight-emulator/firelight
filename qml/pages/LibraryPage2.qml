pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

FocusScope {
    GridView {
        id: listView
        clip: true
        width: Math.min(Math.floor(parent.width / cellContentWidth), 8) * cellContentWidth
        // width: Math.max(5, (Math.min(Math.floor(width / listView.cellContentWidth), 8)) * listView.cellContentWidth)
        anchors.horizontalCenter: parent.horizontalCenter
        height: parent.height
        // anchors.horizontalCenter: parent.horizontalCenter
        // width: Math.min(model.count, Math.floor(parent.width/cellWidth)) * cellWidth
        // anchors.horizontalCenter: parent.horizontalCenter

        // property var actualWidth: Math.max(5, (Math.min(Math.floor(width / listView.cellContentWidth), 8)) * listView.cellContentWidth)

        // Set actualWidth to the width of the number of items that can fit in the view (max of 8)
        // If the number of items is less than 8, set the width to the width of the items
        // If the number of items is greater than 8, set the width to the width of 8 items
        //     property var actualWidth: Math.min(Math.floor(width / listView.cellWidth), 8) * listView.cellWidth + 16

        // property var actualWidth: listView.count > 8 ? listView.cellWidth * 8 : listView.cellContentWidth * listView.count
        // onActualWidthChanged: function() {
        //     console.log("Actual width changed: ", actualWidth)
        // }
        // leftMargin: (width - actualWidth) / 2
        // rightMargin: (width - actualWidth) / 2

        // property var ww: Math.max(5, (Math.min(Math.floor(parent.width / libraryGrid.cellContentWidth), 8)) * libraryGrid.cellContentWidth)

        // ScrollBar.vertical: ScrollBar {
        //     // policy: ScrollBar.AlwaysOn
        // }

        function itemsPerRow() {
            return Math.floor(width / cellContentWidth);
        }

        function rowWidth() {
            return itemsPerRow() * cellContentWidth;
        }

        // onWidthChanged: function() {
        //
        //     console.log("Used: " + rowWidth() + ", Extra: " + (width - rowWidth()) + ", ContentWidth: " + contentWidth)
        // }

        // onWidthChanged: {
        //     leftMargin = Math.round(Math.ceil((parent.width - rowWidth())) / 2)
        //     rightMargin = leftMargin
        //
        //     // if (leftMargin % 2 !== 0) {
        //     //     leftMargin += 1
        //     // }
        //     // if (rightMargin % 2 !== 0) {
        //     //     rightMargin += 1
        //     // }
        // }
        //
        // onLeftMarginChanged: {
        //     console.log("Width: ", width)
        //     console.log("Items per row: ", itemsPerRow())
        //     console.log("Row width: ", rowWidth())
        //     console.log("Margin: ", leftMargin)
        //
        // }

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
        model: library_short_model
        boundsBehavior: Flickable.StopAtBounds

        delegate: GameGridItemDelegate {
            cellSpacing: listView.cellSpacing
            cellWidth: listView.cellContentWidth
            cellHeight: listView.cellContentHeight
        }
    }
}

