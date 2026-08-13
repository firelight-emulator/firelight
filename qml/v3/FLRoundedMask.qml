import QtQuick

// TODO
// SDF rounded-corner mask. Set as a layer.effect to clip an item's content to
// rounded corners; it sizes itself to the layered item
//
//   Item {
//       layer.enabled: true
//       layer.smooth: true
//       layer.effect: FLRoundedMask { radius: AppStyle.radiusLg }
//   }
//
// Each corner can be given its own radius, and follows radius until it is:
//
//   FLRoundedMask { radius: AppStyle.radiusLg; bottomLeftRadius: 0; bottomRightRadius: 0 }
ShaderEffect {
    property real radius: 0

    // TODO
    // Named to match Rectangle's own per-corner properties, and defaulted the same way: each
    // reads as radius until something assigns it
    property real topLeftRadius: radius
    property real topRightRadius: radius
    property real bottomLeftRadius: radius
    property real bottomRightRadius: radius

    property real topLeftPx: topLeftRadius
    property real topRightPx: topRightRadius
    property real bottomLeftPx: bottomLeftRadius
    property real bottomRightPx: bottomRightRadius
    property real widthPx: width
    property real heightPx: height
    fragmentShader: "qrc:/qt/qml/QMLFirelight/shaders/rounded_image.frag.qsb"
}
