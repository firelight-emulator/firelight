import QtQuick


Rectangle {
    color: "transparent"

    Text {
        text: "Firelight is made with ❤️ by BiscuitCakes"
        anchors.centerIn: parent
        color: Constants.colorTestTextMuted
        font.pointSize: 8
        font.family: Constants.lightFontFamily
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
}