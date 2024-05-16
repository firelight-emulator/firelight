import QtQuick
import QtQuick.Controls
import Firelight 1.0

Pane {
    id: root
    background: Rectangle {
        color: "white"
        radius: 4
    }

    parent: Overlay.overlay
    y: parent.height - root.height - 24

    width: 200
    height: 50

    Text {
        anchors.centerIn: parent
        text: library_manager.scanning ? "Scanning..." : "Library scan complete"
        font.pointSize: 11
        font.family: Constants.regularFontFamily
        color: "#212020"
    }

    states: [
        State {
            name: "shown"
            when: library_manager.scanning
            PropertyChanges {
                target: root
                opacity: 1
                // y: parent.height - root.height - 20
            }
        },
        State {
            name: "hidden"
            when: !library_manager.scanning
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