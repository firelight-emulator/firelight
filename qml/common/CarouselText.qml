import QtQuick
import QtQuick.Controls


Text {
    id: root

    property bool hovered: false
    property real originalX: 0
    property real scrollSpeed: 70
    property real scrollDistance: 0
    property real scrollDuration: 0

    Component.onCompleted: {
        originalX = x
        updateScrollDistance()
    }

    onWidthChanged: {
        updateScrollDistance()
    }

    onContentWidthChanged: {
        updateScrollDistance()
    }

    function updateScrollDistance() {
        scrollDistance = contentWidth - originalX - width
        if (scrollDistance < 0) {
            scrollDistance = 0
        }
        scrollDuration = scrollDistance / scrollSpeed * 1000
    }

    maximumLineCount: 1

    states: [
        State {
            name: "hovered"
            when: hovered
        }
    ]

    transitions: [
        Transition {
            to: "hovered"
            ScriptAction {
                script: {
                    root.originalX = root.x
                    if (root.contentWidth > root.width) {
                        scrollAnimation.running = true
                    }
                }
            }
        },
        Transition {
            from: "hovered"
            ScriptAction {
                script: {
                    scrollAnimation.running = false
                    root.x = root.originalX
                }
            }
        }
    ]

    SequentialAnimation {
        id: scrollAnimation
        PauseAnimation {
            duration: 1000
        }
        NumberAnimation {
            target: root
            property: "x"
            to: -(scrollDistance)
            duration: scrollDuration
        }
    }
}