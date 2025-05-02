import QtQuick
import QtQuick.Controls
import QtMultimedia
import QtQuick.Effects
import QtQuick.Layouts
import QtQml
import Qt.labs.qmlmodels 1.0
import QtQuick.Shapes 1.8
// import Qt5Compat.GraphicalEffects
import Firelight 1.0

FocusScope {
    id: root

    property alias currentIndex: list.currentIndex
    property alias count: list.count

    signal itemSelected(index: int)

    function itemAtIndex(index) {
        return list.itemAtIndex(index)
    }

    required property var model
    required property real aspectRatio
    // required property Component libraryPage

    Timer {
        id: wheelTimer
        interval: 500
        repeat: false
    }

    WheelHandler {
        onWheel: function(event) {
            if (wheelTimer.running) {
                if (list.currentIndex === 0 && event.angleDelta.y < 0) {
                    wheelTimer.restart()
                    return
                } else if (list.currentIndex === list.count - 1 && event.angleDelta.y >= 0) {
                    wheelTimer.restart()
                    return
                }
            }

            if (event.angleDelta.y >= 0) {
                list.incrementCurrentIndex()
            } else {
                list.decrementCurrentIndex()
            }

            wheelTimer.restart()
        }
    }

    FLSoundEffect {
        id: sfx
        source: "qrc:sfx/click"
    }

    ListView {
        id: list
        anchors.fill: parent
        focus: true
        highlightMoveDuration: 80
        highlightMoveVelocity: -1
        highlightRangeMode: ListView.StrictlyEnforceRange
        keyNavigationEnabled: true
        keyNavigationWraps: true
        layoutDirection: Qt.RightToLeft
        orientation: ListView.Horizontal
        preferredHighlightBegin: (list.width / 2) - ((180 * root.aspectRatio) / 2)
        preferredHighlightEnd: (list.width / 2) + ((180 * root.aspectRatio) / 2)
        spacing: 16
        width: parent.width

        onCurrentIndexChanged: {
            sfx.play();
            console.log("Current index changed: ", list.currentIndex)
        }

        model: root.model

        delegate: FocusScope {
          id: dele

          required property var model
          required property var index

          height: 180
          width: height * root.aspectRatio

          Behavior on width {
              NumberAnimation {
                  duration: 80
              }
          }

          Button {
              property bool showGlobalCursor: true
              anchors.verticalCenter: parent.verticalCenter
              padding: 0
              spacing: 0
              horizontalPadding: 0
              focus: true
              height: 180
              // height: dele.ListView.isCurrentItem ? 270 : 180
              width: height * root.aspectRatio

              property var inputHints: [
                  {
                      input: Qt.Key_Select,
                      label: "Play"
                  },
                  {
                      input: Qt.Key_Menu,
                      label: "Details"
                  }
              ]

              Behavior on height {
                  NumberAnimation {
                      duration: 80
                  }
              }
              // radius: 4

              Behavior on width {
                  NumberAnimation {
                      duration: 80
                  }
              }

              onClicked: {
                  if (!dele.ListView.isCurrentItem) {
                      dele.ListView.view.currentIndex = dele.index
                      return
                  } else {
                        console.log("selected: ", dele.index)
                      root.itemSelected(dele.index)
                  }
              }

              HoverHandler {
                  cursorShape: Qt.PointingHandCursor
              }

              background: Item {}

              contentItem: Image {
                  id: sourceImage
                  source: dele.model.modelData.image_url
              }
          }
      }
    }
}
