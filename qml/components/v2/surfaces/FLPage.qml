import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// TODO
// Standard screen scaffold: an optional padded strip of header actions
// (right-aligned) over a body that fills the rest flush to the edges — so a
// grid/list can run edge-to-edge and a form can pad itself. The strip hides when
// there are no actions. Not the window title bar
//
//   FLPage {
//       headerActions: [FLButton { text: "Favorites"; checkable: true }]
//       SomeView { anchors.fill: parent }
//   }
Pane {
    id: root

    property alias headerActions: actionsSlot.data
    default property alias content: bodySlot.data

    padding: 0
    background: Item {}

    contentItem: ColumnLayout {
        spacing: 0

        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: AppStyle.windowPadding
            Layout.rightMargin: AppStyle.windowPadding
            Layout.topMargin: AppStyle.spacingMd
            Layout.bottomMargin: AppStyle.spacingMd
            visible: actionsSlot.children.length > 0

            Item {
                Layout.fillWidth: true
            }

            Row {
                id: actionsSlot
                spacing: AppStyle.spacingSm
                Layout.alignment: Qt.AlignVCenter
            }
        }

        Item {
            id: bodySlot
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
