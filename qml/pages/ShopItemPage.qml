pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Flickable {
    id: page
    // property alias model: gridView.model

    contentHeight: 4000
    Rectangle {
        id: sososo
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 400
        color: "red"
    }

    Pane {
        id: main
        padding: 20
        background: Rectangle {
            color: "black"
            // border.color: "black"
            // border.width: 3
            radius: 24
        }
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.topMargin: 36
        anchors.rightMargin: parent.width / 20
        anchors.leftMargin: parent.width / 20
        height: 400

        contentItem: Item {
            ColumnLayout  {
                id: thing
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: parent.width * 27 / 50

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: width / (16 / 9)
                    color: "grey"
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 78
                    color: "blue"
                }
                Item {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                }
            }
            ColumnLayout {
                spacing: 16
                anchors.leftMargin: 12
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.left: thing.right

                Text {
                    text: "Super Cool Game 2024"
                    font.pixelSize: 26
                    font.family: Constants.regularFontFamily
                    font.weight: Font.Bold
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    color: ColorPalette.neutral100
                }

                Text {
                    text: qsTr("Nintendo 64")
                    font.pixelSize: 15
                    font.family: Constants.regularFontFamily
                    font.weight: Font.Normal
                    wrapMode: Text.WordWrap
                    color: ColorPalette.neutral300
                }
                Item {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                }
                Text {
                    text: "Version: 1.0.0 (latest)"
                    font.pixelSize: 15
                    font.family: Constants.regularFontFamily
                    font.weight: Font.Normal
                    horizontalAlignment: Text.AlignHCenter
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    color: ColorPalette.neutral300
                }

                Button {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 48
                    background: Rectangle {
                        color: "#d14c20"
                        radius: 12
                    }
                    contentItem: Text {
                        text: "Download"
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                        font.pixelSize: 18
                        font.family: Constants.regularFontFamily
                        font.weight: Font.Bold
                        color: "white"
                    }

                    onClicked: {
                        console.log("Install button clicked")
                    }
                }
            }
        }
        // anchors.horizontalCenter: parent.horizontalCenter
    }

    Pane {
        padding: 20
        background: Rectangle {
            color: "transparent"
            // border.color: "black"
            // border.width: 3
            radius: 24
        }
        anchors.top: main.bottom
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.topMargin: 12
        anchors.rightMargin: parent.width / 20
        anchors.leftMargin: parent.width / 20

        contentItem: Text {
            text: "Lorem ipsum dolor sit amet, consectetur adipiscing elit. In rutrum mauris a ante tempor vulputate. Suspendisse malesuada velit vitae neque interdum, vitae euismod metus convallis. Fusce id feugiat nisl. Duis eu tempus purus. Donec quis ultricies orci, a dignissim enim. Nunc placerat ex sit amet felis molestie, vitae rhoncus urna dapibus. Donec ac lacus auctor, posuere lacus eget, commodo justo. In tempor aliquet sagittis. Curabitur eget enim massa. Etiam eleifend eget arcu ut sodales. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Phasellus sit amet congue ligula. Etiam quis turpis commodo, molestie diam in, interdum urna.\n" +
                "\n" +
                "Vestibulum nec blandit orci. Quisque non metus quis nibh porta venenatis. Etiam vel fermentum sem, eget vestibulum metus. Maecenas at viverra magna, sed convallis nibh. Proin quis enim sollicitudin, dapibus arcu nec, tincidunt turpis. Donec bibendum varius nisl, et pharetra tellus blandit et. Aenean commodo tellus tortor, id scelerisque lacus pharetra vel. Etiam malesuada ex vel iaculis elementum. Integer lobortis quam metus, id pharetra ex volutpat vitae.\n" +
                "\n" +
                "Phasellus ac nulla at metus placerat cursus. Cras eget magna a felis consequat suscipit. Mauris tristique lorem vel mauris sagittis dictum. Fusce sit amet dolor orci. Donec ultrices imperdiet lorem. Morbi vel hendrerit tortor. Pellentesque ut mauris tellus. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nunc vestibulum, velit condimentum commodo suscipit, dolor enim rutrum nibh, vitae commodo dui lorem non arcu. Nulla pellentesque leo ante, et tincidunt velit luctus vel. Nullam ullamcorper finibus condimentum. Quisque hendrerit vehicula quam id pulvinar. Suspendisse urna diam, facilisis et cursus vel, tempus vel nulla. Maecenas sed lacus dapibus, vehicula massa sed, ullamcorper nulla.\n" +
                "\n" +
                "Vestibulum id tincidunt eros. Ut imperdiet diam et mollis gravida. Curabitur ullamcorper odio dolor, vel bibendum neque egestas quis. Aenean quis arcu nibh. Suspendisse mi lectus, tristique vitae cursus eu, sollicitudin id erat. Fusce id varius sem. Maecenas molestie metus cursus, mattis magna vel, venenatis nibh. Sed mollis, ex in sollicitudin tempor, felis neque venenatis libero, et accumsan odio nisi ac velit. Cras ullamcorper arcu non mattis laoreet. Ut ut elementum mauris. In vulputate ac eros sed venenatis.\n" +
                "\n" +
                "Maecenas et sagittis justo, a tempor nulla. Morbi et sapien congue velit euismod fringilla eu vitae nisl. Nulla scelerisque ac lacus non hendrerit. Proin sodales tempor libero dapibus porttitor. Nam eu interdum odio, vitae eleifend libero. Curabitur vel facilisis sem. Nullam sed nunc at augue auctor convallis sit amet vel dolor. Donec congue risus at nisi interdum, et porta odio iaculis. Suspendisse pellentesque aliquet urna, vel rutrum purus pharetra et. Suspendisse porta lectus tellus, vitae iaculis leo hendrerit dictum. Sed vel arcu ut arcu tincidunt ultrices eu quis erat. Pellentesque luctus, odio a faucibus ultrices, libero metus tincidunt est, quis pellentesque est erat dignissim nulla. Nam non lacinia lorem, at porta nibh. Curabitur aliquam nibh nec nisl feugiat rutrum."
            font.pixelSize: 16
            font.family: Constants.regularFontFamily
            font.weight: Font.Normal
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            color: ColorPalette.neutral100
            wrapMode: Text.WordWrap
        }
    }

    // ColumnLayout {
    //     id: content
    //
    // }



    // GridView {
    //     id: gridView
    //     objectName: "GridView"
    //     clip: true
    //     width: Math.min(Math.floor(parent.width / cellContentWidth), 7) * cellContentWidth
    //     anchors.horizontalCenter: parent.horizontalCenter
    //     height: parent.height
    //
    //     preferredHighlightBegin: 0.33 * gridView.height
    //     preferredHighlightEnd: 0.66 * gridView.height
    //     highlightRangeMode: GridView.ApplyRange
    //
    //     populate: Transition {
    //         id: popTransition
    //         SequentialAnimation {
    //             PauseAnimation {
    //                 duration: popTransition.ViewTransition.index * 50
    //             }
    //             ParallelAnimation {
    //                 PropertyAnimation {
    //                     property: "opacity"
    //                     from: 0
    //                     to: 1
    //                     duration: 300
    //                     easing.type: Easing.InOutQuad
    //                 }
    //                 PropertyAnimation {
    //                     property: "y"
    //                     from: popTransition.ViewTransition.destination.y + 20
    //                     to: popTransition.ViewTransition.destination.y
    //                     duration: 300
    //                     easing.type: Easing.InOutQuad
    //                 }
    //             }
    //         }
    //     }
    //
    //     contentY: 0
    //
    //     header: Pane {
    //         id: header
    //
    //         width: parent.width
    //         height: 120
    //         background: Rectangle {
    //             color: "transparent"
    //             // border.color: "red"
    //         }
    //
    //         horizontalPadding: 8
    //
    //         Text {
    //             anchors.fill: parent
    //             text: "All Mods"
    //             color: "white"
    //             font.pointSize: 26
    //             font.family: Constants.regularFontFamily
    //             font.weight: Font.DemiBold
    //             horizontalAlignment: Text.AlignLeft
    //             verticalAlignment: Text.AlignVCenter
    //         }
    //
    //     }
    //
    //     readonly property int cellSpacing: 12
    //     readonly property int cellContentWidth: 360 + cellSpacing
    //     readonly property int cellContentHeight: 240 + cellSpacing
    //
    //     cellWidth: cellContentWidth
    //     cellHeight: cellContentHeight
    //     focus: true
    //
    //     currentIndex: 0
    //     boundsBehavior: Flickable.StopAtBounds
    //
    //     delegate: ShopGridItemDelegate {
    //         id: delegate
    //         opacity: 0
    //         cellWidth: gridView.cellWidth
    //         cellHeight: gridView.cellHeight
    //         cellSpacing: gridView.cellSpacing
    //     }
    // }
}

