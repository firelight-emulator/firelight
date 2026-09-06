import QtQuick
import QtQuick.Controls
import Firelight 1.0

FLIconButton {
    id: control

    required property bool showButton

    property real _initialX: 0

    Component.onCompleted: {
        control._initialX = control.x;
    }

    states: [
        State {
            name: "collapsed"
            when: !Router.canGoBack || !InputMethodManager.usingMouse || !control.showButton
            PropertyChanges {
                target: control
                opacity: 0
                x: _initialX - width
            }
        },
        State {
            name: "expanded"
            when: Router.canGoBack && InputMethodManager.usingMouse && control.showButton
            PropertyChanges {
                target: control
                opacity: 1
                x: _initialX
            }
        }
    ]

    transitions: Transition {
        NumberAnimation {
            properties: "x,opacity"
            duration: AppStyle.durationSlow
            easing.type: AppStyle.easingStandard
        }
    }

    iconName: "chevron-back"
    tooltipText: "Back"
    filled: false
    compact: false
    onClicked: Router.back()
}