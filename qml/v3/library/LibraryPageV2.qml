import QtQuick
import QtQml
import QtQuick.Controls
import QtQuick.Layouts 1.0
import Firelight 1.0

FocusScope {
    id: root

    // **********************************
    // Title bar stuff
    // **********************************
    property Component headerLeading: root.openCollectionId !== -1 ? collectionHeader : null
    property Component headerCenter: root.openCollectionId !== -1 ? null : tabsHeader
    property Component headerTrailing: null

    property Component collectionHeader: Component {
        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: root.openCollectionRow !== null ? root.openCollectionRow.displayName : ""
            color: Theme.textPrimary
            font.family: AppStyle.fontFamily
            font.pixelSize: AppStyle.fontSizeLarge
            font.weight: Font.Medium
        }
    }

    property Component tabsHeader: Component {
        NavigationTabBar {
            tabs: ["Games", "Collections"]

            currentIndex: root.libraryTab

            clickAction: function () {
                root.enterFocus();
            }

            Keys.onPressed: (event) => {
                if (event.key !== Qt.Key_Down || !libraryContentStack.currentItem || !libraryContentStack.currentItem.focusFirstItem) {
                    return;
                }

                libraryContentStack.currentItem.focusFirstItem();
                event.accepted = true;
            }

            onTabSelected: index => root.showTab(index)
        }
    }

    // **********************************
    // Tab management
    // **********************************
    property int libraryTab: 0
    property int openCollectionId: -1

    function syncFromPath() {
        if (!Router.isActive("/library") || Router.matchedPattern !== "/library") {
            return;
        }

        const matched = Router.match(Router.path, ["/library/collections/:collectionId"]);
        root.openCollectionId = matched.matched ? parseInt(matched.params.collectionId) : -1;
        root.libraryTab = Router.isActive("/library/collections") ? 1 : 0;
        root.syncStack();
    }

    Connections {
        target: Router

        function onNavigated() {
            root.syncFromPath();
        }
    }

    function showTab(index) {
        if (index === root.libraryTab) {
            return;
        }

        Router.replace(index === 1 ? "/library/collections" : "/library");
    }

    function openCollection(collectionId) {
        Router.navigate("/library/collections/" + collectionId);
    }

    function enterFocus() {
        libraryContentStack.forceActiveFocus();
    }

    property int lastStackRank: 0

    function syncStack() {
        const target = root.openCollectionId !== -1 ? collectionPanel : root.libraryTab === 1 ? collectionsPanel : gameView;

        if (libraryContentStack.currentItem === null || libraryContentStack.currentItem === target) {
            return;
        }

        const rank = root.openCollectionId !== -1 ? 2 : root.libraryTab;
        const forward = rank > root.lastStackRank;
        root.lastStackRank = rank;
        libraryContentStack.replaceCurrentItem(target, {}, forward ? StackView.PushTransition : StackView.PopTransition);
    }

    property var openCollectionRow: null

    function refreshOpenCollection() {
        root.openCollectionRow = root.openCollectionId === -1 ? null : LibraryFolderModel.folderById(root.openCollectionId);
    }

    property int shownCollectionId: -1

    onOpenCollectionIdChanged: {
        root.refreshOpenCollection();

        if (root.openCollectionId !== -1) {
            root.shownCollectionId = root.openCollectionId;
            root.collectionPanelUsed = true;
        }
    }

    Component.onCompleted: {
        root.syncFromPath();
        root.refreshOpenCollection();
        Qt.callLater(root.syncStack);
    }

    Connections {
        target: LibraryFolderModel

        function onDataChanged() {
            root.refreshOpenCollection();
        }
    }

    property bool collectionPanelUsed: false

    // **********************************
    // The actual content
    // **********************************
    StackView {
        id: libraryContentStack

        readonly property int _stackSlideDistance: AppStyle.spacingSm
        readonly property real _stackSlideDuration: AppStyle.durationSlow
        readonly property var _stackSlideEasing: AppStyle.easingStandard

        anchors.fill: parent
        anchors.leftMargin: AppStyle.windowPadding
        anchors.rightMargin: AppStyle.windowPadding

        FLFocus.actions: [
            FLAction {
                label: "Previous tab"
                keys: [Qt.Key_Minus]
                hidden: true
                enabled: root.libraryTab === 1
                onTriggered: {
                    if (root.libraryTab === 1) {
                        root.showTab(0);
                    }
                }
                sound: SoundEffects.tabBarShoulderButtonNav
            },
            FLAction {
                label: "Next tab"
                keys: [Qt.Key_Equal]
                hidden: true
                enabled: root.libraryTab === 0
                onTriggered: {
                    if (root.libraryTab === 0) {
                        root.showTab(1);
                    }
                }
                sound: SoundEffects.tabBarShoulderButtonNav
            }
        ]

        bottomPadding: 0
        horizontalPadding: 0
        topPadding: 0
        verticalPadding: 0

        initialItem: gameView

        focus: true

        background: Item {}

        pushEnter: Transition {
            SequentialAnimation {
                ScriptAction {
                    script: {
                        if (root.activeFocus) {
                            FocusCursor.startBlink();
                        }
                    }
                }
                ParallelAnimation {
                    NumberAnimation {
                        property: "opacity"
                        from: 0
                        to: 1
                        duration: libraryContentStack._stackSlideDuration
                        easing.type: libraryContentStack._stackSlideEasing
                    }
                    NumberAnimation {
                        property: "x"
                        from: libraryContentStack._stackSlideDistance
                        to: 0
                        duration: libraryContentStack._stackSlideDuration
                        easing.type: libraryContentStack._stackSlideEasing
                    }
                }
                ScriptAction {
                    script: {
                        FocusCursor.endBlink();
                    }
                }
            }

            onRunningChanged: {
                if (!running) {
                    Qt.callLater(FocusCursor.endBlink);
                }
            }
        }

        pushExit: Transition {
            ParallelAnimation {
                NumberAnimation {
                    property: "opacity"
                    from: 1
                    to: 0
                    duration: libraryContentStack._stackSlideDuration
                    easing.type: libraryContentStack._stackSlideEasing
                }
                NumberAnimation {
                    property: "x"
                    from: 0
                    to: -libraryContentStack._stackSlideDistance
                    duration: libraryContentStack._stackSlideDuration
                    easing.type: libraryContentStack._stackSlideEasing
                }
            }
        }

        popEnter: Transition {
            SequentialAnimation {
                ScriptAction {
                    script: {
                        if (root.activeFocus) {
                            FocusCursor.startBlink();
                        }
                    }
                }
                ParallelAnimation {
                    NumberAnimation {
                        property: "opacity"
                        from: 0
                        to: 1
                        duration: libraryContentStack._stackSlideDuration
                        easing.type: libraryContentStack._stackSlideEasing
                    }
                    NumberAnimation {
                        property: "x"
                        from: -libraryContentStack._stackSlideDistance
                        to: 0
                        duration: libraryContentStack._stackSlideDuration
                        easing.type: libraryContentStack._stackSlideEasing
                    }
                }
                ScriptAction {
                    script: {
                        FocusCursor.endBlink();
                    }
                }
            }

            onRunningChanged: {
                if (!running) {
                    Qt.callLater(FocusCursor.endBlink);
                }
            }
        }

        popExit: Transition {
            ParallelAnimation {
                NumberAnimation {
                    property: "opacity"
                    from: 1
                    to: 0
                    duration: libraryContentStack._stackSlideDuration
                    easing.type: libraryContentStack._stackSlideEasing
                }
                NumberAnimation {
                    property: "x"
                    from: 0
                    to: libraryContentStack._stackSlideDistance
                    duration: libraryContentStack._stackSlideDuration
                    easing.type: libraryContentStack._stackSlideEasing
                }
            }
        }
    }

    CollectionDialog {
        id: collectionDialog
    }

    FocusScope {
        GameView {
            id: gameView
        }

        CollectionsView {
            id: collectionsPanel
            visible: false

            onCollectionOpened: collectionId => root.openCollection(collectionId)
            onNewCollectionRequested: collectionDialog.openForCreate(false, -1)
        }

        Loader {
            id: collectionPanel
            active: root.collectionPanelUsed
            sourceComponent: collectionGameView
        }
    }

    Component {
        id: collectionGameView

        GameView {
            filterFolderId: root.shownCollectionId
        }
    }
}
