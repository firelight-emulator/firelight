import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

// What a guest sees while the host's game is live: the incoming stream,
// scaled to fit, with their inputs already ticking back to the host
Rectangle {
    id: root

    color: "black"

    onVisibleChanged: {
        if (visible) {
            NetworkService.attachStreamItem(streamItem);
        } else {
            NetworkService.detachStreamItem(streamItem);
        }
    }
    Component.onDestruction: NetworkService.detachStreamItem(streamItem)

    NetplayStreamItem {
        id: streamItem

        readonly property real sourceAspect: frameWidth > 0 && frameHeight > 0 ? frameWidth / frameHeight : 4 / 3

        anchors.centerIn: parent
        width: Math.min(parent.width, parent.height * sourceAspect)
        height: width / sourceAspect
    }

    Text {
        visible: streamItem.frameWidth === 0
        anchors.centerIn: parent
        text: "Waiting for the host's stream..."
        color: Theme.textMuted
        font.pixelSize: AppStyle.fontSizeMedium
    }

    RowLayout {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: AppStyle.spacingMd
        spacing: AppStyle.spacingSm

        Text {
            text: NetworkService.gameName
            color: Theme.textMuted
            font.pixelSize: AppStyle.fontSizeSmall
            Layout.alignment: Qt.AlignVCenter
        }
        Button {
            text: "Leave lobby"
            onClicked: NetworkService.leaveLobby()
        }
    }
}
