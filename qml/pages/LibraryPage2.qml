import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

FocusScope {
    id: page

    property alias model: listView.model

    // property var model

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

        populate: Transition {
            id: popTransition
            SequentialAnimation {
                PauseAnimation {
                    duration: popTransition.ViewTransition.index * 10
                }
                ParallelAnimation {
                    PropertyAnimation {
                        property: "opacity"
                        from: 0
                        to: 1
                        duration: 100
                        easing.type: Easing.InOutQuad
                    }
                    PropertyAnimation {
                        property: "y"
                        from: popTransition.ViewTransition.destination.y + 20
                        to: popTransition.ViewTransition.destination.y
                        duration: 100
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
                text: "All Games"
                color: "white"
                font.pointSize: 26
                font.family: Constants.regularFontFamily
                font.weight: Font.DemiBold
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
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
            opacity: 1
            cellSpacing: listView.cellSpacing
            cellWidth: listView.cellContentWidth
            cellHeight: listView.cellContentHeight

            Component.onCompleted: {
                openDetails.connect(page.openDetails)
                startGame.connect(page.startGame)
            }
        }
    }

    // Flickable {
    //     width: parent.width
    //     height: parent.height
    //     contentHeight: col.height
    //
    //     ColumnLayout {
    //         id: col
    //         width: parent.width
    //         spacing: 8
    //         RowLayout {
    //             id: row0
    //             spacing: 8
    //
    //             property int desiredHeight: 100
    //
    //             function recalc() {
    //                 let numItems = 0
    //                 let diff = parent.width - width
    //                 for (let i = 0; i < children.length; i++) {
    //                     if (children[i].thing) {
    //                         numItems += 1
    //                         // let aspect = children[i].width / children[i].height
    //                         let aspect = children[i].aspect
    //                         if (aspect > 0) {
    //                             console.log("aspect: ", aspect)
    //                         }
    //                     }
    //                 }
    //
    //                 console.log("Each item gets: ", diff / numItems)
    //
    //                 console.log("Remaining width: ", diff)
    //
    //                 let myWidth = width
    //                 let myHeight = height
    //                 if (myWidth > parent.width) {
    //                     while (myWidth > parent.width) {
    //                         myHeight -= 0.5
    //                         myWidth = 0
    //                         for (let i = 0; i < children.length; i++) {
    //                             if (children[i].thing) {
    //                                 myWidth += (myHeight * children[i].aspect)
    //                             }
    //                         }
    //                         myWidth += spacing * (numItems - 1)
    //                     }
    //                 } else if (myWidth < parent.width) {
    //                     while (myWidth < parent.width) {
    //                         myHeight += 0.5
    //                         myWidth = 0
    //                         for (let i = 0; i < children.length; i++) {
    //                             if (children[i].thing) {
    //                                 myWidth += (myHeight * children[i].aspect)
    //                             }
    //                         }
    //                         myWidth += spacing * (numItems - 1)
    //                     }
    //                 }
    //
    //                 Layout.preferredHeight = myHeight
    //
    //                 console.log("myWidth: ", myWidth)
    //                 console.log("myHeight: ", myHeight)
    //             }
    //
    //             Layout.fillWidth: true
    //             Layout.preferredHeight: 100
    //             Repeater {
    //                 model: 7
    //                 delegate: Rectangle {
    //                     property bool thing: true
    //                     property real aspect: {
    //                         let a = Math.random()
    //                         if (a < 0.2) {
    //                             return 1 // gba
    //                         } else if (a < 0.4) {
    //                             return 1.4 //snes, n64
    //                         } else if (a < 0.6) {
    //                             return 0.7 // genesis, nes
    //                         } else if (a < 0.8) {
    //                             return 1.11 // ds
    //                         } else if (a <= 1.0) {
    //                             return 0.58 // psp
    //                         }
    //                     }
    //                     Layout.preferredWidth: height * aspect
    //                     Layout.fillHeight: true
    //                     color: Qt.rgba(Math.random(), Math.random(), Math.random(), 1)
    //
    //                 }
    //
    //                 Component.onCompleted: function () {
    //                     row0.recalc()
    //                 }
    //             }
    //         }
    //         RowLayout {
    //             id: row1
    //             spacing: 8
    //
    //             property int desiredHeight: 100
    //
    //             function recalc() {
    //                 let numItems = 0
    //                 let diff = parent.width - width
    //                 for (let i = 0; i < children.length; i++) {
    //                     if (children[i].thing) {
    //                         numItems += 1
    //                         // let aspect = children[i].width / children[i].height
    //                         let aspect = children[i].aspect
    //                         if (aspect > 0) {
    //                             console.log("aspect: ", aspect)
    //                         }
    //                     }
    //                 }
    //
    //                 console.log("Each item gets: ", diff / numItems)
    //
    //                 console.log("Remaining width: ", diff)
    //
    //                 let myWidth = width
    //                 let myHeight = height
    //                 if (myWidth > parent.width) {
    //                     while (myWidth > parent.width) {
    //                         myHeight -= 0.5
    //                         myWidth = 0
    //                         for (let i = 0; i < children.length; i++) {
    //                             if (children[i].thing) {
    //                                 myWidth += (myHeight * children[i].aspect)
    //                             }
    //                         }
    //                         myWidth += spacing * (numItems - 1)
    //                     }
    //                 } else if (myWidth < parent.width) {
    //                     while (myWidth < parent.width) {
    //                         myHeight += 0.5
    //                         myWidth = 0
    //                         for (let i = 0; i < children.length; i++) {
    //                             if (children[i].thing) {
    //                                 myWidth += (myHeight * children[i].aspect)
    //                             }
    //                         }
    //                         myWidth += spacing * (numItems - 1)
    //                     }
    //                 }
    //
    //                 Layout.preferredHeight = myHeight
    //
    //                 console.log("myWidth: ", myWidth)
    //                 console.log("myHeight: ", myHeight)
    //             }
    //
    //             Layout.fillWidth: true
    //             Layout.preferredHeight: 100
    //             Repeater {
    //                 model: 3
    //                 delegate: Rectangle {
    //                     property bool thing: true
    //                     property real aspect: {
    //                         let a = Math.random()
    //                         if (a < 0.2) {
    //                             return 1 // gba
    //                         } else if (a < 0.4) {
    //                             return 1.4 //snes, n64
    //                         } else if (a < 0.6) {
    //                             return 0.7 // genesis, nes
    //                         } else if (a < 0.8) {
    //                             return 1.11 // ds
    //                         } else if (a <= 1.0) {
    //                             return 0.58 // psp
    //                         }
    //                     }
    //                     Layout.preferredWidth: height * aspect
    //                     Layout.fillHeight: true
    //                     color: Qt.rgba(Math.random(), Math.random(), Math.random(), 1)
    //
    //                 }
    //
    //                 Component.onCompleted: function () {
    //                     row1.recalc()
    //                 }
    //             }
    //         }
    //         RowLayout {
    //             id: row2
    //             spacing: 8
    //
    //             property int desiredHeight: 100
    //
    //             function recalc() {
    //                 let numItems = 0
    //                 let diff = parent.width - width
    //                 for (let i = 0; i < children.length; i++) {
    //                     if (children[i].thing) {
    //                         numItems += 1
    //                         // let aspect = children[i].width / children[i].height
    //                         let aspect = children[i].aspect
    //                         if (aspect > 0) {
    //                             console.log("aspect: ", aspect)
    //                         }
    //                     }
    //                 }
    //
    //                 console.log("Each item gets: ", diff / numItems)
    //
    //                 console.log("Remaining width: ", diff)
    //
    //                 let myWidth = width
    //                 let myHeight = height
    //                 if (myWidth > parent.width) {
    //                     while (myWidth > parent.width) {
    //                         myHeight -= 0.5
    //                         myWidth = 0
    //                         for (let i = 0; i < children.length; i++) {
    //                             if (children[i].thing) {
    //                                 myWidth += (myHeight * children[i].aspect)
    //                             }
    //                         }
    //                         myWidth += spacing * (numItems - 1)
    //                     }
    //                 } else if (myWidth < parent.width) {
    //                     while (myWidth < parent.width) {
    //                         myHeight += 0.5
    //                         myWidth = 0
    //                         for (let i = 0; i < children.length; i++) {
    //                             if (children[i].thing) {
    //                                 myWidth += (myHeight * children[i].aspect)
    //                             }
    //                         }
    //                         myWidth += spacing * (numItems - 1)
    //                     }
    //                 }
    //
    //                 Layout.preferredHeight = myHeight
    //
    //                 console.log("myWidth: ", myWidth)
    //                 console.log("myHeight: ", myHeight)
    //             }
    //
    //             Layout.fillWidth: true
    //             Layout.preferredHeight: 100
    //             Repeater {
    //                 model: 5
    //                 delegate: Rectangle {
    //                     property bool thing: true
    //                     property real aspect: {
    //                         let a = Math.random()
    //                         if (a < 0.2) {
    //                             return 1 // gba
    //                         } else if (a < 0.4) {
    //                             return 1.4 //snes, n64
    //                         } else if (a < 0.6) {
    //                             return 0.7 // genesis, nes
    //                         } else if (a < 0.8) {
    //                             return 1.11 // ds
    //                         } else if (a <= 1.0) {
    //                             return 0.58 // psp
    //                         }
    //                     }
    //                     Layout.preferredWidth: height * aspect
    //                     Layout.fillHeight: true
    //                     color: Qt.rgba(Math.random(), Math.random(), Math.random(), 1)
    //
    //                 }
    //
    //                 Component.onCompleted: function () {
    //                     row2.recalc()
    //                 }
    //             }
    //         }
    //         RowLayout {
    //             id: row3
    //             spacing: 8
    //
    //             property int desiredHeight: 100
    //
    //             function recalc() {
    //                 let numItems = 0
    //                 let diff = parent.width - width
    //                 for (let i = 0; i < children.length; i++) {
    //                     if (children[i].thing) {
    //                         numItems += 1
    //                         // let aspect = children[i].width / children[i].height
    //                         let aspect = children[i].aspect
    //                         if (aspect > 0) {
    //                             console.log("aspect: ", aspect)
    //                         }
    //                     }
    //                 }
    //
    //                 console.log("Each item gets: ", diff / numItems)
    //
    //                 console.log("Remaining width: ", diff)
    //
    //                 let myWidth = width
    //                 let myHeight = height
    //                 if (myWidth > parent.width) {
    //                     while (myWidth > parent.width) {
    //                         myHeight -= 0.5
    //                         myWidth = 0
    //                         for (let i = 0; i < children.length; i++) {
    //                             if (children[i].thing) {
    //                                 myWidth += (myHeight * children[i].aspect)
    //                             }
    //                         }
    //                         myWidth += spacing * (numItems - 1)
    //                     }
    //                 } else if (myWidth < parent.width) {
    //                     while (myWidth < parent.width) {
    //                         myHeight += 0.5
    //                         myWidth = 0
    //                         for (let i = 0; i < children.length; i++) {
    //                             if (children[i].thing) {
    //                                 myWidth += (myHeight * children[i].aspect)
    //                             }
    //                         }
    //                         myWidth += spacing * (numItems - 1)
    //                     }
    //                 }
    //
    //                 Layout.preferredHeight = myHeight
    //
    //                 console.log("myWidth: ", myWidth)
    //                 console.log("myHeight: ", myHeight)
    //             }
    //
    //             Layout.fillWidth: true
    //             Layout.preferredHeight: 100
    //             Repeater {
    //                 model: 6
    //                 delegate: Rectangle {
    //                     property bool thing: true
    //                     property real aspect: {
    //                         let a = Math.random()
    //                         if (a < 0.2) {
    //                             return 1 // gba
    //                         } else if (a < 0.4) {
    //                             return 1.4 //snes, n64
    //                         } else if (a < 0.6) {
    //                             return 0.7 // genesis, nes
    //                         } else if (a < 0.8) {
    //                             return 1.11 // ds
    //                         } else if (a <= 1.0) {
    //                             return 0.58 // psp
    //                         }
    //                     }
    //                     Layout.preferredWidth: height * aspect
    //                     Layout.fillHeight: true
    //                     color: Qt.rgba(Math.random(), Math.random(), Math.random(), 1)
    //
    //                 }
    //
    //                 Component.onCompleted: function () {
    //                     row3.recalc()
    //                 }
    //             }
    //         }
    //
    //
    //         // RowLayout {
    //         //     id: row1
    //         //
    //         //     function recalc() {
    //         //         let diff = parent.width - width
    //         //         for (let i = 0; i < children.length; i++) {
    //         //             let aspect = children[i].width / children[i].height
    //         //             if (aspect > 0) {
    //         //                 console.log("aspect: ", aspect)
    //         //             }
    //         //         }
    //         //     }
    //         //
    //         //     Layout.fillWidth: true
    //         //     Layout.preferredHeight: 120
    //         //     // onWidthChanged: function () {
    //         //     //     console.log("width changed: ", width)
    //         //     //     console.log("parent width: ", parent.width)
    //         //     // }
    //         //     Repeater {
    //         //         model: 8
    //         //         delegate: Rectangle {
    //         //             property bool thing: true
    //         //             Layout.preferredWidth: 30 + Math.random() * 100
    //         //             Layout.fillHeight: true
    //         //             color: Qt.rgba(Math.random(), Math.random(), Math.random(), 1)
    //         //
    //         //         }
    //         //
    //         //         Component.onCompleted: function () {
    //         //             row1.recalc()
    //         //         }
    //         //     }
    //         //
    //         //     Item {
    //         //         Layout.fillHeight: true
    //         //         Layout.fillWidth: true
    //         //     }
    //         // }
    //         // RowLayout {
    //         //     id: row2
    //         //
    //         //     function recalc() {
    //         //         let diff = parent.width - width
    //         //         for (let i = 0; i < children.length; i++) {
    //         //             let aspect = children[i].width / children[i].height
    //         //             if (aspect > 0) {
    //         //                 console.log("aspect: ", aspect)
    //         //             }
    //         //         }
    //         //     }
    //         //
    //         //     Layout.fillWidth: true
    //         //     Layout.preferredHeight: 120
    //         //     Repeater {
    //         //         model: 12
    //         //         delegate: Rectangle {
    //         //             Layout.preferredWidth: 30 + Math.random() * 100
    //         //             Layout.fillHeight: true
    //         //             color: Qt.rgba(Math.random(), Math.random(), Math.random(), 1)
    //         //
    //         //         }
    //         //
    //         //         Component.onCompleted: function () {
    //         //             row2.recalc()
    //         //         }
    //         //     }
    //         //
    //         //     Item {
    //         //         Layout.fillHeight: true
    //         //         Layout.fillWidth: true
    //         //     }
    //         // }
    //
    //         // GridLayout {
    //         //     // anchors.fill: parent
    //         //     // columns: 6
    //         //     width: parent.width
    //         //     height: parent.height
    //         //     Repeater {
    //         //         model: 100
    //         //         delegate: Rectangle {
    //         //             Layout.preferredWidth: 30 + Math.random() * 100
    //         //             // Layout.fillWidth: true
    //         //             Layout.preferredHeight: 30 + Math.random() * 100
    //         //             color: Qt.rgba(Math.random(), Math.random(), Math.random(), 1)
    //         //         }
    //         //     }
    //         // }
    //
    //
    //     }
    // }
}

