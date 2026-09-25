import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

FocusScope {
    id: root

    objectName: "SuspendPointsPage"

    implicitHeight: column.implicitHeight

    SuspendPoints {
        id: suspendData
        contentHash: EmulationService.currentContentHash
        saveSlot: EmulationService.currentSaveSlotNumber
    }

    FLDialog {
        id: deleteDialog

        property int index: 0

        showCancel: true
        acceptText: qsTr("Delete")

        onAccepted: suspendData.deleteSuspendPoint(deleteDialog.index)

        Text {
            Layout.fillWidth: true
            text: qsTr("Delete the suspend point in slot %1?").arg(deleteDialog.index + 1)
            color: Theme.textMuted
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            font.family: AppStyle.fontFamily
            font.pixelSize: AppStyle.fontSizeMedium
        }
    }

    FLDialog {
        id: overwriteDialog

        property int index: 0

        showCancel: true
        acceptText: qsTr("Overwrite")

        onAccepted: EmulationService.writeSuspendPoint(overwriteDialog.index)

        Text {
            Layout.fillWidth: true
            text: qsTr("Overwrite the suspend point in slot %1?").arg(overwriteDialog.index + 1)
            color: Theme.textMuted
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            font.family: AppStyle.fontFamily
            font.pixelSize: AppStyle.fontSizeMedium
        }
    }

    ColumnLayout {
        id: column

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        spacing: AppStyle.spacingMd

        RowLayout {
            Layout.fillWidth: true
            spacing: AppStyle.spacingMd

            Text {
                Layout.fillWidth: true
                text: qsTr("Suspend points")
                color: Theme.textPrimary
                font.family: AppStyle.fontFamily
                font.pixelSize: AppStyle.fontSizeLarge
                font.weight: Font.DemiBold
                verticalAlignment: Text.AlignVCenter
            }

            FLButton {
                id: undoButton
                objectName: "SuspendPointsPage|UndoLastLoad"
                text: qsTr("Undo last load")
                iconName: "undo"
                variant: "subtle"
                canInteract: EmulationService.canUndoLoadSuspendPoint
                onClicked: EmulationService.undoLoadSuspendPoint()
            }
        }

        FLGridView {
            id: grid

            Layout.fillWidth: true
            Layout.preferredHeight: grid.contentHeight

            interactive: false
            model: suspendData.suspendPoints
            cellWidth: AppStyle.detailPanelWidth
            cellHeight: Math.round(AppStyle.detailPanelWidth * 1.2)

            delegate: FocusScope {
                id: slot

                required property var model
                required property int index

                width: GridView.view.cellWidth
                height: GridView.view.cellHeight

                FLSuspendPointCard {
                    id: card
                    objectName: "SuspendPointCard|" + (slot.index + 1)
                    anchors.fill: parent
                    anchors.margins: AppStyle.spacingSm
                    visible: slot.model.has_data
                    focus: slot.model.has_data
                    imageUrl: slot.model.image_url
                    dateTimeString: slot.model.timestamp
                    index: slot.index

                    FLFocus.showCursor: true
                    FLFocus.radius: AppStyle.radiusMd + 2
                    FLFocus.actions: [
                        FLAction {
                            keys: [Qt.Key_Enter, Qt.Key_Select, Qt.Key_Return, Qt.Key_Space]
                            label: qsTr("Load")
                            sound: SoundEffects.openPopup
                            onTriggered: card.clicked()
                        }
                    ]

                    onClicked: card.loadClicked()

                    onLoadClicked: {
                        EmulationService.loadSuspendPoint(slot.index);
                        // EmulationService.resume();
                    }

                    onDeleteClicked: {
                        deleteDialog.index = slot.index;
                        deleteDialog.open();
                    }

                    onOverwriteClicked: {
                        overwriteDialog.index = slot.index;
                        overwriteDialog.open();
                    }
                }

                FLButton {
                    objectName: "SuspendPointCreate|" + (slot.index + 1)
                    anchors.fill: parent
                    anchors.margins: AppStyle.spacingSm
                    visible: !slot.model.has_data
                    focus: !slot.model.has_data
                    text: qsTr("Create in slot %1").arg(slot.index + 1)
                    onClicked: EmulationService.writeSuspendPoint(slot.index)
                }
            }
        }
    }
}
