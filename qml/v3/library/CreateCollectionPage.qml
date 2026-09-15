import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import Firelight 1.0

FocusScope {
    id: root

    focus: true

    property int parentId: -1

    property bool routeActive: true

    property string discardMessage: qsTr("Return without saving changes?")
    property string discardConfirmText: qsTr("OK")
    property string discardCancelText: qsTr("Continue reordering")

    // Which step of the workflow is showing
    property int stepIndex: 0
    readonly property int stepCount: 1
    readonly property bool onLastStep: root.stepIndex === root.stepCount - 1

    property bool _dirty: false

    property Component headerLeading: Component {
        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: qsTr("Create Collection")
            color: Theme.textPrimary
            font.family: AppStyle.fontFamily
            font.pixelSize: AppStyle.fontSizeLarge
            font.weight: Font.Medium
        }
    }

    StackView.onActivating: {
        grid.resetCursor();
        grid.focusCurrentItem();
    }

    ListModel {
        id: stagedOrder
    }

    property var originalIds: []

    property int liftOriginIndex: -1

    function load() {
        const folders = LibraryFolderModel.foldersInParent(root.parentId);
        const ids = [];

        stagedOrder.clear();
        _dirty = false;

        for (var i = 0; i < folders.length; i++) {
            stagedOrder.append(folders[i]);
            ids.push(folders[i].folderId);
        }

        root.originalIds = ids;
        root.liftOriginIndex = -1;
    }

    function isDirty() {
        if (stagedOrder.count !== root.originalIds.length) {
            return true;
        }

        for (var i = 0; i < stagedOrder.count; i++) {
            if (stagedOrder.get(i).folderId !== root.originalIds[i]) {
                return true;
            }
        }

        return false;
    }

    function stagedIds() {
        const ids = [];

        for (var i = 0; i < stagedOrder.count; i++) {
            ids.push(stagedOrder.get(i).folderId);
        }

        return ids;
    }

    function save() {
        if (!LibraryFolderModel.reorderFolders(root.parentId, root.stagedIds())) {
            return;
        }

        root.originalIds = root.stagedIds();
        Router.back();
    }

    function mayLeave() {
        if (!root.isDirty()) {
            return true;
        }

        confirmDialog.open();
        return false;
    }

    function enter() {
        root.load();

        Router.setLeaveGuard(root.mayLeave);
    }

    Component.onCompleted: root.enter()

    onRouteActiveChanged: {
        if (root.routeActive) {
            root.enter();
            return;
        }

        Router.clearLeaveGuard();
    }

    FLDialog {
        id: confirmDialog

        showCancel: true
        acceptText: root.discardConfirmText
        rejectText: root.discardCancelText

        onAccepted: Router.resumePending()
        onRejected: Router.cancelPending()

        Text {
            Layout.fillWidth: true
            text: root.discardMessage
            color: Theme.textMuted
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            font.family: AppStyle.fontFamily
            font.pixelSize: AppStyle.fontSizeMedium
        }
    }

    FLGridView {
        id: grid
        anchors.top: parent.top
        anchors.topMargin: AppStyle.gameViewHeaderHeight
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.leftMargin: AppStyle.windowPadding + AppStyle.spacingXl
        anchors.right: separator.left
        anchors.rightMargin: AppStyle.spacingXl
        focus: true

        model: stagedOrder
        clip: true
        cellWidth: Math.round(160 * AppStyle.scale)
        cellHeight: Math.round(210 * AppStyle.scale)

        onMoveRequested: (from, to) => {
            stagedOrder.move(from, to, 1)
        }

        FLFocus.actions: [
            FLAction {
                keys: [Qt.Key_Back, Qt.Key_Escape]
                label: qsTr("Cancel")
                sound: SoundEffects.back
                enabled: grid.lifted
                hidden: !grid.lifted
                onTriggered: {
                    stagedOrder.move(grid.liftedIndex, root.liftOriginIndex, 1);
                    grid.currentIndex = root.liftOriginIndex;
                    grid.drop();
                    root.liftOriginIndex = -1;
                    grid.focusCurrentItem();
                    root._dirty = root.isDirty()
                }
            }
        ]

        move: Transition {
            NumberAnimation {
                properties: "x,y"
                duration: AppStyle.durationBase
                easing.type: AppStyle.easingStandard
            }
        }

        displaced: Transition {
            NumberAnimation {
                properties: "x,y"
                duration: AppStyle.durationBase
                easing.type: AppStyle.easingStandard
            }
        }

        delegate: CollectionTile {
            id: tile

            required property int index

            width: GridView.view.cellWidth
            height: GridView.view.cellHeight

            readonly property bool _beingReordered: grid.liftedIndex === tile.index

            actionLabel: grid.lifted ? qsTr("Drop") : qsTr("Move")
            actionSound: grid.lifted ? SoundEffects.back : SoundEffects.openPopup

            onClicked: {
                if (grid.lifted) {
                    grid.drop();
                    root.liftOriginIndex = -1;
                    root._dirty = root.isDirty()
                    return;
                }

                root.liftOriginIndex = tile.index;
                grid.lift(tile.index);
            }

            layer.enabled: _beingReordered
            layer.effect: MultiEffect {
                shadowEnabled: true
                shadowColor: Theme.shadow
                shadowBlur: AppStyle.elevationBlur
                shadowVerticalOffset: AppStyle.elevationOffset
                shadowHorizontalOffset: AppStyle.elevationOffset
            }

            transform: Translate {
                y: tile._beingReordered ? -AppStyle.reorderingLiftHeight : 0

                Behavior on y {
                    NumberAnimation {
                        duration: AppStyle.durationFast
                        easing.type: Easing.Linear
                    }
                }
            }
        }
    }

    FLColumnDivider {
        id: separator
        anchors.right: menuColumn.left
    }

    FLColumnLayout {
        id: menuColumn
        anchors.right: parent.right
        anchors.rightMargin: AppStyle.windowPadding
        anchors.bottom: parent.bottom
        anchors.top: parent.top
        width: 280

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
        }

        FLButton {
            Layout.alignment: Qt.AlignHCenter
            Layout.margins: AppStyle.spacingXl
            Layout.preferredWidth: parent.width * 0.8
            text: qsTr("Save")
            onClicked: {
                if (!canInteract) {
                    return;
                }

                root.save()
            }
            canInteract: root._dirty
        }
    }

    Component {
        id: noCollectionsView

        Item {
            Text {
                anchors.centerIn: parent
                text: qsTr("There are no collections to arrange")
                color: Theme.textMuted
                font.family: AppStyle.fontFamily
                font.pixelSize: AppStyle.fontSizeMedium
            }
        }
    }
}
