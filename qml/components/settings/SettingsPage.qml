import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

// Renders every group the catalog declares for a page, in declared order. This
// is what a settings page normally is:
//
//     SettingsPage { page: "appearance" }
//
// The group list comes from the catalog rather than being spelled out here, so a
// new group appears just by being declared — no page can silently omit one.
//
// `level`/`platformId`/`contentHash` only matter for emulation settings; app
// settings ignore them and always use the global tier.
FocusScope {
    id: root

    required property string page

    property int level: SettingsLevel.Global
    property var platformId: -1
    property var contentHash: ""

    implicitWidth: col.width
    implicitHeight: col.implicitHeight

    ColumnLayout {
        id: col
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 0

        Repeater {
            model: SettingsCatalog.groupsForPage(root.page)

            delegate: SettingsGroup {
                required property string modelData
                required property int index

                group: modelData
                level: root.level
                platformId: root.platformId
                contentHash: root.contentHash
                // Only the first card sits flush with the top of the page.
                showTopPadding: index > 0
            }
        }
    }
}
