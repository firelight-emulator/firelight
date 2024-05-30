import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Popup {
    id: root

    property string imageUrl
    property string title
    property int numEarnedAchievements
    property int totalAchievements
    property bool hardcoreMode

    function openWith(imageUrl, title, numEarnedAchievements, totalAchievements, hardcoreMode) {
        this.imageUrl = imageUrl
        this.title = title
        this.numEarnedAchievements = numEarnedAchievements
        this.totalAchievements = totalAchievements
        this.hardcoreMode = hardcoreMode

        open()
    }

    parent: Overlay.overlay
    x: 0
    y: parent.height - height
    // width: parent.width
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

    background: Rectangle {
        color: "#191919"
    }

    padding: 16

    contentItem: Item {
        implicitWidth: pic.implicitWidth + 8 + col.implicitWidth
        Image {
            id: pic
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom

            source: root.imageUrl
            fillMode: Image.PreserveAspectFit
        }

        ColumnLayout {
            id: col
            anchors.left: pic.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.leftMargin: 8

            Text {
                text: root.title
                font.pointSize: 16
                font.family: Constants.regularFontFamily
                font.weight: Font.DemiBold
                color: "white"
            }

            Text {
                text: root.numEarnedAchievements + " of " + root.totalAchievements + " achievements earned"
                font.pointSize: 12
                font.family: Constants.regularFontFamily
                // font.weight: Font.Light
                color: "#bbbbbb"
            }

        }
    }

    enter: Transition {
        ParallelAnimation {
            PropertyAnimation {
                property: "x"
                from: -15
                to: 0
                duration: 340
                easing.type: Easing.InOutQuad
            }
            PropertyAnimation {
                property: "opacity"
                from: 0
                to: 1.0
                duration: 340
                easing.type: Easing.InOutQuad
            }
        }
    }

    exit: Transition {
        ParallelAnimation {
            PropertyAnimation {
                property: "x"
                from: 0
                to: -15
                duration: 340
                easing.type: Easing.InOutQuad
            }
            PropertyAnimation {
                property: "opacity"
                from: 1.0
                to: 0
                duration: 340
                easing.type: Easing.InOutQuad
            }
        }
    }
}