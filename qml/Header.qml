import QtQuick

Rectangle {
    color: "transparent"

    Text {
        text: "😎 clock"
        anchors.fill: parent
        horizontalAlignment: Text.AlignRight
        verticalAlignment: Text.AlignVCenter
        color: Constants.colorTestText
        font.pointSize: 12
        font.family: Constants.regularFontFamily
    }
}