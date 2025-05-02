import QtQuick
import QtQuick.Controls
import Firelight 1.0

Item {
    id: root

    required property var epochSeconds

    implicitHeight: timeText.height + dateText.height
    implicitWidth: Math.max(timeText.width, dateText.width)
    Text {
        id: timeText
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: parent.height / 2

        text: {
              let date = new Date(epochSeconds)
              return date.toLocaleTimeString()
          }
          font.pixelSize: 16
          font.weight: Font.DemiBold
          font.family: Constants.regularFontFamily
          color: "white"
          verticalAlignment: Text.AlignBottom
          horizontalAlignment: Text.AlignLeft
    }

    Text {
        id: dateText
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: parent.height / 2

        text: {
              let date = new Date(epochSeconds)
              return date.toLocaleDateString()
          }
          font.pixelSize: 16
          font.weight: Font.DemiBold
          font.family: Constants.regularFontFamily
          color: "#7e7e7e"
          verticalAlignment: Text.AlignTop
          horizontalAlignment: Text.AlignLeft
    }


}
