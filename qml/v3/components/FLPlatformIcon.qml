import QtQuick
import QtQuick.VectorImage

Image {
    id: control

    required property int platformId

    readonly property int _size: Math.min(width, 128)

    // TODO
    // Preferred platform icon; falls back to the generic glyph when its file is
    // missing (e.g. a platform with no shipped icon). _missing resets whenever
    // the platform changes so a recycled delegate re-tries its real icon
    readonly property url _preferred: PlatformService.getPlatformIconSource(platformId)
    property bool _missing: false
    on_PreferredChanged: _missing = false

    source: _missing ? "qrc:/icons/unknown" : _preferred
    onStatusChanged: if (status === Image.Error) {
        _missing = true;
    }

    fillMode: Image.PreserveAspectFit
    sourceSize: Qt.size(_size, _size)
}
