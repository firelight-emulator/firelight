import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

Flickable {
    id: root

    default property alias content: column.children
    property alias spacing: column.spacing

    implicitHeight: column.implicitHeight
    boundsBehavior: Flickable.StopAtBounds

    contentWidth: width
    contentHeight: Math.max(column.implicitHeight, height)
    flickableDirection: Flickable.VerticalFlick
    clip: true

    FLColumnLayout {
        id: column
        width: root.width
    }
}