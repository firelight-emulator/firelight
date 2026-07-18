import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root

    signal clicked

    color: model.occupied ? "#2d3542" : "#222222"
    border.color: model.isSelf ? "#5588dd" : "#333333"
    border.width: 1
    radius: 8

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 2

        RowLayout {
            Layout.fillWidth: true
            Text {
                text: "P" + model.slotNumber
                color: Theme.textMuted
                font.pixelSize: AppStyle.fontSizeSmall
                font.weight: Font.DemiBold
            }
            Item {
                Layout.fillWidth: true
            }
            Rectangle {
                visible: model.occupied
                width: 8
                height: 8
                radius: 4
                color: model.ready ? "#66cc66" : "#666666"
            }
        }

        Text {
            text: model.occupied ? model.displayName : "Empty"
            color: model.occupied ? Theme.textPrimary : "#666666"
            font.pixelSize: AppStyle.fontSizeSmall
            elide: Text.ElideRight
            Layout.fillWidth: true
        }

        Text {
            visible: model.occupied && model.isHostMember
            text: "Host"
            color: Theme.textMuted
            font.pixelSize: AppStyle.fontSizeSmall
        }
    }

    MouseArea {
        anchors.fill: parent
        cursorShape: NetworkService.isHost ? Qt.PointingHandCursor : Qt.ArrowCursor
        onClicked: root.clicked()
    }
}
