// TODO: NEEDS REVIEW
import QtQuick

// TODO
// Procedural rotating two-color focus ring. `phase` (0..1) spins the gradient;
// `radiusPx`/`ringWidthPx` size the band. Drawn by FLFocusHighlight
//
// Each corner can be given its own radius, and follows radiusPx until it is:
//
//   FLFocusRing { radiusPx: 12; bottomLeftPx: 0; bottomRightPx: 0 }
ShaderEffect {
    id: control

    property real radiusPx: 0

    // TODO
    // Named for the corners they round, and defaulted the same way the mask's are: each reads as
    // radiusPx until something assigns it
    property real topLeftPx: radiusPx
    property real topRightPx: radiusPx
    property real bottomLeftPx: radiusPx
    property real bottomRightPx: radiusPx

    property real ringWidthPx: 2
    property real padPx: 8
    property real fillPx: 0
    property color fillColor: "transparent"
    property real phase: 0
    property color colorA: "#89c5f9"
    property color colorB: "#a78bfa"
    property color colorC: "#433173"
    property color colorD: "#8a8ade"
    property real widthPx: width
    property real heightPx: height

    blending: true
    fragmentShader: "qrc:/qt/qml/QMLFirelight/shaders/focus_ring.frag.qsb"
}
