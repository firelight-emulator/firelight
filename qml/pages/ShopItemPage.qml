pragma ComponentBehavior: Bound

import QtQuick

Flickable {
    id: page

    required property var modId

    // onWidthChanged: function() {
    //     console.log("page width changed: ", width)
    // }

    boundsBehavior: Flickable.StopAtBounds
    clip: true
    contentHeight: contentColumn.height

    FLModShopItemPanel {
        id: contentColumn
        focus: true

        landscape: page.width > page.height
        // width: parent.width
        anchors.horizontalCenter: parent.horizontalCenter
        modId: page.modId
        width: Math.min(parent.width, 1200)
    }
}
