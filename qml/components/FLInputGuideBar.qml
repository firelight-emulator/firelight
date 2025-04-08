import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import Firelight 1.0

Pane {
    id: root
    verticalPadding: 10
    horizontalPadding: 16

    focus: false
    implicitHeight: 60

    required property Item target
    property var inputHints: []

    property bool hidden: false

    onHiddenChanged: {
        if (hidden) {
            hideAnimation.start()
        } else {
            showAnimation.start()
        }
    }

    onTargetChanged: {
        listView.model.clear()
        if (target === null) {
            return
        }
        // target.onFocusChanged.connect(function() {
        //     if (target.focus) {
        //         root.close()
        //     }
        // })

        if (target.hasOwnProperty('inputHints')) {
            for (var i = 0; i < target.inputHints.length; i++) {
                listView.model.append({label: target.inputHints[i].label,
                    key: target.inputHints[i].input})
            }
        }
    }

    background: Item {
    }

    PropertyAnimation {
        id: hideAnimation
        target: actualContent
        property: "y"
        to: root.height + 1
        duration: 250
        easing.type: Easing.InOutQuad
    }

    PropertyAnimation {
        id: showAnimation
        target: actualContent
        property: "y"
        to: 0
        duration: 250
        easing.type: Easing.InOutQuad
    }

    Item {
        id: actualContent
        width: parent.width
        height: parent.height
        Rectangle {
            y: -root.padding + height
            width: parent.width
            height: 1
            color: "white"
            opacity: 0.4
        }

        Image {
            id: me
            source: "file:whatever.svg"
            height: parent.height
            width: parent.height
            anchors.left: parent.left
            anchors.leftMargin: 16
            sourceSize.width: parent.height
            fillMode: Image.PreserveAspectFit
            layer.enabled: true
            layer.effect: MultiEffect {
                source: me
                anchors.fill: me
                colorization: 1.0
                colorizationColor: Qt.rgba(1.0, 1.0, 1.0, 1.0)
            }
        }
        ListView {
            id: listView
            spacing: 12
            anchors.fill: parent
            model: ListModel {}
            orientation: ListView.Horizontal
            layoutDirection: Qt.RightToLeft
            interactive: false
            delegate: Button {
                height: ListView.view.height

                focusPolicy: Qt.NoFocus
                verticalPadding: 8
                horizontalPadding: 12
                HoverHandler {
                    id: hover
                    cursorShape: Qt.PointingHandCursor
                }
                background: Rectangle {
                    color: hover.hovered ? "white" : "transparent"
                    opacity: hover.hovered ? 0.2 : 0
                    radius: height / 2
                }
                onClicked: function() {
                    EventEmitter.emitKeyEvent(model.key)
                }

                contentItem: RowLayout {
                    spacing: 10

                    Rectangle {
                        height: 20
                        width: 20
                        Layout.alignment: Qt.AlignVCenter
                        radius: height / 2
                        color: "white"
                    }

                    Text {
                        text: model.label
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        color: "white"
                        // font.weight: Font.DemiBold
                        font.pixelSize: 16
                        font.family: Constants.regularFontFamily
                    }
                }
            }
        }
    }
}