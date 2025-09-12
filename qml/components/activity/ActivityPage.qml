import QtQuick
import QtQuick.Controls
import QtQuick.Layouts 1.0
import Firelight 1.0

FocusScope {
    id: root

    ListView {
        property var anchorMargins: {
            if (parent.width > 1000) {
                return (parent.width - 1000) / 2
            } else {
                return 0
            }
        }

        anchors.leftMargin: anchorMargins
        anchors.rightMargin: anchorMargins

        anchors.fill: parent
        model: GameActivityModel{}
        delegate:  RowLayout {
            height: 60
            required property var model
            required property var index
          spacing: 16
          Rectangle {
              color: "#292929"
              visible: model.iconUrl1x1 === ""
              implicitWidth: 48
              implicitHeight: 48

              FLIcon {
                  icon: model.platformSlug
                  color: "#595959"
                  anchors.centerIn: parent
                  height: parent.height - 16
                  width: parent.width - 16
                  size: height
              }
          }
          Image {
              source: model.iconUrl1x1
              visible: model.iconUrl1x1 !== ""
              asynchronous: true
              Layout.preferredHeight: 48
              Layout.preferredWidth: 48
              Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
              sourceSize.width: 48
              sourceSize.height: 48
              smooth: true
              fillMode: Image.PreserveAspectFit
          }
          Text {
              text: model.displayName
              font.pixelSize: 17
              font.weight: Font.DemiBold
              font.family: Constants.regularFontFamily
              color: "white"
              Layout.fillHeight: true
              Layout.preferredWidth: 600
              elide: Text.ElideRight
              maximumLineCount: 1
              horizontalAlignment: Text.AlignLeft
              verticalAlignment: Text.AlignVCenter
          }
          Item {
             Layout.fillHeight: true
             Layout.fillWidth: true
         }
          Text {
              property var hours: {
                  return Math.floor(model.numSecondsPlayed / 3600);
              }

              property var minutes: {
                  return Math.floor((model.numSecondsPlayed % 3600) / 60);
              }

              property var seconds: {
                  return model.numSecondsPlayed % 60;
              }

              text: (hours > 0 ? hours + " hours, " : "") + (minutes > 0 ? minutes + " minutes, " : "") + (seconds > 0 ? seconds + ( seconds > 1 ? " seconds" : " second") : "")
              color: ColorPalette.neutral300
              font.pixelSize: 17
              font.weight: Font.Medium
              font.family: Constants.regularFontFamily
          }

      }



        // delegate:
        // delegate: RowLayout {
        //     required property var model
        //     required property var index
        //     spacing: 0
        //
        //     height: 80
        //
        //     property var hours: {
        //         return Math.floor(model.numSecondsPlayed / 3600);
        //     }
        //
        //     property var minutes: {
        //         return Math.floor((model.numSecondsPlayed % 3600) / 60);
        //     }
        //
        //     property var seconds: {
        //         return model.numSecondsPlayed % 60;
        //     }
        //
        //     Text {
        //         text: hours
        //         color: "white"
        //         font.pixelSize: 19
        //         font.weight: Font.DemiBold
        //         font.family: Constants.regularFontFamily
        //         visible: hours > 0
        //         Layout.fillHeight: true
        //         Layout.alignment: Qt.AlignBottom
        //         verticalAlignment: Text.AlignBottom
        //     }
        //
        //     Text {
        //         text: "h "
        //         color: ColorPalette.neutral300
        //         font.pixelSize: 18
        //         font.family: Constants.regularFontFamily
        //         visible: hours > 0
        //         Layout.fillHeight: true
        //         Layout.alignment: Qt.AlignBottom
        //         verticalAlignment: Text.AlignBottom
        //     }
        //
        //     Text {
        //         text: minutes
        //         color: "white"
        //         font.pixelSize: 19
        //         font.weight: Font.DemiBold
        //         font.family: Constants.regularFontFamily
        //         visible: minutes > 0 || hours > 0
        //         Layout.fillHeight: true
        //         Layout.alignment: Qt.AlignBottom
        //         verticalAlignment: Text.AlignBottom
        //     }
        //
        //     Text {
        //         text: "m "
        //         color: ColorPalette.neutral300
        //         font.pixelSize: 18
        //         font.family: Constants.regularFontFamily
        //         visible: minutes > 0 || hours > 0
        //         Layout.fillHeight: true
        //         Layout.alignment: Qt.AlignBottom
        //         verticalAlignment: Text.AlignBottom
        //     }
        //
        //     Text {
        //         text: seconds
        //         color: "white"
        //         font.pixelSize: 19
        //         font.weight: Font.DemiBold
        //         font.family: Constants.regularFontFamily
        //         visible: minutes > 0 || hours > 0 || seconds > 0
        //         Layout.fillHeight: true
        //         Layout.alignment: Qt.AlignBottom
        //         verticalAlignment: Text.AlignBottom
        //     }
        //
        //     Text {
        //         text: "s "
        //         color: ColorPalette.neutral300
        //         font.pixelSize: 18
        //         font.family: Constants.regularFontFamily
        //         visible: minutes > 0 || hours > 0 || seconds > 0
        //         Layout.fillHeight: true
        //         Layout.alignment: Qt.AlignBottom
        //         verticalAlignment: Text.AlignBottom
        //     }
        // }
    }

}
