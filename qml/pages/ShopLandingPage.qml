pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

FocusScope {
    id: page
    property alias model: gridView.model

    GridView {
        id: gridView
        objectName: "GridView"
        clip: true
        width: Math.min(Math.floor(parent.width / cellContentWidth), 7) * cellContentWidth
        anchors.horizontalCenter: parent.horizontalCenter
        height: parent.height

        preferredHighlightBegin: 0.33 * gridView.height
        preferredHighlightEnd: 0.66 * gridView.height
        highlightRangeMode: GridView.ApplyRange

        populate: Transition {
            id: popTransition
            SequentialAnimation {
                PauseAnimation {
                    duration: popTransition.ViewTransition.index * 50
                }
                ParallelAnimation {
                    PropertyAnimation {
                        property: "opacity"
                        from: 0
                        to: 1
                        duration: 300
                        easing.type: Easing.InOutQuad
                    }
                    PropertyAnimation {
                        property: "y"
                        from: popTransition.ViewTransition.destination.y + 20
                        to: popTransition.ViewTransition.destination.y
                        duration: 300
                        easing.type: Easing.InOutQuad
                    }
                }
            }
        }

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
                text: "All Mods"
                color: "white"
                font.pointSize: 26
                font.family: Constants.regularFontFamily
                font.weight: Font.DemiBold
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
            }

        }

        readonly property int cellSpacing: 12
        readonly property int cellContentWidth: 360 + cellSpacing
        readonly property int cellContentHeight: 240 + cellSpacing

        cellWidth: cellContentWidth
        cellHeight: cellContentHeight
        focus: true

        currentIndex: 0
        boundsBehavior: Flickable.StopAtBounds

        delegate: ShopGridItemDelegate {
            id: delegate
            opacity: 0
            cellWidth: gridView.cellWidth
            cellHeight: gridView.cellHeight
            cellSpacing: gridView.cellSpacing

            Button {
                anchors.centerIn: parent

                onClicked: function() {
                    page.StackView.view.push(shopItemPage)
                }
            }
        }
    }

    Component {
        id: shopItemPage
        ShopItemPage {
        }
    }
}

