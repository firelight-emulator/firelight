import QtQuick
import QtQuick.Controls
import QtQuick.Layouts 1.0
import Firelight 1.0

FocusScope {
    id: root

    objectName: "mainContent"

    property bool showNavigationBar: true
    property bool gameRunning: false

    signal powerButtonPressed()

    states: [
        State {
            name: "hidden"
            when: !root.showNavigationBar
            PropertyChanges {
                target: navigationBar
                anchors.leftMargin: -navigationBar.implicitWidth
            }
        },
        State {
            name: "visible"
            when: root.showNavigationBar
            PropertyChanges {
                target: navigationBar
                anchors.leftMargin: 0
            }
        }
    ]

    transitions: [
        Transition {
            from: "hidden"
            to: "visible"
            reversible: true
            NumberAnimation {
                target: navigationBar
                property: "anchors.leftMargin"
                duration: 160
                easing.type: Easing.InOutQuad
            }
        }
    ]

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
            root.powerButtonPressed()
        }
    }

    Pane {
        id: content
        objectName: "contentPane"
        anchors.left: navigationBar.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right

        property string title: "Firelight"

        padding: 16
        background: Item {}
        contentItem: ColumnLayout {
            spacing: 16
            RowLayout {
                Layout.fillWidth: true
                Layout.maximumHeight: 42
                Layout.minimumHeight: 42

                spacing: 12

                Button {
                    background: Rectangle {
                        color: "white"
                        opacity: parent.pressed ? 0.16 : 0.1
                        radius: 4
                        visible: theHoverHandler.hovered && parent.enabled
                    }
                    HoverHandler {
                        id: theHoverHandler
                        cursorShape: Qt.PointingHandCursor
                    }
                    padding: 4
                    Layout.fillHeight: true
                    Layout.preferredWidth: parent.height
                    contentItem: FLIcon {
                        icon: "arrow-back"
                        size: height
                        color: parent.enabled ? "white" : "#727272"
                    }

                    enabled: Router.historySize > 1

                    onClicked: {
                        Router.goBack()
                        // contentStack.popCurrentItem()
                    }
                }

                Text {
                    text: content.title
                    // Layout.fillWidth: true
                    // Layout.fillHeight: true
                    color: "white"
                    font.family: Constants.regularFontFamily
                    font.weight: Font.DemiBold
                    font.pixelSize: 24
                    verticalAlignment: Qt.AlignVCenter
                }

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.horizontalStretchFactor: 1
                }
            }

            StackView {
                id: contentStack
                Layout.fillWidth: true
                Layout.fillHeight: true

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
                    }}
            }
        }
    }

}
