import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Window
import QtQml.Models
import QtQuick.Layouts 1.0
import QtNetwork
import QtMultimedia
import QtQuick.Effects
import Firelight 1.0

ListView {
    id: root

    property real initialContentY: contentY

    property string sortRole: "displayName"
    property bool sortAscending: true

    property real titleColumnWidth: 340
    property real timePlayedColumnWidth: 128
    property real lastPlayedColumnWidth: 128
    property real achievementColumnWidth: 128

    Component.onCompleted: {
        initialContentY = contentY
    }

    ScrollBar.vertical: ScrollBar {
        anchors.left: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 8
        // contentItem: Rectangle {
        //     color: "#ffffff"
        //     radius: 4
        //     opacity: 0.12
        // }

    }
     boundsBehavior: Flickable.StopAtBounds

     headerPositioning: ListView.OverlayHeader
     header: Pane {
        z: 2
        padding: 0
        width: ListView.view.width
        background: Rectangle {
            color: "#121010"
            topLeftRadius: 8
            topRightRadius: 8
            opacity: root.contentY > 0 ? 1 : 0
            width: parent.width + 48
            height: parent.height - 16
            anchors.horizontalCenter: parent.horizontalCenter

            Behavior on opacity {
                NumberAnimation {
                    duration: 160
                    easing.type: Easing.InOutQuad
                }
            }
        }
        contentItem: ColumnLayout {
             id: gameListHeader
             spacing: 8
             RowLayout {
                 Layout.fillWidth: true
                 Layout.topMargin: 8
                 Layout.leftMargin: 10
                 Layout.rightMargin: 10
                 Layout.minimumHeight: 32
                 Layout.maximumHeight: 32
                 spacing: 16

                 HoverHandler {
                     id: headerHoverHandler
                 }

                 Button {
                     padding: 0
                     implicitHeight: 24
                     implicitWidth: 24
                     Layout.alignment: Qt.AlignVCenter
                     icon.source: "qrc:/icons/favorite"
                     icon.width: 24
                     icon.height: 24
                     icon.color: "#9c9c9c"
                     background: Item {}
                 }

                 Item {
                     implicitWidth: 48
                     implicitHeight: 1
                 }

                 SplitView {
                     Layout.fillWidth: true
                     Layout.fillHeight: true
                     padding: 0

                     handle: Item {
                         implicitWidth: 16
                         SplitView.fillHeight: true

                         Rectangle {
                             anchors.centerIn: parent
                             width: 1
                             height: parent.height - 8
                             color: "#FFFFFF"
                             opacity: headerHoverHandler.hovered ? 0.12 : 0
                         }
                     }

                     ListViewColumnHeader {
                         id: titleHeader
                         text: "Title"
                         selected: root.sortRole === "displayName"
                         sortAscending: root.sortAscending

                         SplitView.fillHeight: true
                         SplitView.minimumWidth: 256
                         SplitView.preferredWidth: 340
                         SplitView.fillWidth: true

                         onWidthChanged: {
                             root.titleColumnWidth = width
                         }

                         onTapped: {
                             if (root.sortRole === "displayName") {
                                 root.sortAscending = !root.sortAscending
                             } else {
                                 root.sortAscending = true
                                 root.sortRole = "displayName"
                             }
                         }
                     }

                     ListViewColumnHeader {
                         id: timePlayedHeader
                         text: "Time played"
                         selected: root.sortRole === "platformId"
                         sortAscending: root.sortAscending

                         SplitView.fillHeight: true
                         SplitView.minimumWidth: 128
                         SplitView.preferredWidth: 128

                         onWidthChanged: {
                             root.timePlayedColumnWidth = width
                         }

                         onTapped: {
                             if (root.sortRole === "platformId") {
                                 root.sortAscending = !root.sortAscending
                             } else {
                                 root.sortAscending = true
                                 root.sortRole = "platformId"
                             }
                         }
                     }

                     ListViewColumnHeader {
                         id: lastPlayedAtHeader
                         text: "Last played"
                         selected: root.sortRole === "lastPlayedAt"
                         sortAscending: root.sortAscending

                         SplitView.fillHeight: true
                         SplitView.minimumWidth: 128
                         SplitView.preferredWidth: 128

                         onWidthChanged: {
                             root.lastPlayedColumnWidth = width
                         }

                         onTapped: {
                             if (root.sortRole === "lastPlayedAt") {
                                 root.sortAscending = !root.sortAscending
                             } else {
                                 root.sortAscending = true
                                 root.sortRole = "lastPlayedAt"
                             }
                         }
                     }

                     Text {
                         id: achievementHeader
                         SplitView.fillHeight: true
                         SplitView.minimumWidth: 128
                         SplitView.preferredWidth: 128
                         onWidthChanged: {
                             gamesPanel.achievementColumnWidth = width
                         }
                         text: "Achievements"
                         font.pointSize: 11
                         font.weight: Font.DemiBold
                         font.family: Constants.regularFontFamily
                         color: "#919191"
                         horizontalAlignment: Text.AlignLeft
                         verticalAlignment: Text.AlignVCenter
                     }
                 }

                 Item {
                     implicitWidth: 32
                     implicitHeight: 1
                 }
             }

             Rectangle {
                 Layout.fillWidth: true
                 implicitHeight: 1
                 color: "#FFFFFF"
                 opacity: 0.12
             }
             Item {
                 implicitWidth: 1
                 implicitHeight: 8
             }
         }
     }
     delegate: Button {
        id: delegateButton
        required property var model
         height: 64
         width: ListView.view.width
         padding: 8
         hoverEnabled: true

         TapHandler {
             acceptedButtons: Qt.LeftButton
             onDoubleTapped: EmulationService.loadEntry(delegateButton.model.entryId)
         }

         Keys.onPressed: function (event) {
             if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter || event.key === Qt.Key_Select) {
                 EmulationService.loadEntry(delegateButton.model.entryId)
                 event.accepted = true
             }
         }

         Drag.dragType: Drag.Automatic
         Drag.supportedActions: Qt.CopyAction
         Drag.mimeData: {
             "text/plain": "Copied text"
         }

         DragHandler {
             id: dragHandler
             target: null
             snapMode: DragHandler.SnapAlways

             onActiveChanged:
                 if (active) {
                     parent.grabToImage(function(result) {
                         parent.Drag.imageSource = result.url
                         parent.Drag.hotSpot.x = centroid.pressPosition.x
                         parent.Drag.hotSpot.y = centroid.pressPosition.y
                         parent.Drag.mimeData = {
                             "text/plain": model.entryId
                         }
                         parent.Drag.active = true
                     })
                 } else {
                     parent.Drag.active = false
                 }
         }

         background: Rectangle {
             color: "white"
             opacity: hovered ? 0.08 : 0
             radius: 4
         }
         contentItem: RowLayout {
             spacing: 16


            Image {
                sourceSize.width: 48
                sourceSize.height: 48
                source: model.icon1x1SourceUrl
                fillMode: Image.PreserveAspectFit
                visible: model.icon1x1SourceUrl !== undefined && model.icon1x1SourceUrl !== ""
            }

            Rectangle {
                width: 48
                height: 48
                color: "grey"
                visible: model.icon1x1SourceUrl === "" || model.icon1x1SourceUrl === undefined
            }

             ColumnLayout {
                 Layout.minimumWidth: root.titleColumnWidth
                 Layout.maximumWidth: root.titleColumnWidth
                 Layout.fillHeight: true
                 Text {
                     Layout.fillWidth: true
                     Layout.fillHeight: true
                     Layout.verticalStretchFactor: 1
                     text: model.displayName
                     font.pointSize: 12
                     font.weight: Font.DemiBold
                     font.family: Constants.regularFontFamily
                     color: "#FFFFFF"
                     elide: Text.ElideRight
                 }
                 Text {
                     Layout.fillWidth: true
                     Layout.fillHeight: true
                     Layout.verticalStretchFactor: 1
                     text: model.platformId
                     font.pointSize: 11
                     font.weight: Font.DemiBold
                     font.family: Constants.regularFontFamily
                     color: "#919191"
                     elide: Text.ElideRight
                     }
             }

             Item {
                 width: 24
                 height: 24

                 Button {
                      padding: 0
                      anchors.fill: parent
                      icon.source: model.favorite ? "qrc:/icons/favorite" : "qrc:/icons/empty/favorite"
                      visible: model.favorite || delegateButton.hovered
                     icon.width: 24
                     icon.height: 24
                     icon.color: model.favorite ? Qt.darker("#e55aa2", 1.3) : "#9c9c9c"
                     background: Item {}
                     HoverHandler {
                          cursorShape: Qt.PointingHandCursor
                     }
                     onClicked: {
                        model.favorite = !model.favorite
                     }
                  }
             }

             Text {
                 Layout.fillHeight: true
                 Layout.minimumWidth: root.timePlayedColumnWidth
                 Layout.maximumWidth: root.timePlayedColumnWidth
                 text: "1000 hours"
                 font.pointSize: 12
                 font.weight: Font.Medium
                 font.family: Constants.regularFontFamily
                 color: "#919191"
                 elide: Text.ElideRight
                 verticalAlignment: Text.AlignVCenter
             }
             Text {
                 Layout.fillHeight: true
                 Layout.minimumWidth: root.lastPlayedColumnWidth
                 Layout.maximumWidth: root.lastPlayedColumnWidth
                 text: model.lastPlayedAt
                 font.pointSize: 12
                 font.weight: Font.Medium
                 font.family: Constants.regularFontFamily
                 color: "#919191"
                 elide: Text.ElideRight
                 verticalAlignment: Text.AlignVCenter
             }

             Text {
                 Layout.fillHeight: true
                 Layout.minimumWidth: root.achievementColumnWidth
                 Layout.maximumWidth: root.achievementColumnWidth
                 text: "19/55 achievements"
                 font.pointSize: 12
                 font.weight: Font.Medium
                 font.family: Constants.regularFontFamily
                 color: "#919191"
                 elide: Text.ElideRight
                 verticalAlignment: Text.AlignVCenter
             }
             Button {
                 padding: 0
                 implicitHeight: 32
                 implicitWidth: 32
                 icon.source: "qrc:/icons/more-vertical"
                icon.width: 32
                icon.height: 32
                icon.color: "#919191"
                background: Item {}
                visible: delegateButton.hovered

             }

             Item {
                 Layout.fillWidth: true
                 Layout.fillHeight: true
             }

        }
    }
 }