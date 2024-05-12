import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FirelightStyle 1.0

ColumnLayout {
    id: root

    Button {
        text: "toggle achievement"

        onClicked: {
            popup.open()
        }

        Popup {
            id: popup
            parent: Overlay.overlay
            x: parent.width / 2 - width / 2
            y: parent.height - height - 40
            width: 310
            height: 80
            modal: false

            closePolicy: Popup.NoAutoClose

            background: Rectangle {
                color: "#191919"
                radius: 8
                border.color: "#5c5c5c"
            }

            enter: Transition {
                ParallelAnimation {
                    NumberAnimation {
                        property: "scale"
                        from: 0.7
                        to: 1.0
                        duration: 220
                        easing.type: Easing.OutBack
                        easing.overshoot: 3.0
                    }
                    NumberAnimation {
                        property: "opacity"
                        from: 0
                        to: 1.0
                        duration: 220
                        easing.type: Easing.InOutQuad
                    }
                }

                ScriptAction {
                    script: {
                        popupAnimation.running = true
                    }
                }
            }

            exit: Transition {
                ParallelAnimation {
                    NumberAnimation {
                        property: "scale"
                        from: 1.0
                        to: 0.7
                        duration: 220
                        easing.type: Easing.InBack
                        easing.overshoot: 3.0
                    }
                    NumberAnimation {
                        property: "opacity"
                        from: 1.0
                        to: 0
                        duration: 220
                        easing.type: Easing.InOutQuad
                    }
                }
            }

            padding: 6

            SequentialAnimation {
                id: popupAnimation
                running: false

                PropertyAction {
                    target: unlockedText
                    property: "opacity"
                    value: 1.0
                }

                PropertyAction {
                    target: unlockedText
                    property: "visible"
                    value: true
                }

                PropertyAction {
                    target: row
                    property: "textVisible"
                    value: false
                }

                PauseAnimation {
                    duration: 1500
                }

                NumberAnimation {
                    target: unlockedText
                    property: "opacity"
                    from: 1.0
                    to: 0.0
                    duration: 220
                    easing.type: Easing.InOutQuad
                }

                PropertyAction {
                    target: unlockedText
                    property: "visible"
                    value: false
                }

                PropertyAction {
                    target: row
                    property: "textOpacity"
                    value: 0
                }

                PropertyAction {
                    target: row
                    property: "textVisible"
                    value: true
                }

                NumberAnimation {
                    target: row
                    property: "textOpacity"
                    from: 0.0
                    to: 1.0
                    duration: 220
                    easing.type: Easing.InOutQuad
                }

                PauseAnimation {
                    duration: 3000
                }

                PropertyAction {
                    target: popup
                    property: "visible"
                    value: false
                }
            }

            contentItem: Item {
                id: row
                property real textOpacity: 1.0
                property bool textVisible: false

                Image {
                    id: pic
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.bottom: parent.bottom
                    width: parent.height
                    fillMode: Image.PreserveAspectFit
                    source: "file:system/_img/achieve.png"
                }

                ColumnLayout {
                    anchors.top: parent.top
                    anchors.left: pic.right
                    anchors.leftMargin: 8
                    anchors.bottom: parent.bottom
                    anchors.right: parent.right
                    spacing: 4

                    Text {
                        id: unlockedText
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        text: "Achievement Unlocked!"
                        font.pointSize: 11
                        font.family: Constants.regularFontFamily
                        font.weight: Font.DemiBold
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        color: "white"
                    }

                    Text {
                        id: titleText
                        visible: row.textVisible
                        opacity: row.textOpacity
                        Layout.fillWidth: true
                        text: "Mighty Morphin' Shy Guys"
                        font.pointSize: 11
                        font.family: Constants.regularFontFamily
                        font.weight: Font.DemiBold
                        elide: Text.ElideRight
                        maximumLineCount: 1
                        color: "white"
                    }

                    Text {
                        id: descriptionText
                        visible: row.textVisible
                        opacity: row.textOpacity
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        text: "Defeat the groups of two Red, Pink, Yellow, Green, and Blue Shy Guys IN ORDER in the Blue Zone of Shy Guys Toybox"
                        font.pointSize: 10
                        font.family: Constants.regularFontFamily
                        wrapMode: Text.WordWrap
                        elide: Text.ElideRight
                        maximumLineCount: 2
                        verticalAlignment: Text.AlignVCenter
                        font.weight: Font.Normal
                        color: "#c1c1c1"
                    }
                }
            }
        }
    }

    Item {
        Layout.fillWidth: true
        Layout.fillHeight: true

    }
}