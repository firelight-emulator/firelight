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
                text: "Super Mario 64: Beyond the Cursed Mirror"
                font.pixelSize: 36
                font.family: Constants.regularFontFamily
                font.weight: Font.Black
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
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    ImageViewer {
                        id: thing
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }
                    // Text {
                    //     Layout.fillWidth: true
                    //     topPadding: 24
                    //     bottomPadding: 24
                    //     text: "16 brand new courses, additional game modes, new music and gameplay techniques, modern visuals, and much more!"
                    //     font.family: Constants.regularFontFamily
                    //     font.pixelSize: 16
                    //     lineHeight: 1.2
                    //     font.weight: Font.DemiBold
                    //     color: "white"
                    //     wrapMode: Text.WordWrap
                    // }
                    Text {
                        Layout.fillWidth: true
                        topPadding: 24
                        bottomPadding: 24
                        text: "Greetings Superstar, did you know that your nemesis hid in the decrepit castle all this time? It appears he fosters his strength in a hidden compartment behind one cursed mirror. I implore you to defeat him, once and for all – for the sake of the kingdom. -Yours truly, The Showrunner"
                        font.family: Constants.regularFontFamily
                        font.pixelSize: 16
                        lineHeight: 1.2
                        font.weight: Font.DemiBold
                        color: "white"
                        wrapMode: Text.WordWrap
                    }
                    // Flow {
                    //     Layout.fillWidth: true
                    //     spacing: 4
                    //     Repeater {
                    //         model: ListModel {
                    //             ListElement { name: "Single Player" }
                    //             ListElement { name: "Multiplayer" }
                    //             ListElement { name: "Time Trials" }
                    //             ListElement { name: "Grand Prix" }
                    //             ListElement { name: "Versus" }
                    //             ListElement { name: "Single Player" }
                    //             ListElement { name: "Multiplayer" }
                    //             ListElement { name: "Time Trials" }
                    //             ListElement { name: "Grand Prix" }
                    //             ListElement { name: "Versus" }
                    //             ListElement { name: "Single Player" }
                    //             ListElement { name: "Multiplayer" }
                    //         }
                    //
                    //         delegate: TextArea {
                    //             readOnly: true
                    //             required property var model
                    //             background: Rectangle {
                    //                 color: ColorPalette.neutral800
                    //                 radius: height / 2
                    //             }
                    //             topPadding: 4
                    //             bottomPadding: 4
                    //             leftPadding: 8
                    //             rightPadding: 8
                    //             text: model.name
                    //             font.pixelSize: 14
                    //             font.family: Constants.regularFontFamily
                    //             font.weight: Font.Normal
                    //             Layout.fillWidth: true
                    //             wrapMode: Text.WordWrap
                    //             color: ColorPalette.neutral300
                    //         }
                    //     }
                    // }
                    // Pane {
                    //     Layout.fillWidth: true
                    //     Layout.preferredHeight: 60
                    //     background: Rectangle {
                    //         color: ColorPalette.neutral800
                    //         radius: 12
                    //     }
                    //     contentItem: Text {
                    //         text: "Tags!"
                    //         font.pixelSize: 16
                    //         font.family: Constants.regularFontFamily
                    //         font.weight: Font.Bold
                    //         color: ColorPalette.neutral200
                    //     }
                    // }
                    // Text {
                    //     Layout.topMargin: 16
                    //     leftPadding: 12
                    //     text: "Description"
                    //     font.pixelSize: 15
                    //     font.family: Constants.regularFontFamily
                    //     font.weight: Font.Normal
                    //     Layout.fillWidth: true
                    //     wrapMode: Text.WordWrap
                    //     color: ColorPalette.neutral400
                    // }
                    Text {
                        text: "SM64: Beyond the Cursed Mirror is a major Super Mario 64 ROM hack created by Rovert, which contains 15 custom-made courses and 120 stars waiting to be collected. With a unique in-depth story, original characters, and new mechanics, this SM64 ROM hack will provide a traditional yet unique experience."
                        textFormat: Text.RichText
                        font.pixelSize: 16
                        font.family: Constants.regularFontFamily
                        font.weight: Font.DemiBold
                        horizontalAlignment: Text.AlignLeft
                        lineHeight: 1.2
                        verticalAlignment: Text.AlignVCenter
                        color: ColorPalette.neutral400
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }
                    // Text {
                    //     text: "<h4>Grand Prix Modes:</h4>" +
                    //         "<ul><li>Default</li>" +
                    //         "<li>N64 Coin Mode</li>" +
                    //         "<li>Elimination Mode</li>" +
                    //         "<li>Balloon Race</li></ul>" +
                    //
                    //         "<h4>Versus Modes:</h4>" +
                    //         "<ul><li>Default</li>" +
                    //         "<li>Elimination Mode</li>" +
                    //         "<li>Balloon Race</li></ul>" +
                    //
                    //         "<h4>Time Trial Modes:</h4>" +
                    //         "<ul><li>Default</li>" +
                    //         "<li>N64 Coin Mode</li>" +
                    //         "<li>Target Zones</li>" +
                    //         "<li>Mini Turbos</li></ul>"
                    //     textFormat: Text.RichText
                    //     font.pixelSize: 16
                    //     font.family: Constants.regularFontFamily
                    //     font.weight: Font.Normal
                    //     horizontalAlignment: Text.AlignLeft
                    //     verticalAlignment: Text.AlignVCenter
                    //     color: ColorPalette.neutral400
                    //     wrapMode: Text.WordWrap
                    //     Layout.fillWidth: true
                    // }
                }
                ColumnLayout {
                    Layout.preferredWidth: 350
                    Layout.maximumWidth: 350
                    Layout.minimumWidth: 350
                    Layout.leftMargin: 24
                    Layout.fillHeight: true
                    Image {
                        source: "file:system/_img/cursedmirrorclear.png"
                        fillMode: Image.PreserveAspectFit
                        Layout.fillWidth: true
                    }
                    // Pane {
                    //     Layout.fillWidth: true
                    //     Layout.preferredHeight: 60
                    //     background: Rectangle {
                    //         color: ColorPalette.neutral800
                    //         radius: 12
                    //     }
                    //     contentItem: Text {
                    //         text: "Requires Mario Kart 64"
                    //         font.pixelSize: 16
                    //         font.family: Constants.regularFontFamily
                    //         font.weight: Font.Bold
                    //         color: ColorPalette.neutral200
                    //     }
                    // }
                    // Text {
                    //     text: "Version: 2.9A (latest)"
                    //     font.pixelSize: 15
                    //     font.family: Constants.regularFontFamily
                    //     font.weight: Font.Normal
                    //     horizontalAlignment: Text.AlignHCenter
                    //     Layout.fillWidth: true
                    //     wrapMode: Text.WordWrap
                    //     color: ColorPalette.neutral300
                    // }

                    Button {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 48
                        background: Rectangle {
                            color: "#d14c20"
                            radius: 12
                        }
                        contentItem: Text {
                            text: "Add to Library"
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
                    Text {
                        Layout.topMargin: 16
                        leftPadding: 12
                        text: "Author"
                        font.pixelSize: 15
                        font.family: Constants.regularFontFamily
                        font.weight: Font.DemiBold
                        Layout.fillWidth: true
                        wrapMode: Text.WordWrap
                        color: ColorPalette.neutral400
                    }
                    Pane {
                        Layout.fillWidth: true
                        background: Rectangle {
                            color: ColorPalette.neutral800
                            radius: 12
                        }
                        contentItem: Text {
                            text: "Rovertronic"
                            font.pixelSize: 16
                            font.family: Constants.regularFontFamily
                            // font.weight: Font.DemiBold
                            wrapMode: Text.WordWrap
                            color: ColorPalette.neutral100
                        }
                    }
                    // Text {
                    //     Layout.topMargin: 8
                    //     text: "Credits"
                    //     font.pixelSize: 15
                    //     font.family: Constants.regularFontFamily
                    //     font.weight: Font.Normal
                    //     Layout.fillWidth: true
                    //     wrapMode: Text.WordWrap
                    //     color: ColorPalette.neutral300
                    // }
                    // Pane {
                    //     Layout.fillWidth: true
                    //     Layout.preferredHeight: 240
                    //     background: Rectangle {
                    //         color: ColorPalette.neutral800
                    //         radius: 12
                    //     }
                    //     contentItem: Text {
                    //         text: "Blah"
                    //         font.pixelSize: 16
                    //         font.family: Constants.regularFontFamily
                    //         font.weight: Font.Bold
                    //         color: ColorPalette.neutral200
                    //     }
                    // }


                    Item {
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                    }
                }

            }

            Pane {
                Layout.fillWidth: true
                Layout.preferredHeight: 48
                Layout.topMargin: 24
                background: Item {}
                contentItem: Text {
                    text: "Trademarks, tradenames, and copyrights are property of their respective owners."
                    font.pixelSize: 14
                    font.family: Constants.regularFontFamily
                    font.weight: Font.DemiBold
                    color: ColorPalette.neutral400
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
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

