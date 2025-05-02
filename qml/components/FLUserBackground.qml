import QtQuick
import QtQuick.Effects

Rectangle {
    id: root

    property bool usingCustomBackground: false
    property var backgroundFile: ""
    property bool blur: false
    property color defaultColor: "black"

    property real blurAmount: root.blur ? 0.5 : 0

    color: defaultColor

    Behavior on blurAmount {
        NumberAnimation {
            easing.type: Easing.InOutQuad
            duration: 250
        }
    }

    layer.enabled: blurAmount !== 0
    layer.effect: MultiEffect {
        // enabled: root.blurAmount !== 0

        source: root
        anchors.fill: root
        blurEnabled: true
        blurMultiplier: 0
        blurMax: 64
        blur: root.blurAmount
    }


    Item {
        anchors.fill: parent
        visible: root.usingCustomBackground && root.backgroundFile !== ""
        AnimatedImage {
            id: customBackground
            source: root.backgroundFile
            fillMode: Image.PreserveAspectCrop
            anchors.fill: parent
            playing: true

            cache: false

            onSourceChanged: {
                playing = true
            }
        }

        Rectangle {
            anchors.fill: parent
            color: "black"
            opacity: 0.65
        }

        Rectangle {
            anchors.topMargin: -70
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 200
            gradient: Gradient {
                GradientStop {
                    position: 0.0; color: "black"
                }
                GradientStop {
                    position: 1.0; color: "transparent"
                }
            }
        }

        Rectangle {
            anchors.bottomMargin: -70
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            height: 200
            gradient: Gradient {
                GradientStop {
                    position: 1.0; color: "black"
                }
                GradientStop {
                    position: 0.0; color: "transparent"
                }
            }
        }

    }
}
