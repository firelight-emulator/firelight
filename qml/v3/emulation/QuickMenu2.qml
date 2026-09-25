import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

FLPage {
    id: root

    objectName: "QuickMenu"

    headerText: EmulationService.currentGameName

    FLFocus.actions: [
        FLAction {
            keys: [Qt.Key_Back, Qt.Key_Escape]
            label: qsTr("Resume")
            sound: SoundEffects.back
            onTriggered: EmulationService.resume()
        }
    ]

    FLDialog {
        id: resetDialog

        showCancel: true
        acceptText: qsTr("Reset")

        onAccepted: {
            EmulationService.resetGame();
            EmulationService.resume();
        }

        Text {
            Layout.fillWidth: true
            text: qsTr("Reset the game? Unsaved progress will be lost.")
            color: Theme.textMuted
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            font.family: AppStyle.fontFamily
            font.pixelSize: AppStyle.fontSizeMedium
        }
    }

    FLDialog {
        id: closeDialog

        showCancel: true
        acceptText: qsTr("Close game")

        onAccepted: EmulationService.stopEmulation()

        Text {
            Layout.fillWidth: true
            text: qsTr("Close the game? Unsaved progress will be lost.")
            color: Theme.textMuted
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            font.family: AppStyle.fontFamily
            font.pixelSize: AppStyle.fontSizeMedium
        }
    }

    Component {
        id: suspendPointsPage
        SuspendPointsPage {}
    }

    FLTwoColumnPage {
        id: content
        anchors.fill: parent

        model: [
            {
                "type": "action",
                "key": "resume",
                "label": qsTr("Resume game")
            },
            {
                "type": "action",
                "key": "reset",
                "label": qsTr("Reset game")
            },
            {
                "type": "divider"
            },
            {
                "type": "page",
                "key": "suspend-points",
                "label": qsTr("Suspend points"),
                "page": suspendPointsPage
            },
            {
                "type": "action",
                "key": "rewind",
                "label": qsTr("Rewind"),
                "enabled": function () {
                    return EmulationService.rewindEnabled;
                }
            },
            {
                "type": "divider"
            },
            {
                "type": "action",
                "key": "close",
                "label": qsTr("Close game")
            }
        ]

        onActionTriggered: function (key) {
            if (key === "resume") {
                EmulationService.resume();
            } else if (key === "reset") {
                resetDialog.open();
            } else if (key === "rewind") {
                EmulationService.resume();
                EmulationService.openRewindMenu();
            } else if (key === "close") {
                closeDialog.open();
            }
        }
    }
}
