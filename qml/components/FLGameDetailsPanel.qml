import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import Firelight 1.0

ColumnLayout {
    id: root

    required property var entryId

    spacing: 12

    LibraryEntry {
        id: libEntry
        entryId: root.entryId
    }

    // AchievementSet {
    //     id: achievements
    //     setId: libEntry.achievementSetId
    // }

    GameActivity {
        id: gameActivity
        contentHash: libEntry.contentHash
    }

    // RowLayout {
    //     Layout.fillWidth: true
    //     Layout.maximumHeight: 42
    //     spacing: 8
    //     Button {
    //         Layout.fillHeight: true
    //         horizontalPadding: 12
    //         autoExclusive: true
    //         checkable: true
    //         background: Rectangle {
    //             color: "white"
    //             opacity: parent.pressed ? 0.16 : 0.1
    //             radius: 4
    //             visible: (parent.checked || createHoverHandler3.hovered) && parent.enabled
    //         }
    //         HoverHandler {
    //             id: createHoverHandler3
    //             cursorShape: parent.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
    //         }
    //         contentItem: RowLayout {
    //             spacing: 8
    //             FLIcon {
    //                 icon: "info"
    //                 Layout.fillHeight: true
    //                 size: height
    //                 color:"white"
    //                 filled: parent.parent.checked
    //             }
    //             Text {
    //                 Layout.fillHeight: true
    //                 Layout.fillWidth: true
    //                  text: "Details"
    //                 color:"white"
    //                  font.family: Constants.regularFontFamily
    //                  font.pixelSize: 16
    //                  font.weight: Font.DemiBold
    //                  verticalAlignment: Text.AlignVCenter
    //                     horizontalAlignment: Text.AlignHCenter
    //              }
    //          }
    //     }
    //     Button {
    //         Layout.fillHeight: true
    //         autoExclusive: true
    //         checkable: true
    //         horizontalPadding: 12
    //         background: Rectangle {
    //             color: "white"
    //             opacity: parent.pressed ? 0.16 : 0.1
    //             radius: 4
    //             visible: (parent.checked || createHoverHandler2.hovered) && parent.enabled
    //         }
    //         HoverHandler {
    //             id: createHoverHandler2
    //             cursorShape: parent.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
    //         }
    //         contentItem: RowLayout {
    //             spacing: 8
    //             FLIcon {
    //                 id: createIcon
    //                 icon: "trophy"
    //                 Layout.fillHeight: true
    //                 size: height
    //                 color:"white"
    //                 filled: parent.parent.checked
    //             }
    //             Text {
    //                 Layout.fillHeight: true
    //                 Layout.fillWidth: true
    //                  text: "Achievements"
    //                 color:"white"
    //                  font.family: Constants.regularFontFamily
    //                  font.pixelSize: 16
    //                  font.weight: Font.DemiBold
    //                  verticalAlignment: Text.AlignVCenter
    //                     horizontalAlignment: Text.AlignHCenter
    //              }
    //          }
    //     }
    //     Button {
    //         Layout.fillHeight: true
    //         horizontalPadding: 12
    //         autoExclusive: true
    //         checkable: true
    //         background: Rectangle {
    //             color: "white"
    //             opacity: parent.pressed ? 0.16 : 0.1
    //             radius: 4
    //             visible: (parent.checked || createHoverHandler.hovered) && parent.enabled
    //         }
    //         HoverHandler {
    //             id: createHoverHandler
    //             cursorShape: parent.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
    //         }
    //         contentItem: RowLayout {
    //             spacing: 8
    //             FLIcon {
    //                 icon: "bar-chart"
    //                 Layout.fillHeight: true
    //                 size: height
    //                 color:"white"
    //                 filled: parent.parent.checked
    //             }
    //             Text {
    //                 Layout.fillHeight: true
    //                 Layout.fillWidth: true
    //                  text: "Activity"
    //                 color:"white"
    //                  font.family: Constants.regularFontFamily
    //                  font.pixelSize: 16
    //                  font.weight: Font.DemiBold
    //                  verticalAlignment: Text.AlignVCenter
    //                     horizontalAlignment: Text.AlignHCenter
    //              }
    //          }
    //     }
    //     Button {
    //         Layout.fillHeight: true
    //         horizontalPadding: 12
    //         autoExclusive: true
    //         checkable: true
    //         background: Rectangle {
    //             color: "white"
    //             opacity: parent.pressed ? 0.16 : 0.1
    //             radius: 4
    //             visible: (parent.checked || createHoverHandler4.hovered) && parent.enabled
    //         }
    //         HoverHandler {
    //             id: createHoverHandler4
    //             cursorShape: parent.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
    //         }
    //         contentItem: RowLayout {
    //             spacing: 8
    //             FLIcon {
    //                 icon: "settings"
    //                 Layout.fillHeight: true
    //                 size: height
    //                 color:"white"
    //                 filled: parent.parent.checked
    //             }
    //             Text {
    //                 Layout.fillHeight: true
    //                 Layout.fillWidth: true
    //                  text: "Settings"
    //                 color:"white"
    //                  font.family: Constants.regularFontFamily
    //                  font.pixelSize: 16
    //                  font.weight: Font.DemiBold
    //                  verticalAlignment: Text.AlignVCenter
    //                     horizontalAlignment: Text.AlignHCenter
    //              }
    //          }
    //     }
    // }
    //
    // Rectangle {
    //     color: "#454545"
    //     Layout.fillWidth: true
    //     Layout.maximumHeight: 1
    //     Layout.minimumHeight: 1
    // }

    FLGameActivityPage {
        Layout.fillWidth: true
        Layout.fillHeight: true
        activity: gameActivity
    }

    // Pane {
    //     id: thingy
    //     clip: true
    //     padding: 16
    //     Layout.fillWidth: true
    //     Layout.fillHeight: true
    //     // background: Item{}
    //     background: Rectangle {
    //         color: "transparent"
    //         radius: 12
    //         MultiEffect {
    //             source: ShaderEffectSource {
    //                 width: thingy.width
    //                 height: thingy.height
    //                 sourceItem: window.background
    //                 sourceRect: Qt.rect(thingy.x, thingy.y, thingy.width, thingy.height)
    //             }
    //
    //             anchors.fill: parent
    //             blurEnabled: true
    //             blurMultiplier: 1.0
    //             blurMax: 64
    //             blur: 1.0
    //         }
    //
    //         Rectangle {
    //             gradient: Gradient {
    //                 GradientStop { position: 0.0; color: "#222227" }
    //                 GradientStop { position: 0.2; color: "#2d2d32" }
    //                 GradientStop { position: 0.8; color: "#2d2d32" }
    //                 GradientStop { position: 1.0; color: "#222227" }
    //             }
    //             // color: "#1D1D1D"
    //             opacity: 0.68
    //             anchors.fill: parent
    //             radius: 12
    //         }
    //     }
    //     contentItem: ColumnLayout {
    //         spacing: 16
    //
    //         Text {
    //             text: "Play Sessions"
    //             Layout.fillWidth: true
    //             color: "white"
    //             font.family: Constants.regularFontFamily
    //             font.weight: Font.DemiBold
    //             font.pixelSize: 18
    //             // verticalAlignment: Qt.AlignVCenter
    //         }
    //
    //         // Rectangle {
    //         //     color: "#454545"
    //         //     Layout.fillWidth: true
    //         //     Layout.maximumHeight: 1
    //         //     Layout.minimumHeight: 1
    //         // }
    //
    //         // Pane {
    //         //      id: activityHeader
    //         //      property var startHeaderWidth: 100
    //         //      property var endHeaderWidth: 100
    //         //      property var timePlayedHeaderWidth: 100
    //         //      padding: 8
    //         //      leftInset: -16
    //         //     rightInset: -16
    //         //     topInset: -16
    //         //      Layout.fillWidth: true
    //         //      Layout.preferredHeight: 24
    //         //      background: Rectangle {
    //         //          color: "#373737"
    //         //          topLeftRadius: 8
    //         //             topRightRadius: 8
    //         //          // Rectangle {
    //         //          //     anchors.bottom: parent.bottom
    //         //          //     anchors.left: parent.left
    //         //          //     anchors.right: parent.right
    //         //          //     height: 1
    //         //          //     color: "#6e6e6e"
    //         //          // }
    //         //      }
    //         //      contentItem: RowLayout {
    //         //          Text {
    //         //              id: startTimeHeader
    //         //              Layout.preferredWidth: activityHeader.startHeaderWidth
    //         //              Layout.fillHeight: true
    //         //              text: "Started"
    //         //              font.pixelSize: 16
    //         //              font.weight: Font.DemiBold
    //         //              font.family: Constants.regularFontFamily
    //         //              color: "#9e9e9e"
    //         //              verticalAlignment: Text.AlignVCenter
    //         //              horizontalAlignment: Text.AlignLeft
    //         //          }
    //         //
    //         //          Text {
    //         //              id: endTimeHeader
    //         //              Layout.preferredWidth: activityHeader.endHeaderWidth
    //         //              Layout.fillHeight: true
    //         //              text: "Ended"
    //         //              font.pixelSize: 16
    //         //              font.weight: Font.DemiBold
    //         //              font.family: Constants.regularFontFamily
    //         //              color: "#9e9e9e"
    //         //              verticalAlignment: Text.AlignVCenter
    //         //              horizontalAlignment: Text.AlignLeft
    //         //          }
    //         //
    //         //          Text {
    //         //              id: timePlayedHeader
    //         //              Layout.preferredWidth: activityHeader.timePlayedHeaderWidth
    //         //              Layout.fillHeight: true
    //         //              text: "Time played"
    //         //              font.pixelSize: 16
    //         //              font.weight: Font.DemiBold
    //         //              font.family: Constants.regularFontFamily
    //         //              color: "#9e9e9e"
    //         //              verticalAlignment: Text.AlignVCenter
    //         //              horizontalAlignment: Text.AlignLeft
    //         //          }
    //         //      }
    //         //  }
    //         ListView {
    //              property var startHeaderWidth: 100
    //              property var endHeaderWidth: 100
    //              property var timePlayedHeaderWidth: 100
    //              Layout.fillWidth: true
    //              Layout.fillHeight: true
    //              spacing: 8
    //
    //              Rectangle {
    //                  anchors.top: parent.top
    //                  // x: -16
    //                  // width: parent.width + 32
    //                  anchors.leftMargin: -16
    //                     anchors.rightMargin: -16
    //                     anchors.left: parent.left
    //                     anchors.right: parent.right
    //                     height: 48
    //                     opacity: parent.contentY > 0 ? 0.3 : 0
    //                     gradient: Gradient {
    //                         GradientStop { position: 0.0; color: "#000000" }
    //                         GradientStop { position: 1.0; color: "transparent" }
    //                     }
    //
    //                     Behavior on opacity {
    //                         NumberAnimation {
    //                             duration: 200
    //                             easing.type: Easing.OutQuad
    //                         }
    //                     }
    //              }
    //
    //              model: gameActivity.playSessions
    //              // delegate: TableViewDelegate {
    //              //     width: TableView.view.width
    //              // }
    //              delegate: Pane {
    //                  id: item
    //                  required property var model
    //                  required property var index
    //                  height: 72
    //                  width: ListView.view.width
    //                  padding: 8
    //
    //                  background: Item{
    //                      Rectangle {
    //                          color: "#373737"
    //                          height: 1
    //                             anchors.bottom: parent.bottom
    //                             anchors.left: parent.left
    //                             anchors.right: parent.right
    //                      }
    //                  }
    //
    //                  contentItem: RowLayout {
    //                      spacing: 8
    //
    //                      FLDateTime {
    //                          Layout.preferredWidth: item.ListView.view.startHeaderWidth
    //                          Layout.maximumWidth: item.ListView.view.startHeaderWidth
    //                          Layout.fillHeight: true
    //
    //                          epochSeconds: item.model.startTime
    //                      }
    //
    //                      FLDateTime {
    //                          Layout.preferredWidth: item.ListView.view.startHeaderWidth
    //                          Layout.maximumWidth: item.ListView.view.startHeaderWidth
    //                          Layout.fillHeight: true
    //
    //                          epochSeconds: item.model.endTime
    //                      }
    //
    //                      Text {
    //                          Layout.preferredWidth: item.ListView.view.timePlayedHeaderWidth
    //                          Layout.fillHeight: true
    //                          text: {
    //                              if (item.model.numUnpausedSeconds === 0) {
    //                                  return "<b>0</b>s"
    //                              }
    //
    //                              let hours = Math.floor(item.model.numUnpausedSeconds / 3600)
    //                             let minutes = Math.floor((item.model.numUnpausedSeconds % 3600) / 60)
    //                             let seconds = item.model.numUnpausedSeconds % 60
    //
    //                             if (hours > 0) {
    //                                 return ("<b>" + hours + "</b>h ") + ("<b>" + minutes + "</b>m ") + ("<b>" + seconds + "</b>s ")
    //                             } else if (minutes > 0) {
    //                                 return ("<b>" + minutes + "</b>m ") + ("<b>" + seconds + "</b>s ")
    //                             } else if (seconds > 0) {
    //                                 return ("<b>" + seconds + "</b>s ")
    //                             }
    //
    //                             // return (hours > 0 ? ("<em>" + hours + "</em>h ") : "") + (minutes > 0 ? ("<em>" + minutes + "</em>m ") : "") + "<em>" + seconds + "</em>s "
    //                              // model.numUnpausedSeconds
    //                          }
    //                          font.pixelSize: 16
    //                          font.weight: Font.Medium
    //                          font.family: Constants.regularFontFamily
    //                          color: "white"
    //                          verticalAlignment: Text.AlignVCenter
    //                          horizontalAlignment: Text.AlignLeft
    //                      }
    //
    //
    //                  }
    //              }
    //          }
    //      }
    // }
    //
    // // FLTwoColumnMenu {
    // //     Layout.fillWidth: true
    // //     Layout.fillHeight: true
    // //
    // //     menuItems: ["Achievements", "Activity"]
    // //     pages: [thing, activity]
    // // }
    // //
    // // Component {
    // //     id: activity
    // //
    // //
    // // }
    //
    // // RowLayout {
    // //     Layout.fillWidth: true
    // //     Layout.fillHeight: true
    // //     ListView {
    // //         Layout.fillHeight: true
    // //         Layout.preferredWidth: 200
    // //         model: ListModel {
    // //             ListElement {
    // //                 name: "Details"
    // //             }
    // //             ListElement {
    // //                 name: "Achievements"
    // //             }
    // //             ListElement {
    // //                 name: "Activity"
    // //             }
    // //             ListElement {
    // //                 name: "Settings"
    // //             }
    // //         }
    // //         delegate: Text {
    // //             required property var model
    // //             text: model.name
    // //             color: "white"
    // //             font.family: Constants.regularFontFamily
    // //             font.weight: Font.Medium
    // //             font.pixelSize: 24
    // //             verticalAlignment: Qt.AlignVCenter
    // //         }
    // //     }
    //
    // //     Rectangle {
    // //         color: "red"
    // //         Layout.fillWidth: true
    // //         Layout.fillHeight: true
    // //     }
    // // }
    //
    // Component {
    //     id: thing
    //     FocusScope {
    //
    //         clip: true
    //         // Text {
    //         //     anchors.fill: parent
    //         //     text: "lol no achievements"
    //         //     // visible: achievements.numAchievements === 0
    //         //     font.pixelSize: 24
    //         //     font.weight: Font.Medium
    //         //     font.family: Constants.regularFontFamily
    //         //     color: "white"
    //         //     verticalAlignment: Qt.AlignVCenter
    //         //     horizontalAlignment: Qt.AlignHCenter
    //         // }
    //         ListView {
    //             id: theList
    //             // anchors.left: gameNavList.right
    //             anchors.fill: parent
    //             anchors.leftMargin: 4
    //             anchors.rightMargin: 4
    //             anchors.topMargin: 4
    //             anchors.bottomMargin: 4
    //             focus: true
    //             // visible: achievements.numAchievements > 0
    //             highlightMoveDuration: 80
    //             highlightMoveVelocity: -1
    //             highlightRangeMode: InputMethodManager.usingMouse ? ListView.NoHighlightRange : ListView.ApplyRange
    //             preferredHighlightBegin: 64
    //             preferredHighlightEnd: height - 64
    //             model: achievements.achievements
    //
    //             ScrollBar.vertical: ScrollBar { }
    //
    //             spacing: 0
    //
    //             // Timer {
    //             //     id: wheelTimer
    //             //     interval: 500
    //             //     repeat: false
    //             // }
    //             //
    //             // WheelHandler {
    //             //     onWheel: function(event) {
    //             //         if (wheelTimer.running) {
    //             //             if (theList.currentIndex === 0 && event.angleDelta.y >= 0) {
    //             //                 wheelTimer.restart()
    //             //                 return
    //             //             } else if (theList.currentIndex === theList.count - 1 && event.angleDelta.y < 0) {
    //             //                 wheelTimer.restart()
    //             //                 return
    //             //             }
    //             //         }
    //             //
    //             //         if (event.angleDelta.y >= 0) {
    //             //             // theList.highlightRangeMode = ListView.ApplyRange
    //             //             theList.decrementCurrentIndex()
    //             //         } else {
    //             //             // theList.highlightRangeMode = ListView.ApplyRange
    //             //             theList.incrementCurrentIndex()
    //             //         }
    //             //
    //             //         wheelTimer.restart()
    //             //     }
    //             // }
    //
    //             delegate: Button {
    //                 id: tttt
    //                 required property var model
    //                 required property var index
    //                 width: ListView.view.width
    //
    //                 property bool showGlobalCursor: true
    //
    //                 RightClickMenu {
    //                     id: rightClick
    //
    //                     RightClickMenuItem {
    //                         text: "See on RetroAchievements.org"
    //                         externalLink: true
    //                         onTriggered: {
    //                             Qt.openUrlExternally("https://retroachievements.org/achievement/" + tttt.model.achievement_id)
    //                         }
    //                     }
    //
    //                 }
    //
    //                 TapHandler {
    //                     acceptedButtons: Qt.LeftButton | Qt.RightButton
    //
    //                     onSingleTapped: function(event, button) {
    //                         if (button === 2) {
    //                             rightClick.popupNormal()
    //                         }
    //                         if (tttt.ListView.view.currentIndex !== tttt.index) {
    //                             // theList.highlightRangeMode = ListView.NoHighlightRange
    //                             tttt.ListView.view.currentIndex = tttt.index
    //                         }
    //                     }
    //                 }
    //
    //                 HoverHandler {
    //                     id: hoverHandler
    //                     cursorShape: Qt.PointingHandCursor
    //                 }
    //
    //                 height: 80
    //                 padding: 8
    //                 horizontalPadding: 12
    //
    //                 background: Rectangle {
    //                     color: "white"
    //                     opacity: tttt.activeFocus ? 0.2 : (hoverHandler.hovered ? 0.1 : 0.0)
    //                     // color: "#333333"
    //                     //
    //                     // gradient: Gradient {
    //                     //     orientation: Gradient.Horizontal
    //                     //     GradientStop { position: 0.0; color: "#252525" }
    //                     //     GradientStop { position: 1.0; color: "#363636" }
    //                     // }
    //                     radius: 6
    //                     // opacity: 0.95
    //                     // visible: tttt.ListView.isCurrentItem
    //                 }
    //
    //                 contentItem: RowLayout {
    //                     spacing: 16
    //                     Image {
    //                         source: tttt.model.image_url
    //                         Layout.preferredHeight: parent.height
    //                         Layout.preferredWidth: parent.height
    //                         Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
    //                         sourceSize.width: parent.height
    //                         sourceSize.height: parent.height
    //                         smooth: false
    //                         fillMode: Image.PreserveAspectFit
    //                     }
    //                     ColumnLayout {
    //                         Layout.fillWidth: true
    //                         Layout.fillHeight: true
    //                         Layout.topMargin: 4
    //                         Layout.bottomMargin: 4
    //                         spacing: 0
    //                         Text {
    //                             text: tttt.model.name
    //                             font.pixelSize: 17
    //                             font.weight: Font.DemiBold
    //                             font.family: Constants.regularFontFamily
    //                             // color: tttt.ListView.isCurrentItem ? "black" : "white"
    //                             color: "white"
    //                             elide: Text.ElideRight
    //                             Layout.fillWidth: true
    //                             maximumLineCount: 1
    //                             horizontalAlignment: Text.AlignLeft
    //                             verticalAlignment: Text.AlignVCenter
    //                         }
    //                         Text {
    //                             text: tttt.model.description
    //                             font.pixelSize: 15
    //                             font.weight: Font.Normal
    //                             font.family: Constants.regularFontFamily
    //                             // color: tttt.ListView.isCurrentItem ? "black" : "white"
    //                             color: "#dadada"
    //                             Layout.fillHeight: true
    //                             elide: Text.ElideRight
    //                             Layout.fillWidth: true
    //                             maximumLineCount: 2
    //                             horizontalAlignment: Text.AlignLeft
    //                             verticalAlignment: Text.AlignVCenter
    //                             wrapMode: Text.WordWrap
    //                         }
    //                     }
    //
    //                     Text {
    //                         text: tttt.model.earned ? tttt.model.earned_date : "Not earned"
    //                         font.pixelSize: 17
    //                         font.weight: Font.Medium
    //                         font.family: Constants.regularFontFamily
    //                         // color: tttt.ListView.isCurrentItem ? "black" : "white"
    //                         color: "white"
    //                         Layout.fillHeight: true
    //                         Layout.preferredWidth: 300
    //                         elide: Text.ElideRight
    //                         maximumLineCount: 1
    //                         horizontalAlignment: Text.AlignLeft
    //                         verticalAlignment: Text.AlignVCenter
    //                     }
    //
    //                     Text {
    //                         text: tttt.model.points
    //                         font.pixelSize: 17
    //                         font.weight: Font.Medium
    //                         font.family: Constants.regularFontFamily
    //                         // color: tttt.ListView.isCurrentItem ? "black" : "white"
    //                         color: "white"
    //                         Layout.fillHeight: true
    //                         horizontalAlignment: Text.AlignRight
    //                         verticalAlignment: Text.AlignVCenter
    //                         Layout.preferredWidth: 100
    //                     }
    //                     Item {
    //                         Layout.fillHeight: true
    //                         Layout.fillWidth: true
    //                     }
    //                 }
    //             }
    //         }
    //     }
    //
    // }

}
