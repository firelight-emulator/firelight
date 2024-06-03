import QtQuick
import QtQuick.Controls

Pane {
    property alias text: label.text

    background: Item {
    }

    Text {
        id: label
        color: "#ececec"
        font.family: Constants.regularFontFamily
        font.pointSize: 12
        font.weight: Font.DemiBold
    }
}