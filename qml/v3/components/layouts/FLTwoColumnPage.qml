import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

FocusScope {
    id: root

    // {
    //   "title": "Page Title",
    //   "icon": "icon_name",
    //
    required property var pages

    FLColumnLayout {
        id: navColumn
    }
}