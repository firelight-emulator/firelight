import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

FLPage {
    id: root

    FLColumnLayout {
        anchors.fill: parent
        spacing: AppStyle.spacingMd

        FLGridView {
            id: slotGrid

            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
            Layout.fillWidth: true
            Layout.preferredHeight: 160

            FLFocus.enterFromBelow: slotGrid.currentItem

            cellWidth: 160
            cellHeight: 160

            model: GamepadListModel {}
            delegate: FocusScope {
                id: controllerButton
                required property var model
                required property int index

                width: GridView.view.cellWidth
                height: GridView.view.cellHeight

                FLButtonBase {
                    anchors.fill: parent
                    anchors.margins: AppStyle.spacingMd

                    focus: true

                    contentItem: Item {
                        anchors.fill: parent
                        anchors.margins: AppStyle.spacingMd
                        Image {
                            id: controllerIcon
                            source: controllerButton.model.image_url
                            visible: controllerButton.model.connected
                            fillMode: Image.PreserveAspectFit
                            sourceSize.width: 256
                            anchors.fill: parent
                        }

                        Text {
                            anchors.centerIn: parent
                            text: controllerButton.index + 1
                            font.pixelSize: AppStyle.fontSizeMedium
                            font.family: AppStyle.fontFamily
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            visible: !controllerButton.model.connected
                        }
                    }

                    variant: "default"
                    rounded: false

                    onClicked: {
                        console.log("Controller " + (index + 1) + " clicked");
                    }
                }
            }
        }

        SettingsGroup {
            group: "controllers-configure-all"
            Layout.fillWidth: true
        }

        FLDivider {
            Layout.fillWidth: true
        }

        SettingsGroup {
            group: "controllers-general"
            Layout.fillWidth: true
        }

        FLColumnSpacer {}
    }
}
