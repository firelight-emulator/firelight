import QtQuick

// TODO
// Rounded-corner mask whose edge fades inward over `feather` pixels instead of
// ending, so the content reads as lifting off the surface rather than being cut
// out of it. At feather 0 it is the crisp mask, so it can stay attached across an
// animation that starts and ends sharp
//
//   Item {
//       layer.enabled: true
//       layer.smooth: true
//       layer.effect: FLFeatheredMask {
//           radius: AppStyle.radiusLg
//           feather: 12
//       }
//   }
ShaderEffect {
    property real radius: 0

    // How far the fade reaches in from the edge, in item pixels. Scales with the
    // item, so a growing item's edge softens in proportion
    property real feather: 0

    // How far in from the edge the fill fades to clear; 0 keeps the whole shape. Half the shorter side
    // fades all the way to the middle
    property real thickness: 0

    property real radiusPx: radius
    property real featherPx: feather
    property real thicknessPx: thickness
    property real widthPx: width
    property real heightPx: height

    fragmentShader: "qrc:/qt/qml/QMLFirelight/shaders/feathered_edge.frag.qsb"
}
