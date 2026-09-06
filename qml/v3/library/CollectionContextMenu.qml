import QtQuick
import Firelight 1.0

FLMenu {
    id: control

    required property var collectionModel

    FLMenuItem {
        label: qsTr("Edit")
    }

    FLMenuSeparator {}

    FLMenuItem {
        label: "View details"
    }
}
