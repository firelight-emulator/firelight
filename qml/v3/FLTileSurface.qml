import QtQuick

// TODO
// SDF rounded-corner mask that also bevels the edges: a rim of light on the
// top-facing edges and shade on the bottom-facing edges. Set as a layer.effect
// to clip an item to rounded corners with a 3D sheen
ShaderEffect {
    property real radius: 0
    property real radiusPx: radius
    property real widthPx: width
    property real heightPx: height
    property real topLight: 0
    property real bottomShade: 0
    property real bevelFrac: 0.06

    fragmentShader: "qrc:/qt/qml/QMLFirelight/shaders/tile_surface.frag.qsb"
}
