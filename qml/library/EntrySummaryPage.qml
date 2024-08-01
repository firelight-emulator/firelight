import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id: root
    objectName: "Details Tab Content"

    required property var entryData

    Text {
        Layout.fillWidth: true
        text: qsTr("Content path")
        color: "white"
        font.pointSize: 12
        font.family: Constants.regularFontFamily
        font.weight: Font.DemiBold
    }
    Pane {
        id: texxxxt
        // Layout.fillWidth: true
        padding: 4

        background: Rectangle {
            color: "black"
            radius: 8
        }

        contentItem: TextInput {
            padding: 4
            text: root.entryData.content_path
            font.family: Constants.regularFontFamily
            font.pointSize: 12
            color: "white"
            verticalAlignment: Text.AlignVCenter
            readOnly: true
        }
    }
    Text {
        text: "Active save slot: " + root.entryData.active_save_slot
        color: "white"
        font.family: Constants.regularFontFamily
        font.pointSize: 10
    }
    Text {
        text: "Added to library: " + root.entryData.created_at
        color: "white"
        font.family: Constants.regularFontFamily
        font.pointSize: 10
    }
    Item {
        Layout.fillHeight: true
    }
}