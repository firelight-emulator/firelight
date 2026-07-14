import QtQuick
import QtQuick.Controls
import QtQuick.Layouts


Item {
    Text {
        text: "System settings will go here"
        font.pixelSize: AppStyle.fontSizeMedium
        font.family: Constants.regularFontFamily
        color: Theme.textMuted
        anchors.centerIn: parent
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
}