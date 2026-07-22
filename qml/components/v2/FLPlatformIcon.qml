import QtQuick
import QtQuick.VectorImage

Image {
    required property int platformId

    source: PlatformService.getPlatformIconSource(platformId)
    fillMode: Image.PreserveAspectFit
    sourceSize: Qt.size(128, 128)
}
