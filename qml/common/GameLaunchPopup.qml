import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Popup {
    id: root
    parent: Overlay.overlay
    x: 0
    y: parent.height - height
    width: parent.width
    height: 100

    onOpened: {
        if (!timer.running) {
            timer.start()
        }
    }

    onClosed: {
        timer.stop()
    }

    Timer {
        id: timer
        interval: 3000
        running: false
        repeat: false
        onTriggered: {
            root.close()
        }
    }

    closePolicy: Popup.NoAutoClose

    background: Item {}

    padding: 16

    contentItem: Item {
        Image {
            id: pic
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom

            source: "https://media.retroachievements.org/Images/047942.png"
            fillMode: Image.PreserveAspectFit
        }

        ColumnLayout {
            anchors.left: pic.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.leftMargin: 8

            Text {
                text: "Super Mario 64"
                font.pointSize: 16
                font.family: Constants.regularFontFamily
                font.weight: Font.DemiBold
                color: "white"
            }

            Text {
                text: "You've earned 9 of 119 achievements"
                font.pointSize: 12
                font.family: Constants.regularFontFamily
                font.weight: Font.Light
                color: "#bbbbbb"
            }

        }
    }

    enter: Transition {
        ParallelAnimation {
            PropertyAnimation {
                property: "x"
                from: -30
                to: 0
                duration: 500
                easing.type: Easing.InOutQuad
            }
            PropertyAnimation {
                property: "opacity"
                from: 0
                to: 1.0
                duration: 500
                easing.type: Easing.InOutQuad
            }
        }
    }

    exit: Transition {
        ParallelAnimation {
            PropertyAnimation {
                property: "x"
                from: 0
                to: -30
                duration: 500
                easing.type: Easing.InOutQuad
            }
            PropertyAnimation {
                property: "opacity"
                from: 1.0
                to: 0
                duration: 500
                easing.type: Easing.InOutQuad
            }
        }
    }
}