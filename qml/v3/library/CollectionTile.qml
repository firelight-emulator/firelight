import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import Firelight 1.0

FocusScope {
    id: root

    required property int folderId
    required property string displayName
    required property string icon1x1SourceUrl
    required property string color
    required property int folderType

    readonly property int gameCount: {
        const held = LibraryEntryModel.countByFolderId[String(root.folderId)];
        return held === undefined ? 0 : held;
    }

    readonly property bool isSmart: root.folderType === 1

    property string actionLabel: qsTr("Open")
    property var actionSound: SoundEffects.openPopup

    signal clicked

    Button {
        id: control
        anchors.fill: parent
        anchors.margins: AppearanceSettings.libraryIconGridTileSpacing / 2
        padding: AppStyle.spacingXs
        focus: true

        HoverHandler {
            cursorShape: Qt.PointingHandCursor
        }

        objectName: "CollectionTile|" + root.displayName

        FLFocus.showCursor: true
        FLFocus.spacing: 3
        FLFocus.radius: AppStyle.radiusMd + 2
        FLFocus.focusSound: SoundEffects.gameTileFocus
        FLFocus.actions: [
            FLAction {
                keys: [Qt.Key_Enter, Qt.Key_Select, Qt.Key_Return, Qt.Key_Space]
                label: root.actionLabel
                sound: root.actionSound
                onTriggered: root.clicked()
            }
        ]

        onClicked: root.clicked()

        background: Rectangle {
            radius: AppStyle.radiusMd
            color: root.color !== "" ? root.color : Theme.surfaceElevated
        }

        contentItem: ColumnLayout {
            spacing: 0
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: width

                Icon {
                    anchors.centerIn: parent
                    visible: cover.status !== Image.Ready
                    name: root.isSmart ? "bookmark-star" : "folder"
                    size: Math.round(parent.width * 0.34)
                    color: Theme.textPrimary
                }

                Image {
                    id: cover
                    anchors.fill: parent
                    anchors.margins: Math.round(parent.width * 0.22)
                    visible: status === Image.Ready
                    source: root.icon1x1SourceUrl
                    fillMode: Image.PreserveAspectFit
                }
            }

            Text {
                id: title
                Layout.fillWidth: true
                Layout.topMargin: AppStyle.spacingSm

                text: root.displayName
                color: Theme.textPrimary
                font.family: AppStyle.fontFamily
                font.pixelSize: AppStyle.fontSizeMedium
                font.weight: Font.DemiBold
                elide: Text.ElideRight
            }

            Text {
                Layout.fillWidth: true
                text: root.gameCount === 1 ? "1 game" : root.gameCount + " games"
                color: Theme.textMuted
                font.family: AppStyle.fontFamily
                font.pixelSize: AppStyle.fontSizeSmall
                elide: Text.ElideRight
            }
        }
    }
}
