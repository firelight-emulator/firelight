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
ShaderEffect {
    property real radius: 0
    property real radiusPx: radius
    property real widthPx: width
    property real heightPx: height
    fragmentShader: "qrc:/qt/qml/QMLFirelight/shaders/rounded_image.frag.qsb"
}
