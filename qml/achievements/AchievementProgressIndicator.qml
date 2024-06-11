import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Popup {
    id: root
    parent: Overlay.overlay
    x: 40
    y: parent.height - height - 40
    width: 240
    height: 80
    modal: false

    property string imageUrl: ""
    property string title: "dev messed up"
    property string description: "dev messed up"
    property int current: 0
    property int desired: 0

    function openWith(imageUrl, title, description, current, desired) {
        if (!achievement_manager.progressNotificationsEnabled) {
            return
        }
        root.imageUrl = imageUrl
        root.title = title
        root.description = description
        root.current = current
        root.desired = desired
        root.open()
        if (bounceAnimation.running) {
            bounceAnimation.restart()
        } else {
            bounceAnimation.running = true
        }
        timer.restart()
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
        radius: 8
        border.color: "#404040"
    }

    enter: Transition {
        ParallelAnimation {
            NumberAnimation {
                property: "scale"
                from: 0.7
                to: 1.0
                duration: 100
                easing.type: Easing.OutBack
                easing.overshoot: 2.0
            }
            NumberAnimation {
                property: "opacity"
                from: 0
                to: 1.0
                duration: 100
                easing.type: Easing.InOutQuad
            }
        }

        ScriptAction {
            script: {
                timer.start()
            }
        }
    }

    exit: Transition {
        ParallelAnimation {
            NumberAnimation {
                property: "opacity"
                from: 1.0
                to: 0
                duration: 200
                easing.type: Easing.InOutQuad
            }
        }
    }

    padding: 6

    contentItem: Item {
        Image {
            id: picasdf
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            width: parent.height
            fillMode: Image.PreserveAspectFit
            source: root.imageUrl
        }

        ColumnLayout {
            anchors.left: picasdf.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            spacing: 4

            Text {
                Layout.fillWidth: true
                text: root.description
                font.pointSize: 10
                font.family: Constants.regularFontFamily
                wrapMode: Text.WordWrap
                elide: Text.ElideRight
                leftPadding: 8
                maximumLineCount: 2
                verticalAlignment: Text.AlignVCenter
                font.weight: Font.Normal
                color: "#c1c1c1"
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Row {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    Text {
                        id: first
                        text: root.current
                        font.family: Constants.regularFontFamily
                        font.pointSize: 16
                        font.weight: Font.DemiBold
                        color: "white"
                        verticalAlignment: Text.AlignBottom
                        horizontalAlignment: Text.AlignRight
                        width: parent.width / 2 - 8

                        Behavior on y {
                            NumberAnimation {
                                duration: 150 // Duration of the animation in milliseconds
                                easing.type: Easing.OutBounce // Easing function for bounce effect
                            }
                        }

                        SequentialAnimation {
                            id: bounceAnimation
                            running: false
                            PropertyAnimation {
                                target: first
                                property: "y"
                                from: 0
                                to: -4
                                duration: 100
                                easing.type: Easing.InOutExpo
                            }
                            PropertyAnimation {
                                target: first
                                property: "y"
                                to: 0
                                duration: 100
                                easing.type: Easing.InOutExpo
                            }
                        }
                    }

                    Text {
                        text: " /"
                        height: first.height
                        font.family: Constants.regularFontFamily
                        font.pointSize: 15
                        color: "#aaaaaa"
                        verticalAlignment: Text.AlignBottom
                    }

                    Text {
                        text: root.desired
                        height: first.height
                        font.family: Constants.regularFontFamily
                        font.pointSize: 12
                        color: "#aaaaaa"
                        verticalAlignment: Text.AlignBottom
                    }
                }

            }
        }

    }
}