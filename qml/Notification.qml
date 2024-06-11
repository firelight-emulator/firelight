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
            font.pointSize: 13
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
        // radius: parent.radius
        // color: "green"
        Text {
            anchors.fill: parent
            // anchors.leftMargin: 10

            text: "ain't that neat"
            font.pointSize: 8
            color: "white"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter

        }
    }

    // BusyIndicator {
    //     id: spinner
    //     width: 40
    //     height: 40
    //     anchors.verticalCenter: parent.verticalCenter
    //     anchors.left: parent.left
    //     anchors.leftMargin: 10
    //     // running: library_manager.scanning
    //     running: true
    // }
    //
    // Text {
    //     id: label
    //     text: "Updating your Library..."
    //     font.pointSize: 12
    //     color: "red"
    //     anchors.left: spinner.right
    //     anchors.leftMargin: 15
    //     anchors.verticalCenter: parent.verticalCenter
    //     horizontalAlignment: Text.AlignHCenter
    //     verticalAlignment: Text.AlignVCenter
    // }
}