import QtQuick

// Reusable crisp icon backed by the Material Symbols Rounded variable font.
// Usage: Icon { name: "settings"; size: 22; color: Theme.textPrimary }
// Renders a font glyph (not a raster/SVG), so it is sharp at any size and DPR and
// is colored simply via `color`. `name` is looked up in the MaterialSymbols singleton.
Text {
    id: root

    // Semantic icon name (see MaterialSymbols.qml), e.g. "settings", "home", "arrow-back".
    property string name

    // Raw glyph passthrough for callers that already hold a codepoint (e.g. a dynamic
    // `iconCode`). When set, it overrides the `name` lookup.
    property string glyph: ""

    // Icon size in device-independent px (drives font.pixelSize and the opsz axis).
    property int size: 24

    // Material Symbols variable-font axes.
    property bool filled: true
    property int weight: Font.Normal
    property int grade: 0

    text: root.glyph.length > 0 ? root.glyph : MaterialSymbols.glyph(root.name)
    color: Theme.textPrimary

    font.family: Constants.symbolFontFamily
    font.pixelSize: root.size
    // Drive the optical-size axis to the rendered size (font range is 20-48) so the
    // glyph outline is optically correct; expose fill/weight/grade for variants.
    font.variableAxes: ({
        "opsz": Math.max(20, Math.min(48, root.size)),
        "wght": root.weight,
        "FILL": root.filled ? 1 : 0,
        "GRAD": root.grade
    })

    // GPU curve rasterizer: crisp, resolution-independent, and safe under rotation/scaling
    // (unlike NativeRendering). Set per-instance if a different mode is ever needed.
    renderType: Text.CurveRendering
    textFormat: Text.PlainText
    horizontalAlignment: Text.AlignHCenter
    verticalAlignment: Text.AlignVCenter
}
