import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Flickable {
    boundsBehavior: Flickable.StopAtBounds
    contentHeight: theColumn.height

    ScrollBar.vertical: ScrollBar {
    }

    ColumnLayout {
        id: theColumn
        width: parent.width
        spacing: 0

        ToggleOption {
            Layout.fillWidth: true
            label: "Menu sounds"
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

        }
    }
}
