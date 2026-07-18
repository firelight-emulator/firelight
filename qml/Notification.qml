import QtQuick
import QtQuick.Controls

Rectangle {
    id: root

    parent: Overlay.overlay

    x: parent.x + 30
    y: parent.height - height - 30

    color: "grey"
    width: icon.width + topLabel.width
    height: 40
    radius: 50

    Rectangle {
        id: icon

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 0

        color: "blue"
    }

    Item {
        id: topLabel

        anchors.left: icon.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.verticalCenter
        width: 250
        // color: "orange"

        Text {
            id: topLabelText
            anchors.fill: parent
            // anchors.leftMargin: 10

            text: "P1 controller connected"
            font.pixelSize: AppStyle.fontSizeMedium
            color: "white"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter

        }
    }

    Item {
        id: bottomLabel

        anchors.left: icon.right
        anchors.right: parent.right
        anchors.top: topLabel.bottom
        anchors.bottom: parent.bottom
        Text {
            anchors.fill: parent
            // anchors.leftMargin: 10

            text: "ain't that neat"
            font.pixelSize: AppStyle.fontSizeSmall
            color: "white"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter

        }
    }

}
