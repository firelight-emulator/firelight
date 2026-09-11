import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import Firelight 1.0

Pane {
    id: root

    readonly property var buttonOrder: [Qt.Key_Menu, Qt.Key_Back, Qt.Key_Select, Qt.Key_Return, Qt.Key_Enter, Qt.Key_Space]

    property var groups: []
    property bool showDivider: true

    verticalPadding: AppStyle.spacingSm
    horizontalPadding: 0

    function rankOf(group) {
        let best = root.buttonOrder.length;

        for (const binding of group.bindings) {
            const at = root.buttonOrder.indexOf(binding.key);

            if (at !== -1 && at < best) {
                best = at;
            }
        }

        return best === root.buttonOrder.length ? -1 : best;
    }

    function refresh() {
        const focused = root.Window.window ? root.Window.window.activeFocusItem : null;

        if (!focused) {
            root.groups = [];
            return;
        }

        root.groups = focused.FLFocus.collectActionGroups(focused).sort((a, b) => root.rankOf(a) - root.rankOf(b) || a.label.localeCompare(b.label));
    }

    Component.onCompleted: root.refresh()

    Connections {
        target: root.Window.window

        function onActiveFocusItemChanged() {
            Qt.callLater(root.refresh);
        }
    }

    background: Item {
        Rectangle {
            color: "transparent"
            anchors.fill: parent
        }

        Rectangle {
            color: "#4c4c4c"
            height: 1
            width: parent.width
            visible: root.showDivider
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: AppStyle.spacingXs

        FLGuideBarPill {
            text: qsTr("Open menu")
            bindings: [
                {
                    text: qsTr("Open menu"),
                    key: Qt.Key_Home
                }
            ]
            actionEnabled: true
            Layout.alignment: Qt.AlignVCenter
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        Repeater {
            Layout.alignment: Qt.AlignVCenter

            model: root.groups
            delegate: FLGuideBarPill {
                required property var modelData

                text: modelData.label
                bindings: modelData.bindings
                actionEnabled: modelData.enabled
                visible: !modelData.hidden
            }
        }
    }
}
