import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

// Advanced control to choose which libretro core runs a platform, at a given
// tier (level: 0 Game, 1 Platform). Writes the reserved "core" override via
// CoreRegistry; the settings/options below re-resolve automatically.
RowLayout {
    id: root

    property int level: 1 // Platform
    property int platformId: -1
    property string contentHash: ""

    property var cores: root.platformId !== -1 ? CoreRegistry.coresForPlatform(root.platformId) : []

    spacing: 12

    ColumnLayout {
        Layout.fillWidth: true
        spacing: 2
        Text {
            text: "Core"
            color: Theme.textPrimary
            font.pixelSize: AppStyle.fontSizeMedium
            font.family: Constants.regularFontFamily
            font.weight: Font.DemiBold
        }
        Text {
            text: "The emulator core used to run this platform. Changing it can affect save states and achievements."
            color: Theme.textMuted
            font.pixelSize: AppStyle.fontSizeSmall
            font.family: Constants.regularFontFamily
            font.weight: Font.Medium
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
    }

    ComboBox {
        id: combo
        Layout.preferredWidth: 280
        textRole: "name"
        valueRole: "id"
        model: root.cores

        function syncSelection() {
            const override = CoreRegistry.coreOverride(root.level, root.platformId, root.contentHash)
            const target = override !== "" ? override : CoreRegistry.resolvedCore(root.platformId, root.contentHash)
            currentIndex = Math.max(0, indexOfValue(target))
        }

        Component.onCompleted: syncSelection()
        onModelChanged: syncSelection()
        onActivated: CoreRegistry.setCoreOverride(root.level, root.platformId, root.contentHash, currentValue)
    }
}
