import QtQuick
import QtQuick.Controls
import Firelight 1.0

FocusScope {
    id: root

    property var currentIndex: 0
    property var currentMenuItem: menuItems[currentIndex]
    property bool landscape: parent.width > parent.height
    property var menuItems: []
    property var pages: []

    SwipeView {
        id: portraitContent

        anchors.fill: parent
        interactive: false
        visible: !root.landscape

        ListView {
            id: menu2

            property int lastIndex: 0
            property bool movingDown: true

            visible: SwipeView.isCurrentItem
            // KeyNavigation.right: stack
            // anchors.bottom: parent.bottom
            // anchors.left: parent.left
            // anchors.top: parent.top
            currentIndex: root.currentIndex
            focus: true
            interactive: false
            keyNavigationEnabled: true
            model: root.menuItems

            delegate: FirelightMenuItem {
                required property var index
                required property var modelData
                property bool showGlobalCursor: true

                checked: ListView.isCurrentItem
                focus: ListView.isCurrentItem
                height: 50
                labelText: modelData
                width: ListView.view.width

                onClicked: function () {
                    portraitStack.replaceCurrentItem(root.pages[index]);
                    portraitContent.incrementCurrentIndex();
                }

                onToggled: {
                    if (checked) {
                        ListView.view.currentIndex = index;
                    }
                }
            }
        }
        StackView {
            id: portraitStack
            visible: SwipeView.isCurrentItem

            initialItem: root.pages[root.currentIndex]

            popEnter: Transition {
            }
            popExit: Transition {
            }
            pushEnter: Transition {
            }
            pushExit: Transition {
            }
            replaceEnter: Transition {
            }
            replaceExit: Transition {
            }
            //

            // Keys.onBackPressed: {
            //     menu.forceActiveFocus();
            // }
            // Keys.onEscapePressed: {
            //     menu.forceActiveFocus();
            // }

            //
            // anchors.bottom: parent.bottom
            // anchors.left: menu.right
            // anchors.right: parent.right
            // anchors.top: parent.top
            Keys.onEscapePressed: {
                portraitContent.decrementCurrentIndex();
            }
        }
    }
    FocusScope {
        id: landscapeContent

        anchors.fill: parent
        visible: root.landscape

        ListView {
            id: menu

            property int lastIndex: 0
            property bool movingDown: true

            KeyNavigation.right: stack
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.top: parent.top
            currentIndex: root.currentIndex
            focus: true
            interactive: false
            keyNavigationEnabled: true
            model: root.menuItems
            width: 300

            delegate: FirelightMenuItem {
                required property var index
                required property var modelData
                property bool showGlobalCursor: true

                checked: ListView.isCurrentItem
                focus: ListView.isCurrentItem
                height: 50
                labelText: modelData
                width: ListView.view.width

                onClicked: function () {
                    stack.forceActiveFocus();
                }
                onToggled: {
                    if (checked) {
                        ListView.view.currentIndex = index;
                    }
                }
            }

            onCurrentIndexChanged: function () {
                if (currentIndex === lastIndex) {
                    return;
                }
                movingDown = currentIndex > lastIndex;
                lastIndex = currentIndex;
                stack.replaceCurrentItem(root.pages[currentIndex]);
            }
        }
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

            Keys.onBackPressed: {
                menu.forceActiveFocus();
            }
            Keys.onEscapePressed: {
                menu.forceActiveFocus();
            }
        }
    }
}

