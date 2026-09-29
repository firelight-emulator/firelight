import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

FLPage {
    id: root

    FLColumnLayout {
        anchors.fill: parent
        spacing: AppStyle.spacingMd

        FLReorderGridView {
            id: slotGrid

            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
            Layout.fillWidth: true
            Layout.leftMargin: -AppStyle.spacingSm
            Layout.rightMargin: -AppStyle.spacingSm
            Layout.preferredHeight: cellHeight

            FLFocus.enterFromBelow: slotGrid.currentItem

            cellWidth: slotGrid.width / 4
            cellHeight: slotGrid.cellWidth

            onMoveRequested: (fromIndex, toIndex) => {
                InputService.moveGamepad(fromIndex, toIndex);
            }

            model: GamepadListModel {}

            // A reload under a lifted tile ends the reorder where it stands
            Connections {
                target: slotGrid.model

                function onModelReset() {
                    if (slotGrid.lifted) {
                        slotGrid.place();
                    }
                }
            }

            delegate: FocusScope {
                id: controllerButton
                required property var model
                required property int index

                width: GridView.view.cellWidth
                height: GridView.view.cellHeight

                Rectangle {
                    visible: !controllerButton.model.connected
                    border.color: Theme.border
                    color: "transparent"
                    radius: AppStyle.radiusSm

                    anchors.fill: parent
                    anchors.margins: AppStyle.spacingSm

                    Text {
                        anchors.centerIn: parent
                        text: controllerButton.index + 1
                        font.pixelSize: AppStyle.fontSizeMedium
                        font.family: AppStyle.fontFamily
                        color: Theme.borderStrong
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                ControllerGridItem {
                    id: controllerGridItem

                    index: controllerButton.index
                    connected: controllerButton.model.connected
                    imageUrl: controllerButton.model.image_url
                    isLifted: slotGrid.lifted && slotGrid.liftedIndex === controllerButton.index

                    anchors.fill: parent
                    anchors.margins: AppStyle.spacingSm
                    visible: controllerButton.model.connected

                    onReorderingRequested: {
                        slotGrid.beginReorder(controllerButton.index)
                    }

                    onConfigureRequested: {
                        Router.navigate("/controllers/configure/1")
                    }

                    onClicked: {
                        if (slotGrid.lifted) {
                            slotGrid.place();
                            return;
                        }

                        controllerGridItem.configureRequested();
                    }
                }
            }
        }

        SettingsGroup {
            group: "controllers-configure-all"
            showHeader: false
            Layout.fillWidth: true
        }

        FLDivider {
            Layout.fillWidth: true
        }

        SettingsGroup {
            group: "controllers-general"
            showHeader: false
            Layout.fillWidth: true
        }

        FLColumnSpacer {}
    }
}
