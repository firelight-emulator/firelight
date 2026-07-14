import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Always-visible reminder that a lobby is active while browsing the rest of
// the app. Click to open the lobby page.
Rectangle {
    id: root

    visible: NetworkService.inLobby
    width: chipRow.implicitWidth + 24
    height: 34
    radius: 17
    color: Theme.surface
    border.color: Theme.border
    border.width: 1

    RowLayout {
        id: chipRow
        anchors.centerIn: parent
        spacing: 8

        Rectangle {
            width: 8
            height: 8
            radius: 4
            color: Theme.success
        }
        Text {
            text: {
                if (NetworkService.sessionPhase === "in-game") {
                    return "Playing " + NetworkService.gameName
                }
                return "Lobby · " + NetworkService.memberCount + " player"
                       + (NetworkService.memberCount === 1 ? "" : "s")
            }
            color: Theme.textPrimary
            font.pixelSize: AppStyle.fontSizeSmall
        }
    }

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: Router.navigateTo("/netplay")
    }
}
