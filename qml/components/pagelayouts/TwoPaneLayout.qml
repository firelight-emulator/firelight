import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

FocusScope {
    id: root
    property bool landscape: width > height
    default property list<TwoPaneMenuItem> menuItems

    StackView {
        id: stack

        anchors.bottom: parent.bottom
        anchors.left: menu.right
        anchors.right: parent.right
        anchors.top: parent.top
        initialItem: root.pages[root.currentIndex]
        visible: root.landscape

        popEnter: Transition {
        }
        popExit: Transition {
        }
        pushEnter: Transition {
        }
        pushExit: Transition {
        }
        replaceEnter: Transition {
            NumberAnimation {
                duration: 200
                from: 0.0
                property: "opacity"
                to: 1.0
            }
            NumberAnimation {
                duration: 200
                easing.type: Easing.InOutQuad
                from: 30 * (menu.movingDown ? 1 : -1)
                property: "y"
                to: 0
            }
        }
        replaceExit: Transition {
            NumberAnimation {
                duration: 20
                from: 1.0
                property: "opacity"
                to: 0.0
            }
            NumberAnimation {
                duration: 200
                easing.type: Easing.InOutQuad
                from: 0
                property: "y"
                to: 30 * (menu.movingDown ? -1 : 1)
            }
        }
    }
}
