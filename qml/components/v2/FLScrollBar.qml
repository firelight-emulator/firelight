import QtQuick
import QtQuick.Controls

ScrollBar {
    id: control

    contentItem: Rectangle {
        color: control.pressed ? Theme.textMuted : Theme.borderStrong

        opacity: 0.0

        states: State {
            name: "active"
            when: control.policy === ScrollBar.AlwaysOn || (control.active && control.size < 1.0)
            PropertyChanges {
                control.contentItem.opacity: 0.75
            }
        }

        transitions: Transition {
            from: "active"
            SequentialAnimation {
                PauseAnimation {
                    duration: 450
                }
                NumberAnimation {
                    target: control.contentItem
                    duration: 200
                    property: "opacity"
                    to: 0.0
                }
            }
        }
    }

    background: Rectangle {
        color: "transparent"
    }
}
