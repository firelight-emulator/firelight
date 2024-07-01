import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQml.Models

GridView {
    id: listView
    clip: true

    onActiveFocusChanged: {
        console.log("Active focus changed")
    }

    ScrollBar.vertical: ScrollBar {
        // policy: ScrollBar.AlwaysOn
    }

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

    model: ListModel {
        ListElement {
            thing: "074224"
        }
        ListElement {
            thing: "066393"
        }
        ListElement {
            thing: "069326"
        }
        ListElement {
            thing: "047942"
        }
        ListElement {
            thing: "091553"
        }
        ListElement {
            thing: "091555"
        }
        ListElement {
            thing: "029672"
        }
        ListElement {
            thing: "040625"
        }
        ListElement {
            thing: "027488"
        }
        ListElement {
            thing: "074224"
        }
        ListElement {
            thing: "066393"
        }
        ListElement {
            thing: "069326"
        }
        ListElement {
            thing: "047942"
        }
        ListElement {
            thing: "091553"
        }
        ListElement {
            thing: "091555"
        }
        ListElement {
            thing: "029672"
        }
        ListElement {
            thing: "040625"
        }
        ListElement {
            thing: "027488"
        }
        ListElement {
            thing: "066393"
        }
        ListElement {
            thing: "069326"
        }
        ListElement {
            thing: "047942"
        }
        ListElement {
            thing: "040155"
        }
        ListElement {
            thing: "091553"
        }
        ListElement {
            thing: "091555"
        }
        ListElement {
            thing: "040625"
        }
        ListElement {
            thing: "027488"
        }
    }
    // model: library_short_model
    boundsBehavior: Flickable.StopAtBounds

    delegate: FocusScope {
        width: GridView.view.cellWidth
        height: GridView.view.cellHeight
        Button {
            anchors {
                fill: parent
                margins: listView.cellSpacing / 2
            }
            focus: true
            onClicked: {
                console.log("Button clicked: " + model.thing)
                // parent.GridView.view.currentIndex = index
            }

            hoverEnabled: true

            background: Rectangle {
                color: parent.hovered ? "#3a3e45" : "#25282C"
            }

            // contentItem: Text {
            //     anchors.centerIn: parent
            //     text: model.display_name
            //     color: "white"
            // }

            contentItem: Item {
                Image {
                    id: image
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    sourceSize.width: parent.width
                    source: "https://media.retroachievements.org/Images/" + model.thing + ".png"
                    fillMode: Image.PreserveAspectFit
                    cache: true
                }
                Text {
                    anchors.top: image.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    text: model.thing
                    color: "white"
                    font.pixelSize: 12
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }
    }
}
