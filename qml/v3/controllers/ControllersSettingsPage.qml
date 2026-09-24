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
            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
            Layout.fillWidth: true
            Layout.preferredHeight: 160

            cellWidth: 160
            cellHeight: 160

            model: 4
            delegate: FLButtonBase {
                text: "Controller " + (index + 1)
                variant: "subtle"
                width: GridView.view.cellWidth
                height: GridView.view.cellHeight
                rounded: false
                checkedColor: Theme.switch2Color
                onClicked: {
                    console.log("Controller " + (index + 1) + " clicked")
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