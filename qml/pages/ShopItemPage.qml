pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

Flickable {
    id: page
    // property alias model: gridView.model

    contentHeight: contentColumn.height + contentPane.verticalPadding * 2
    // Item {
    //     id: sososo
    //     anchors.top: parent.top
    //     anchors.left: parent.left
    //     anchors.right: parent.right
    //     height: 480
    //     clip: true
    //
    //     Image {
    //         id: sourceImage
    //         anchors.horizontalCenter: parent.horizontalCenter
    //         anchors.top: parent.top
    //         fillMode: Image.PreserveAspectCrop
    //         width: parent.width
    //         height: Math.max(width / 3, 600)
    //         // visible: false
    //
    //         source: "file:system/_img/ampedup_hero.jpg"
    //     }
    //
    //     // MultiEffect {
    //     //     source: sourceImage
    //     //     anchors.fill: sourceImage
    //     //     blurEnabled: true
    //     //     blurMax: 64
    //     //     blur: 1.0
    //     // }
    // }

    // Rectangle {
    //     id: sososo
    //     anchors.top: parent.top
    //     anchors.left: parent.left
    //     anchors.right: parent.right
    //     height: 400
    //     color: "#930000"
    // }

    Pane {
        id: contentPane
        horizontalPadding: 20
        width: parent.width
        background: Item {}
        ColumnLayout {
            id: contentColumn
            width: Math.min(parent.width, 1600)
            // width: parent.width
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 8

            Text {
                Layout.topMargin: 48
                text: "Mario Kart 64: Amped Up"
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
                Layout.bottomMargin: 12
            }
            RowLayout {
                Layout.fillWidth: true
                ColumnLayout {
                    // Layout.fillWidth: true
                    Layout.fillHeight: true
                    ImageViewer {
                        id: thing
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }
                    Text {
                        Layout.fillWidth: true
                        Layout.topMargin: 12
                        Layout.bottomMargin: 12
                        topPadding: 12
                        bottomPadding: 12
                        text: "Something about a tagline"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.family: Constants.regularFontFamily
                        font.pixelSize: 18
                        color: ColorPalette.neutral100
                    }
                    Pane {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 60
                        background: Rectangle {
                            color: ColorPalette.neutral800
                            radius: 12
                        }
                        contentItem: Text {
                            text: "Tags!"
                            font.pixelSize: 16
                            font.family: Constants.regularFontFamily
                            font.weight: Font.Bold
                            color: ColorPalette.neutral200
                        }
                    }
                    Text {
                        text: "This is a mod for the game Mario Kart 64.\n" +
                            "\n" +
                            "It adds 16 brand new courses, as well as additional game modes, music, modern visuals, gameplay techniques, and much more!\n" +
                            "\n" +
                            "\n" +
                            "*Available game modes*\n" +
                            "\n" +
                            "For Grand Prix:\n" +
                            "- Default\n" +
                            "- N64 Coin Mode\n" +
                            "- Elimination Mode\n" +
                            "- Balloon Race\n" +
                            "\n" +
                            "For Versus:\n" +
                            "- Default\n" +
                            "- Elimination Mode\n" +
                            "- Balloon Race\n" +
                            "\n" +
                            "For Time Trial:\n" +
                            "- Default\n" +
                            "- N64 Coin Mode\n" +
                            "- Target Zones\n" +
                            "- Mini Turbos"
                        font.pixelSize: 16
                        font.family: Constants.regularFontFamily
                        font.weight: Font.Normal
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                        color: ColorPalette.neutral100
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }
                }
                ColumnLayout {
                    Layout.preferredWidth: 500
                    Layout.leftMargin: 24
                    Layout.fillHeight: true
                    Image {
                        source: "file:system/_img/mklogoclear.png"
                        fillMode: Image.PreserveAspectFit
                        Layout.fillWidth: true
                    }
                    Pane {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 60
                        background: Rectangle {
                            color: ColorPalette.neutral800
                            radius: 12
                        }
                        contentItem: Text {
                            text: "Requires Mario Kart 64"
                            font.pixelSize: 16
                            font.family: Constants.regularFontFamily
                            font.weight: Font.Bold
                            color: ColorPalette.neutral200
                        }
                    }
                    Text {
                        text: "Version: 2.9A (latest)"
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
                    Button {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 48
                        hoverEnabled: true
                        background: Rectangle {
                            color: parent.hovered ? ColorPalette.neutral800 : "transparent"
                            radius: 12
                        }
                        contentItem: Text {
                            text: "Download older versions"
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignHCenter
                            font.pixelSize: 14
                            font.family: Constants.regularFontFamily
                            font.weight: Font.DemiBold
                            color: parent.hovered ? ColorPalette.neutral300 : ColorPalette.neutral400
                        }

                        onClicked: {
                            console.log("Install button clicked")
                        }
                    }
                    Item {
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                    }
                }

            }
            // Item {
            //     Layout.fillWidth: true
            //     Layout.preferredHeight: 500
            //     ColumnLayout {
            //         id: stuff
            //         anchors.right: parent.right
            //         anchors.top: parent.top
            //         anchors.bottom: parent.bottom
            //         width: 300
            //     }
            // }



            // Pane {
            //     id: main
            //     padding: 20
            //     background: Item {
            //         Rectangle {
            //             id: sourceItem
            //             color: ColorPalette.neutral900
            //             anchors.fill: parent
            //             // border.color: "black"
            //             // border.width: 3
            //             radius: width >= 1240 ? 24 : 0
            //         }
            //         MultiEffect {
            //             source: sourceItem
            //             anchors.fill: sourceItem
            //             shadowEnabled: true
            //         }
            //     }
            //
            //     Layout.fillWidth: true
            //     Layout.topMargin: width >= 1240 ? 36 : 0
            //
            //     Behavior on Layout.topMargin {
            //         NumberAnimation {
            //             duration: 200
            //             easing.type: Easing.InOutQuad
            //         }
            //     }
            //     Layout.preferredHeight: 520
            //
            //     contentItem: Item {
            //
            //
            //         ColumnLayout {
            //             spacing: 16
            //             anchors.leftMargin: 12
            //             anchors.right: parent.right
            //             anchors.top: parent.top
            //             anchors.bottom: parent.bottom
            //             anchors.left: thing.right
            //
            //
            //             Item {
            //                 Layout.fillHeight: true
            //                 Layout.fillWidth: true
            //             }
            //             Text {
            //                 text: "Version: 2.9A (latest)"
            //                 font.pixelSize: 15
            //                 font.family: Constants.regularFontFamily
            //                 font.weight: Font.Normal
            //                 horizontalAlignment: Text.AlignHCenter
            //                 Layout.fillWidth: true
            //                 wrapMode: Text.WordWrap
            //                 color: ColorPalette.neutral300
            //             }
            //
            //             Button {
            //                 Layout.fillWidth: true
            //                 Layout.preferredHeight: 48
            //                 background: Rectangle {
            //                     color: "#d14c20"
            //                     radius: 12
            //                 }
            //                 contentItem: Text {
            //                     text: "Download"
            //                     verticalAlignment: Text.AlignVCenter
            //                     horizontalAlignment: Text.AlignHCenter
            //                     font.pixelSize: 18
            //                     font.family: Constants.regularFontFamily
            //                     font.weight: Font.Bold
            //                     color: "white"
            //                 }
            //
            //                 onClicked: {
            //                     console.log("Install button clicked")
            //                 }
            //             }
            //         }
            //     }
            //     // anchors.horizontalCenter: parent.horizontalCenter
            // }
            //
            // Pane {
            //     padding: 20
            //     background: Rectangle {
            //         color: "transparent"
            //         // border.color: "black"
            //         // border.width: 3
            //         radius: 24
            //     }
            //     Layout.fillWidth: true
            //
            //     contentItem: Text {
            //         text: "This is a mod for the game Mario Kart 64.\n" +
            //             "\n" +
            //             "It adds 16 brand new courses, as well as additional game modes, music, modern visuals, gameplay techniques, and much more!\n" +
            //             "\n" +
            //             "\n" +
            //             "*Available game modes*\n" +
            //             "\n" +
            //             "For Grand Prix:\n" +
            //             "- Default\n" +
            //             "- N64 Coin Mode\n" +
            //             "- Elimination Mode\n" +
            //             "- Balloon Race\n" +
            //             "\n" +
            //             "For Versus:\n" +
            //             "- Default\n" +
            //             "- Elimination Mode\n" +
            //             "- Balloon Race\n" +
            //             "\n" +
            //             "For Time Trial:\n" +
            //             "- Default\n" +
            //             "- N64 Coin Mode\n" +
            //             "- Target Zones\n" +
            //             "- Mini Turbos"
            //         font.pixelSize: 16
            //         font.family: Constants.regularFontFamily
            //         font.weight: Font.Normal
            //         horizontalAlignment: Text.AlignLeft
            //         verticalAlignment: Text.AlignVCenter
            //         color: ColorPalette.neutral100
            //         wrapMode: Text.WordWrap
            //     }
            // }
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

