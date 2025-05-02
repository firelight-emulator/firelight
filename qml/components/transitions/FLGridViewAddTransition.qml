import QtQuick
import QtQuick.Controls
import Firelight 1.0

Transition {
    id: root
    SequentialAnimation {
        PropertyAction {
            property: "opacity"
            value: 0
        }
        ParallelAnimation {
            PropertyAnimation {
                property: "opacity"
                from: 0
                to: 1
                duration: 200
                easing.type: Easing.InOutQuad
            }
            PropertyAnimation {
                property: "y"
                from: root.ViewTransition.destination.y + 20
                to: root.ViewTransition.destination.y
                duration: 200
                easing.type: Easing.InOutQuad
            }
        }
    }
}