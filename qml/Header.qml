import QtQuick
import FirelightStyle 1.0

Rectangle {
    color: "transparent"

    Text {
        text: "6:30pm"
        anchors.fill: parent
        horizontalAlignment: Text.AlignRight
        verticalAlignment: Text.AlignVCenter
        color: Constants.colorTestText
        font.pointSize: 12
        font.family: Constants.regularFontFamily
    }
}