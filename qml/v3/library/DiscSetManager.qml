// TODO: NEEDS REVIEW
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

Item {
    id: root

    objectName: "DiscSetManager"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: AppStyle.spacingLg
        spacing: AppStyle.spacingMd

        RowLayout {
            Layout.fillWidth: true
            spacing: AppStyle.spacingMd

            Text {
                text: qsTr("Disc sets")
                color: Theme.textPrimary
                font.pixelSize: AppStyle.fontSizeXLarge
                font.weight: Font.DemiBold
                Layout.fillWidth: true
            }

            FLButton {
                objectName: "DiscSetManagerRefreshButton"
                text: qsTr("Refresh")
                onClicked: DiscSetModel.refresh()
            }
        }

        Text {
            text: qsTr("%1 rows").arg(discSetList.count)
            color: Theme.textMuted
            font.pixelSize: AppStyle.fontSizeSmall
            Layout.fillWidth: true
        }

        ListView {
            id: discSetList

            objectName: "DiscSetManagerList"
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: DiscSetModel
            spacing: AppStyle.spacingSm
            clip: true

            ScrollBar.vertical: ScrollBar {}

            delegate: Rectangle {
                id: row

                required property int index
                required property bool isSet
                required property int setId
                required property string title
                required property int discNumber
                required property int memberId
                required property int contentFileId
                required property string fileName
                required property bool isPresent
                required property bool isClaimed
                required property bool isUncertain
                required property string source
                required property int discCount

                objectName: "DiscSetManagerRow|" + (row.isSet ? row.title : row.fileName)
                width: discSetList.width
                height: AppStyle.rowHeight
                radius: AppStyle.radiusSm
                color: row.isSet ? Theme.surfaceElevated : "transparent"

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: row.isSet ? AppStyle.spacingMd : AppStyle.spacingXl
                    anchors.rightMargin: AppStyle.spacingMd
                    spacing: AppStyle.spacingMd

                    Text {
                        text: row.isSet ? row.title : qsTr("Disc %1").arg(row.discNumber)
                        color: Theme.textPrimary
                        font.pixelSize: AppStyle.fontSizeMedium
                        font.weight: row.isSet ? Font.DemiBold : Font.Normal
                        Layout.preferredWidth: Math.round(220 * AppStyle.scale)
                    }

                    Text {
                        text: row.isSet ? (row.discCount > 0 ? qsTr("%1 discs").arg(row.discCount) : qsTr("disc count unknown")) : row.fileName
                        color: Theme.textMuted
                        font.pixelSize: AppStyle.fontSizeSmall
                        elide: Text.ElideMiddle
                        Layout.fillWidth: true
                    }

                    Text {
                        visible: !row.isSet
                        text: row.isClaimed ? qsTr("not here yet") : (row.isPresent ? row.source : qsTr("file missing"))
                        color: Theme.textMuted
                        font.pixelSize: AppStyle.fontSizeSmall
                    }

                    Text {
                        visible: !row.isSet && row.isUncertain
                        text: qsTr("not sure")
                        color: Theme.warning
                        font.pixelSize: AppStyle.fontSizeSmall
                    }

                    FLButton {
                        visible: !row.isSet && row.isUncertain
                        text: qsTr("It's right")
                        onClicked: DiscSets.confirmPlacement(row.memberId)
                    }

                    FLIconButton {
                        visible: !row.isSet && row.discNumber > 1
                        objectName: "DiscSetManagerMoveUpButton|" + row.discNumber
                        iconName: "keyboard_arrow_up"
                        onClicked: DiscSets.setDiscNumber(row.memberId, row.discNumber - 1)
                    }

                    FLIconButton {
                        visible: !row.isSet
                        objectName: "DiscSetManagerMoveDownButton|" + row.discNumber
                        iconName: "keyboard_arrow_down"
                        onClicked: DiscSets.setDiscNumber(row.memberId, row.discNumber + 1)
                    }

                    FLButton {
                        visible: !row.isSet && !row.isClaimed
                        text: qsTr("Remove")
                        onClicked: DiscSets.removeFromSet(row.contentFileId)
                    }
                }
            }
        }
    }
}
