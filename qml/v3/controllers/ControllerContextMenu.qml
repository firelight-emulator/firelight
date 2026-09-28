import QtQuick
import Firelight 1.0

FLMenu {
    id: control

    signal configureRequested()
    signal reorderingRequested()

    FLMenuItem {
        label: qsTr("Configure controller")
        onClicked: {
            control.configureRequested();
        }
    }

    FLMenuSeparator {}

    FLMenuItem {
        label: "Reorder"
        onClicked: {
            control.reorderingRequested();
            control.close();
        }
    }
}
