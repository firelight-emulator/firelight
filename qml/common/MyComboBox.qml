import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ComboBox {
    id: sortBox

    contentItem: Pane {
        background: Item {
        }
        padding: 0

        MouseArea {
            anchors.fill: parent
            onClicked: {
                sortBox.popup.visible = !sortBox.popup.visible
            }
        }

        Text {
            topPadding: 8
            bottomPadding: 8
            leftPadding: 10
            rightPadding: 10
            anchors.fill: parent
            text: sortBox.currentText
            color: "#ececec"
            font.family: Constants.regularFontFamily
            verticalAlignment: Text.AlignVCenter
            font.pointSize: 11
            font.weight: Font.DemiBold
        }
    }

    background: Rectangle {
        implicitWidth: 140
        implicitHeight: 40
        color: "#32363a"
        radius: 12
    }

    palette.buttonText: "white"
}