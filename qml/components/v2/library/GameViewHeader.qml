import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// TODO
// The library view's header: the scope identity block (icon + name over a
// description/count line) with a clear-filters action. Reads scope state on the
// GameView `view`
ColumnLayout {
    id: root

    property var view

    spacing: AppStyle.spacingSm

    // TODO
    // Scope identity: icon chip + name over a muted description/count line
    RowLayout {
        Layout.fillWidth: true
        spacing: AppStyle.spacingMd

        // TODO
        // Icon chip: platform logo, custom folder art, or a glyph fallback
        Rectangle {
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: AppStyle.artIconSizeMd
            Layout.preferredHeight: AppStyle.artIconSizeMd
            radius: AppStyle.radiusSm
            color: Qt.rgba(root.view.scopeIconColor.r, root.view.scopeIconColor.g, root.view.scopeIconColor.b, 0.14)

            FLPlatformIcon {
                anchors.fill: parent
                anchors.margins: AppStyle.spacingMd
                visible: root.view.scopePlatformId >= 0
                platformId: root.view.scopePlatformId
            }

            FLRoundedImage {
                anchors.fill: parent
                visible: root.view.scopePlatformId < 0 && root.view.scopeIconUrl !== ""
                source: root.view.scopeIconUrl
                radius: AppStyle.radiusSm
                fillMode: Image.PreserveAspectCrop
                background: "transparent"
            }
            Icon {
                anchors.centerIn: parent
                visible: root.view.scopePlatformId < 0 && root.view.scopeIconUrl === ""
                name: root.view.scopeIconName
                filled: false
                size: AppStyle.iconSizeMd
                color: root.view.scopeIconColor
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: AppStyle.spacingXs

            Text {
                Layout.fillWidth: true
                text: root.view.scopeLabel
                color: Theme.textPrimary
                font.family: AppStyle.fontFamily
                font.pixelSize: AppStyle.fontSizeLarge
                font.weight: Font.DemiBold
                elide: Text.ElideRight
            }
            Text {
                Layout.fillWidth: true
                text: {
                    const count = root.view.gameCount + (root.view.gameCount === 1 ? " game" : " games");
                    return root.view.scopeDescription !== "" ? root.view.scopeDescription + " · " + count : count;
                }
                color: Theme.textMuted
                font.family: AppStyle.fontFamily
                font.pixelSize: AppStyle.fontSizeMedium
                elide: Text.ElideRight
            }
        }

        FLButton {
            Layout.alignment: Qt.AlignVCenter
            visible: root.view.isDirty
            compact: true
            variant: "subtle"
            text: "Clear filters"
            onClicked: root.view.clearAll()
        }
    }
}
