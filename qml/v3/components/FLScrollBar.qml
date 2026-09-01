import QtQuick
import QtQuick.Controls
import Firelight 1.0

ScrollBar {
    id: control

    policy: ScrollBar.AlwaysOn
    width: AppStyle.spacingSm

    contentItem: Rectangle {
        color: control.pressed ? Theme.textMuted : Theme.borderStrong
        radius: width / 2
        width: AppStyle.spacingSm

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
                    duration: AppStyle.durationBase
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
