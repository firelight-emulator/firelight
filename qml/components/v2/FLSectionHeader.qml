import QtQuick

// A section heading. Set `text`
Text {
    color: Theme.textPrimary
    font.pixelSize: AppStyle.fontSizeLarge
    font.family: Constants.regularFontFamily
    font.weight: Font.DemiBold
    verticalAlignment: Text.AlignVCenter
    elide: Text.ElideRight
}
