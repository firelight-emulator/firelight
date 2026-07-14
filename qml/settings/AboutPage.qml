import QtQuick
import QtQuick.Controls
import QtQuick.Layouts


Item {
    Text {
        text: "Firelight Emulator\nv0.12.2"
        font.pixelSize: AppStyle.fontSizeMedium
        font.family: Constants.regularFontFamily
        color: Theme.textPrimary
        width: parent.width
        font.weight: Font.DemiBold
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
}