import QtQuick
import QtQuick.Controls
import QtQuick.Layouts 1.0
import Firelight 1.0

FocusScope {
    id: root

    objectName: "mainContent"

    property bool showNavigationBar: true
    property bool gameRunning: false

    function goToContent(title, page, args, transition) {
        content.title = title
        contentStack.replaceCurrentItem(page, args, transition)

    }

    LeftNavigationBar {
        id: navigationBar
        objectName: "leftNavigationBar"
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        showQuickMenuButton: root.gameRunning
        onPowerButtonClicked: {
            quitDialog.open()
        }
    }

    Pane {
        id: content
        objectName: "contentPane"
        anchors.left: navigationBar.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right

        KeyNavigation.left: navigationBar

        focus: true

        property string title: "Firelight"

        padding: 16
        background: Item {}
        contentItem: StackView {
            id: contentStack

            focus: true

            initialItem: Text {
                text: "Loading..."
                anchors.fill: parent
                font.pixelSize: 22
                font.family: Constants.regularFontFamily
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                color: "#FFFFFF"
            }

            Component.onCompleted: {
                Router.navigateTo("/library")
            }

            pushEnter: Transition {
                NumberAnimation {
                    property: "opacity"
                    from: 0
                    to: 1
                    duration: 200
                    easing.type: Easing.InOutQuad
                }
                NumberAnimation {
                    property: "scale"
                    from: 1.03
                    to: 1
                    duration: 200
                    easing.type: Easing.InOutQuad
                }
            }
            pushExit: Transition {
                NumberAnimation {
                    property: "opacity"
                    from: 1
                    to: 0
                    duration: 200
                    easing.type: Easing.InOutQuad
                }
                NumberAnimation {
                    property: "scale"
                    from: 1
                    to: 0.97
                    duration: 200
                    easing.type: Easing.InOutQuad
                }
            }
            popEnter: Transition {}
            popExit: Transition {}
            replaceEnter: Transition {
                NumberAnimation {
                    property: "opacity"
                    from: 0
                    to: 1
                    duration: 200
                    easing.type: Easing.InOutQuad
                }
                NumberAnimation {
                    property: "x"
                    from: -20
                    to: 0
                    duration: 200
                    easing.type: Easing.InOutQuad
                }
            }
            replaceExit: Transition {
                NumberAnimation {
                    property: "opacity"
                    from: 1
                    to: 0
                    duration: 200
                    easing.type: Easing.InOutQuad
                }
                NumberAnimation {
                    property: "x"
                    from: 0
                    to: -20
                    duration: 200
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }

    FirelightDialog {
        id: quitDialog
        text: "Are you sure you want to quit Firelight?"
        onAccepted: {
            Qt.quit()
        }
    }

}
