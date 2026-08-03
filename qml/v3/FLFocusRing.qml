import QtQuick

// TODO
// Procedural rotating two-color focus ring. `phase` (0..1) spins the gradient;
// `radiusPx`/`ringWidthPx` size the band. Drawn by FLFocusHighlight
ShaderEffect {
    id: control

    property real radiusPx: 0
    property real ringWidthPx: 2
    property real padPx: 8
    property real fillPx: 0
    property color fillColor: "transparent"
    property real phase: 0
    property color colorA: "#89c5f9"
    property color colorB: "#a78bfa"
    property real widthPx: width
    property real heightPx: height

    blending: true
    fragmentShader: "qrc:/qt/qml/QMLFirelight/shaders/focus_ring.frag.qsb"
}
