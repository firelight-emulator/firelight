pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import Firelight 1.0

Flickable {
    id: page

    required property var model

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
        modId: model.id
        width: Math.min(parent.width, 1600)
    }

    // Pane {
    //     id: contentPane
    //
    //     horizontalPadding: 20
    //     // height: parent.height
    //     width: Math.min(parent.width, 1600)
    //
    //
    //     background: Item {
    //     }
    //
    //     contentItem: FLModShopItemPanel {
    //         id: contentColumn
    //
    //         // width: parent.width
    //         anchors.horizontalCenter: parent.horizontalCenter
    //         modId: 0
    //         // width: Math.min(parent.width, 1600)
    //     }
    // }
}

