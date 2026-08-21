import QtQuick
import QtQuick.Effects

// App background. Renders a solid color, a two-color gradient, or an image
// (animated ok), with an optional blur and dim. Driven by AppearanceSettings via
// its host (see Main4)
Rectangle {
    id: root

    // "solid" | "gradient" | "image"
    property string mode: "gradient"

    property color color1: defaultColor
    property color color2: Qt.lighter(defaultColor, 1.5)

    property bool usingCustomBackground: false
    property var backgroundFile: ""

    property bool blur: false
    property color defaultColor: "black"

    // 0..1. Legacy `blur` bool maps to a default level; hosts can set it directly
    property real blurAmount: root.blur ? 0.6 : 0
    property real dimAmount: 0

    color: root.color1
    // Solid mode collapses the two stops to the same color; gradient mode uses
    // both. Image mode is covered by the AnimatedImage below
    gradient: Gradient {
        GradientStop {
            position: 0.0
            color: root.color1
        }
        GradientStop {
            position: 1.0
            color: root.mode === "gradient" ? root.color2 : root.color1
        }
    }

    Behavior on blurAmount {
        NumberAnimation {
            easing.type: Easing.InOutQuad
            duration: 250
        }
    }

    layer.enabled: blurAmount !== 0
    layer.effect: MultiEffect {
        source: root
        anchors.fill: root
        blurEnabled: true
        blurMultiplier: 0
        blurMax: 64
        blur: root.blurAmount
        autoPaddingEnabled: false
    }

    Item {
        anchors.fill: parent
        visible: root.mode === "image" && root.backgroundFile !== ""

        AnimatedImage {
            id: customBackground
            source: root.backgroundFile
            fillMode: Image.PreserveAspectCrop
            anchors.fill: parent
            playing: true

            cache: false

            onSourceChanged: {
                playing = true;
            }
        }

        Rectangle {
            anchors.topMargin: -70
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 200
            gradient: Gradient {
                GradientStop {
                    position: 0.0
                    color: "black"
                }
                GradientStop {
                    position: 1.0
                    color: "transparent"
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
                    position: 1.0
                    color: "black"
                }
                GradientStop {
                    position: 0.0
                    color: "transparent"
                }
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "black"
        opacity: root.dimAmount
    }
}
