import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

FocusScope {
    id: page

    property alias model: listView.model

    // property var model

    signal openDetails(entryId: int)

    signal startGame(entryId: int, hash: string)

    // padding: 24
    // leftPadding: 48
    // rightPadding: 48

    // handle: Rectangle {
    //     id: handleDelegate
    //     implicitWidth: 4
    //     implicitHeight: 4
    //     color: SplitHandle.pressed ? "#81e889"
    //         : (SplitHandle.hovered ? Qt.lighter("#c2f4c6", 1.1) : "#c2f4c6")
    //
    //     containmentMask: Item {
    //         x: (handleDelegate.width - width) / 2
    //         width: 64
    //         height: splitView.height
    //     }
    // }

    Pane {
        id: details

        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: listViewContainer.right

        background: Item {
        }

        contentItem: ColumnLayout {
            spacing: 12
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
            Text {
                Layout.fillWidth: true
                text: listView.currentItem.model.displayName
                font.pixelSize: 24
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                font.weight: Font.DemiBold
                font.family: Constants.regularFontFamily
                color: "white"
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            FirelightButton {
                id: hamburger
                focus: true
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

                Layout.preferredWidth: AppStyle.buttonStandardWidth
                Layout.preferredHeight: AppStyle.buttonStandardHeight

                label: "Play"

                onClicked: function () {
                    page.startGame(listView.currentItem.model.id, listView.currentItem.model.contentHash)
                }
            }

            // Button {
            //     id: playButton
            //     focus: true
            //     Layout.preferredWidth: 200
            //     Layout.preferredHeight: 42
            //     property bool showGlobalCursor: true
            //     Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            //
            //     background: Rectangle {
            //         color: playButton.activeFocus && !InputMethodManager.usingMouse ? ColorPalette.neutral100 : playButton.hovered ? ColorPalette.neutral700 : ColorPalette.neutral800
            //         radius: height / 2
            //     }
            //
            //     contentItem: Text {
            //         text: "Play"
            //         font.family: Constants.regularFontFamily
            //         font.pixelSize: 16
            //         font.weight: Font.DemiBold
            //         // opacity: parent.checked ? 1 : 0.5
            //         horizontalAlignment: Text.AlignHCenter
            //         verticalAlignment: Text.AlignVCenter
            //         color: playButton.activeFocus && !InputMethodManager.usingMouse ? "black" : "white"
            //     }
            //
            //     onClicked: function () {
            //         page.startGame(listView.currentItem.model.id, listView.currentItem.model.contentHash)
            //     }
            // }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }

    Pane {
        id: listViewContainer
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 400
        clip: true
        focus: true
        background: Item {
        }
        KeyNavigation.right: details
        contentItem: ListView {
            id: listView
            objectName: "GridView"
            focus: true
            // anchors.fill: parent
            // width: parent.width * 0.8
            // anchors.horizontalCenter: parent.horizontalCenter
            // height: parent.height
            //

            // SplitView.preferredWidth: parent.width / 2

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AlwaysOn
            }

            preferredHighlightBegin: 0.33 * listView.height
            preferredHighlightEnd: 0.66 * listView.height
            highlightRangeMode: InputMethodManager.usingMouse ? GridView.NoHighlightRange : GridView.ApplyRange

            contentY: 0

            // header: Pane {
            //     id: header
            //
            //     width: parent.width
            //     height: 120
            //     background: Rectangle {
            //         color: "transparent"
            //         // border.color: "red"
            //     }
            //
            //     horizontalPadding: 8
            //
            //     Text {
            //         anchors.fill: parent
            //         text: "All Games"
            //         color: "white"
            //         font.pointSize: 26
            //         font.family: Constants.regularFontFamily
            //         font.weight: Font.DemiBold
            //         horizontalAlignment: Text.AlignLeft
            //         verticalAlignment: Text.AlignVCenter
            //     }
            //
            // }
            highlightMoveDuration: 10
            highlightMoveVelocity: -1

            boundsBehavior: Flickable.StopAtBounds

            // delegate: GameGridItemDelegate {
            //     opacity: 1
            //     cellSpacing: listView.cellSpacing
            //     cellWidth: listView.cellContentWidth
            //     cellHeight: listView.cellContentHeight
            //
            //     Component.onCompleted: {
            //         openDetails.connect(page.openDetails)
            //         startGame.connect(page.startGame)
            //     }
            // }
            delegate: FocusScope {
                id: scope
                required property var index
                required property var model
                width: ListView.view.width
                height: 50
                focus: true
                Button {
                    id: thingg
                    anchors.fill: parent
                    focus: true

                    property var platformTextColor: ColorPalette.neutral400

                    property bool showGlobalCursor: true

                    TapHandler {
                        acceptedButtons: Qt.LeftButton | Qt.RightButton
                        onTapped: function (event, button) {
                            if (button === Qt.RightButton) {
                                rightClickMenu.popup()
                                event.accepted = true
                            } else if (button === Qt.LeftButton) {
                                // Router.navigateTo("library/" + myDelegate.model.id + "/details")
                                // myDelegate.openDetails(myDelegate.model.id)
                                // page.startGame(myDelegate.model.id, myDelegate.model.contentHash)
                                listView.currentIndex = scope.index
                            }
                        }
                    }

                    onClicked: {
                        listView.currentIndex = scope.index
                        if (!InputMethodManager.usingMouse) {
                            details.forceActiveFocus()
                        }
                    }

                    RightClickMenu {
                        id: rightClickMenu
                        objectName: "rightClickMenu"

                        RightClickMenuItem {
                            text: "Play " + scope.model.displayName
                            onTriggered: {
                                page.startGame(scope.model.id, scope.model.contentHash)
                            }
                        }

                        RightClickMenuItem {
                            enabled: false
                            text: "View details"
                            onTriggered: {
                                page.openDetails(scope.model.id)
                            }
                        }

                        MenuSeparator {
                            contentItem: Rectangle {
                                implicitWidth: rightClickMenu.width
                                implicitHeight: 1
                                color: "#606060"
                            }
                        }

                        RightClickMenuItem {
                            enabled: false
                            text: "Manage save data"
                            onTriggered: {
                                page.manageSaveData(scope.model.id)
                            }
                            // onTriggered: {
                            //     addPlaylistRightClickMenu.entryId = libraryEntryRightClickMenu.entryId
                            //     addPlaylistRightClickMenu.popup()
                            // }
                        }
                    }

                    background: Rectangle {
                        color: "white"
                        opacity: parent.pressed ? 0.1 : parent.hovered ? 0.2 : 0
                    }

                    hoverEnabled: true

                    contentItem: RowLayout {
                        spacing: 12
                        // Rectangle {
                        //     Layout.fillHeight: true
                        //     Layout.preferredWidth: 72 - thingg.verticalPadding * 2
                        //     color: "grey"
                        //
                        //     Image {
                        //         source: model.icon1x1SourceUrl
                        //         fillMode: Image.PreserveAspectFit
                        //         anchors.fill: parent
                        //     }
                        // }
                        Text {
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                            // Layout.preferredWidth: 500
                            // Layout.minimumWidth: 300
                            text: model.displayName
                            color: "white"
                            elide: Text.ElideRight
                            maximumLineCount: 1
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignLeft
                            font.pixelSize: 16
                            font.weight: Font.Normal
                            font.family: Constants.regularFontFamily
                        }
                        // Text {
                        //     Layout.fillHeight: true
                        //     Layout.fillWidth: true
                        //     // text: model.platformId
                        //     text: "Nintendo 64"
                        //     color: thingg.platformTextColor
                        //     verticalAlignment: Text.AlignVCenter
                        //     font.pixelSize: 15
                        //     font.weight: Font.Normal
                        //     font.family: Constants.regularFontFamily
                        // }
                    }
                }
            }
        }
    }


    // Flickable {
    //     // anchors.right: parent.right
    //     // anchors.top: parent.top
    //     // anchors.bottom: parent.bottom
    //     // anchors.left: listView.right
    //
    //     // SplitView.preferredWidth: parent.width / 2
    //
    //     boundsBehavior: Flickable.StopAtBounds
    //
    //     ScrollBar.vertical: ScrollBar {
    //     }
    //
    //     clip: true
    //
    //     contentHeight: col.height
    //
    //     ColumnLayout {
    //         id: col
    //         width: parent.width
    //         height: 2000
    //         Rectangle {
    //             Layout.fillWidth: true
    //             Layout.preferredHeight: 300
    //             color: "grey"
    //
    //             Text {
    //                 anchors.top: parent.top
    //                 anchors.left: parent.left
    //                 anchors.right: previewImage.left
    //                 text: listView.currentItem.model.displayName
    //                 color: "white"
    //                 font.pointSize: 24
    //                 font.family: Constants.regularFontFamily
    //                 font.weight: Font.DemiBold
    //                 horizontalAlignment: Text.AlignLeft
    //                 verticalAlignment: Text.AlignVCenter
    //                 height: parent.height * 2 / 3
    //             }
    //
    //             Rectangle {
    //                 anchors.bottom: parent.bottom
    //                 anchors.left: parent.left
    //                 anchors.right: previewImage.left
    //                 height: parent.height / 3
    //                 color: "lightblue"
    //             }
    //
    //             Rectangle {
    //                 id: previewImage
    //                 anchors.right: parent.right
    //                 anchors.top: parent.top
    //                 anchors.bottom: parent.bottom
    //                 width: parent.width / 2
    //             }
    //         }
    //
    //         FocusScope {
    //             Layout.fillWidth: true
    //             Layout.preferredHeight: 60
    //             Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
    //
    //             NavigationTabBar {
    //                 id: navBar
    //
    //                 tabs: ["Details", "Achievements", "Settings"]
    //                 tabWidth: 160
    //                 height: parent.height
    //                 anchors.horizontalCenter: parent.horizontalCenter
    //             }
    //         }
    //
    //         SwipeView {
    //             id: swipeView
    //             Layout.fillWidth: true
    //             Layout.fillHeight: true
    //
    //             clip: true
    //
    //             currentIndex: navBar.currentIndex
    //             onCurrentIndexChanged: navBar.currentIndex = currentIndex
    //
    //             Rectangle {
    //                 color: "blue"
    //             }
    //
    //             Rectangle {
    //                 color: "green"
    //             }
    //
    //             Rectangle {
    //                 color: "yellow"
    //             }
    //
    //         }
    //     }
    // }

    // GridView {
    //     id: listView
    //     objectName: "GridView"
    //     clip: true
    //     width: Math.min(Math.floor(parent.width / cellContentWidth), 7) * cellContentWidth
    //     anchors.horizontalCenter: parent.horizontalCenter
    //     height: parent.height
    //
    //     preferredHighlightBegin: 0.33 * listView.height
    //     preferredHighlightEnd: 0.66 * listView.height
    //     highlightRangeMode: GridView.ApplyRange
    //
    //     populate: Transition {
    //         id: popTransition
    //         SequentialAnimation {
    //             PauseAnimation {
    //                 duration: popTransition.ViewTransition.index * 10
    //             }
    //             ParallelAnimation {
    //                 PropertyAnimation {
    //                     property: "opacity"
    //                     from: 0
    //                     to: 1
    //                     duration: 100
    //                     easing.type: Easing.InOutQuad
    //                 }
    //                 PropertyAnimation {
    //                     property: "y"
    //                     from: popTransition.ViewTransition.destination.y + 20
    //                     to: popTransition.ViewTransition.destination.y
    //                     duration: 100
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
    //             text: "All Games"
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
    //     readonly property int cellSpacing: 8
    //     readonly property int cellContentWidth: 180 + cellSpacing
    //     readonly property int cellContentHeight: 240 + cellSpacing
    //
    //     cellWidth: cellContentWidth
    //     cellHeight: cellContentHeight
    //     focus: true
    //
    //     currentIndex: 0
    //     boundsBehavior: Flickable.StopAtBounds
    //
    //     delegate: GameGridItemDelegate {
    //         opacity: 1
    //         cellSpacing: listView.cellSpacing
    //         cellWidth: listView.cellContentWidth
    //         cellHeight: listView.cellContentHeight
    //
    //         Component.onCompleted: {
    //             openDetails.connect(page.openDetails)
    //             startGame.connect(page.startGame)
    //         }
    //     }
    // }

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

