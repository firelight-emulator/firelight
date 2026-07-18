import QtQuick

// Backwards-compatible wrapper around Icon. FLIcon historically took an icon `name` via its
// `icon` property and rendered a raster SVG (blurry/choppy on HiDPI); it now renders the crisp
// Material Symbols glyph through Icon. `icon` aliases Icon's `name`; `size`/`filled`/`color`
// are inherited from Icon. Prefer using Icon directly in new code
Icon {
    id: control
    property alias icon: control.name
}
