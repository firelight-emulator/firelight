import QtQuick
import QtQuick.Controls

Pane {
    property alias text: label.text

    background: Item {
    }

    Text {
        id: label
        color: Theme.textPrimary
        font.family: Constants.regularFontFamily
        font.pointSize: 12
        font.weight: Font.DemiBold
    }
}