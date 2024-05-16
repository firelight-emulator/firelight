import QtQuick
import QtQuick.Controls
import Firelight 1.0

Pane {
    id: root

    required property string modName

    function show() {
        root.state = "shown"
    }

    function hide() {
        root.state = "hidden"
    }

    background: Rectangle {
        color: "white"
        radius: 4
    }

    parent: Overlay.overlay
    y: parent.height - root.height - 24

    width: 200
    height: 50

    state: "hidden"

    Text {
        anchors.centerIn: parent
        text: "Added " + modName + " to library"
        font.pointSize: 11
        font.family: Constants.regularFontFamily
        color: "#212020"
    }

    states: [
        State {
            name: "shown"
            PropertyChanges {
                target: root
                opacity: 1
                // y: parent.height - root.height - 20
            }
        },
        State {
            name: "hidden"
            PropertyChanges {
                target: root
                opacity: 0
                // y: parent.height
            }
        }
    ]
    transitions: [
        Transition {
            from: "hidden"
            to: "shown"
            NumberAnimation {
                properties: "opacity"
                duration: 200
                easing.type: Easing.InOutQuad
            }
        },
        Transition {
            from: "shown"
            to: "hidden"
            SequentialAnimation {
                PauseAnimation {
                    duration: 1000
                }
                NumberAnimation {
                    properties: "opacity"
                    duration: 200
                    easing.type: Easing.InOutQuad
                }
            }
        }
    ]
}