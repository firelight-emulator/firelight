// TODO: NEEDS REVIEW
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

// TODO
// A menu beside the page it selects. Each model entry is one of:
//   { type: "page", key, label, page, route? }   route defaults to key, read only when routeBase is set
//   { type: "action", key, label }               emits actionTriggered(key)
//   { type: "header", label }
//   { type: "divider" }
//
// A row's label, enabled and visible may each be a function. The row re-reads it whenever what it
// reads changes; the content swap reads it only when model, currentKey or the route changes
// An entry with enabled false dims the row and blocks activation; the cursor still reaches it.
//
// menuOnRight also starts the cursor in the content and lets Back fall through to the router.
// header and footer children size from menuWidth, never from parent.width
FLPage {
    id: root
    objectName: "FLTwoColumnPage|" + root.currentKey

    property bool menuOnRight: false
    property real menuWidth: AppStyle.sidebarWidth
    property real contentMaxWidth: Infinity
    property bool activateOnFocus: true
    property string routeBase: ""
    property var model: []
    property alias header: headerColumn.data
    property alias footer: footerColumn.data
    property string currentKey: ""

    signal actionTriggered(string key)

    property Component _extraPage: null
    property string _shownKey: ""
    property int _checkedRowFocusClaim: 0
    property bool _isReady: false
    property bool _isSwapImmediate: false
    property bool _isSyncingFromRouter: false

    readonly property bool _hasMenu: root.model.length + headerColumn.children.length + footerColumn.children.length > 0

    // TODO
    // Shows a page that is not in the menu until a row is selected or showCurrentPage is called
    function showPage(page: Component) {
        if (root._extraPage === page) {
            return;
        }

        root._extraPage = page;
        root._shownKey = "";
        root._swap(page, 0);
    }

    function showCurrentPage() {
        if (root._extraPage === null) {
            return;
        }

        root._clearExtraPage();
        root._syncContent();
    }

    function focusContent() {
        if (root._canEnterContent()) {
            contentPane.forceActiveFocus();
        }
    }

    function _select(key: string) {
        if (key !== root.currentKey) {
            root.currentKey = key;
            return;
        }

        root.showCurrentPage();
    }

    // TODO
    // Selects a row and moves the cursor into its page
    function _open(key: string) {
        if (InputMethodManager.usingMouse) {
            root._select(key);
            return;
        }

        root._isSwapImmediate = true;
        root._select(key);
        root._isSwapImmediate = false;
        root.focusContent();
    }

    function _focusMenu() {
        menuPane.forceActiveFocus();
    }

    function _canEnterContent(): bool {
        return stack.currentItem !== null && !stack.busy && stack.currentItem.FLFocus.mode !== FLFocus.Skip;
    }

    function _findPage(matches: var): int {
        for (let i = 0; i < root.model.length; i++) {
            const entry = root.model[i];

            if (entry.type === "page" && root._readEntry(entry, "visible", true) && matches(entry)) {
                return i;
            }
        }

        return -1;
    }

    function _routeOf(entry: var): string {
        return entry.route !== undefined ? entry.route : entry.key;
    }

    function _readEntry(entry: var, field: string, fallback: var): var {
        const value = typeof entry[field] === "function" ? entry[field]() : entry[field];

        return value === undefined || value === null ? fallback : value;
    }

    readonly property string _firstRowKey: {
        for (let i = 0; i < root.model.length; i++) {
            const entry = root.model[i];
            const isRow = entry.type === "page" || entry.type === "action";

            if (isRow && root._readEntry(entry, "visible", true)) {
                return entry.key;
            }
        }

        return "";
    }

    function _clearExtraPage() {
        if (root._extraPage === null) {
            return;
        }

        root._extraPage = null;

        if (!menuPane.activeFocus) {
            root._checkedRowFocusClaim++;
        }
    }

    function _syncAll() {
        root._isSwapImmediate = true;
        root._syncFromRouter();
        root._syncContent();
        root._isSwapImmediate = false;
    }

    function _syncContent() {
        if (!root._isReady || root._extraPage !== null) {
            return;
        }

        const index = root._findPage(entry => entry.key === root.currentKey);

        if (index < 0) {
            const first = root._findPage(() => true);

            if (first >= 0 && (root.activateOnFocus || root.routeBase !== "")) {
                root.currentKey = root.model[first].key;
            }

            return;
        }

        if (root.currentKey === root._shownKey) {
            return;
        }

        const shownIndex = root._findPage(entry => entry.key === root._shownKey);
        root._shownKey = root.currentKey;
        root._swap(root.model[index].page, shownIndex < 0 ? 0 : index - shownIndex);
    }

    function _swap(page: Component, direction: int) {
        if (page === null) {
            console.warn("FLTwoColumnPage: no page for " + root.currentKey);
            return;
        }

        contentFlick.contentY = 0;

        if (stack.currentItem === null) {
            stack.pushItem(page, {}, StackView.Immediate);
        } else {
            const isImmediate = root._isSwapImmediate || contentPane.activeFocus || InputMethodManager.keyRepeating;
            const preset = direction > 0 ? "sectionDown" : "sectionUp";
            const isAnimated = direction !== 0 && !isImmediate;
            const operation = isAnimated ? stack.transitions.apply(preset, null) : StackView.Immediate;
            stack.replaceCurrentItem(page, {}, operation);
        }

        if (contentPane.focus && !root._canEnterContent()) {
            root._focusMenu();
        }
    }

    function _syncFromRouter() {
        if (root.routeBase === "" || !root._isReady || Router.matchedPattern !== root.routeBase) {
            return;
        }

        const matched = Router.match(Router.path, [root.routeBase + "/:section"]);
        const currentIndex = root._findPage(entry => entry.key === root.currentKey);
        let target = -1;

        if (matched.matched) {
            const section = matched.params.section;
            const isCurrentRoute = currentIndex >= 0 && root._routeOf(root.model[currentIndex]) === section;
            target = isCurrentRoute ? currentIndex : root._findPage(entry => root._routeOf(entry) === section);
        }

        if (target < 0) {
            target = currentIndex >= 0 ? currentIndex : root._findPage(() => true);
        }

        if (target < 0) {
            return;
        }

        if (root.model[target].key !== root.currentKey) {
            root._isSyncingFromRouter = true;
            root._select(root.model[target].key);
            root._isSyncingFromRouter = false;
        }

        const canonical = root.routeBase + "/" + root._routeOf(root.model[target]);

        if (Router.path === canonical) {
            return;
        }

        const stale = Router.path;
        Qt.callLater(() => {
            if (Router.path === stale) {
                Router.replace(canonical);
            }
        });
    }

    function _syncToRouter() {
        if (root.routeBase === "" || !root._isReady || root._isSyncingFromRouter) {
            return;
        }

        if (Router.matchedPattern !== root.routeBase) {
            return;
        }

        const index = root._findPage(entry => entry.key === root.currentKey);

        if (index < 0) {
            return;
        }

        const path = root.routeBase + "/" + root._routeOf(root.model[index]);

        if (Router.path !== path) {
            Router.replace(path);
        }
    }

    onCurrentKeyChanged: {
        root._clearExtraPage();
        root._syncContent();
        root._syncToRouter();
    }

    onModelChanged: root._syncAll()

    Component.onCompleted: {
        root._isReady = true;
        root._syncAll();
    }

    StackView.onActivating: root._syncAll()

    Connections {
        target: Router
        enabled: root.routeBase !== ""

        function onNavigated() {
            root._syncFromRouter();
        }
    }

    RowLayout {
        anchors.fill: parent
        layoutDirection: root.menuOnRight ? Qt.RightToLeft : Qt.LeftToRight
        spacing: AppStyle.spacingLg

        FocusScope {
            id: menuPane
            objectName: "FLTwoColumnPage|menu"
            Layout.preferredWidth: root.menuWidth
            Layout.fillHeight: true
            visible: root._hasMenu
            focus: !root.menuOnRight && root._hasMenu
            focusPolicy: Qt.TabFocus

            FLFocus.mode: FLFocus.Stop

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                FLScrollColumnLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 0

                    FLFocus.holdEdges: FLFocus.Vertical

                    Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: AppStyle.spacingLg
                        visible: headerColumn.children.length === 0
                    }

                    ColumnLayout {
                        id: headerColumn
                        Layout.fillWidth: true
                        spacing: AppStyle.spacingMd
                    }

                    FLDivider {
                        Layout.topMargin: AppStyle.spacingMd + AppStyle.spacingSm
                        Layout.bottomMargin: AppStyle.spacingMd
                        visible: headerColumn.children.length > 0
                    }

                    Repeater {
                        model: root.model

                        delegate: DelegateChooser {
                            role: "type"

                            DelegateChoice {
                                roleValue: "page"

                                MenuNavigationItem {
                                    id: pageRow
                                    required property var modelData
                                    label: root._readEntry(pageRow.modelData, "label", "")
                                    visible: root._readEntry(pageRow.modelData, "visible", true)
                                    canInteract: root._readEntry(pageRow.modelData, "enabled", true)
                                    checked: pageRow.modelData.key === root.currentKey
                                    // TODO
                                    // Re-asserted whenever the claim counter changes
                                    focus: pageRow.checked && root._checkedRowFocusClaim >= 0
                                    onActiveFocusChanged: {
                                        if (pageRow.activeFocus && root.activateOnFocus && pageRow.canInteract) {
                                            root._select(pageRow.modelData.key);
                                        }
                                    }
                                    onClicked: {
                                        if (pageRow.canInteract) {
                                            root._open(pageRow.modelData.key);
                                        }
                                    }
                                }
                            }

                            DelegateChoice {
                                roleValue: "action"

                                MenuNavigationItem {
                                    id: actionRow
                                    required property var modelData
                                    label: root._readEntry(actionRow.modelData, "label", "")
                                    visible: root._readEntry(actionRow.modelData, "visible", true)
                                    canInteract: root._readEntry(actionRow.modelData, "enabled", true)
                                    focus: root.currentKey === "" && actionRow.modelData.key === root._firstRowKey
                                    onClicked: {
                                        if (actionRow.canInteract) {
                                            root.actionTriggered(actionRow.modelData.key);
                                        }
                                    }
                                }
                            }

                            DelegateChoice {
                                roleValue: "header"

                                Text {
                                    id: sectionTitle
                                    required property var modelData
                                    Layout.fillWidth: true
                                    Layout.topMargin: AppStyle.spacingSm
                                    Layout.bottomMargin: AppStyle.spacingSm
                                    text: root._readEntry(sectionTitle.modelData, "label", "")
                                    visible: root._readEntry(sectionTitle.modelData, "visible", true)
                                    color: Theme.textMuted
                                    elide: Text.ElideRight
                                    font.pixelSize: AppStyle.fontSizeSmall
                                    font.family: AppStyle.fontFamily
                                }
                            }

                            DelegateChoice {
                                roleValue: "divider"

                                FLDivider {
                                    id: rowDivider
                                    required property var modelData
                                    Layout.topMargin: AppStyle.spacingMd
                                    Layout.bottomMargin: AppStyle.spacingMd
                                    visible: root._readEntry(rowDivider.modelData, "visible", true)
                                }
                            }

                            DelegateChoice {
                                roleValue: "spacer"

                                Item {
                                    id: rowSpacer
                                    required property var modelData
                                    Layout.topMargin: AppStyle.spacingMd
                                    Layout.bottomMargin: AppStyle.spacingMd
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 1
                                    visible: root._readEntry(rowSpacer.modelData, "visible", true)
                                }
                            }
                        }
                    }
                }

                ColumnLayout {
                    id: footerColumn
                    Layout.fillWidth: true
                    Layout.preferredWidth: root.menuWidth
                    spacing: AppStyle.spacingMd
                }
            }
        }

        FLColumnDivider {
            Layout.fillHeight: true
            visible: root._hasMenu
        }

        FocusScope {
            id: contentPane
            objectName: "FLTwoColumnPage|content"
            Layout.fillWidth: true
            Layout.fillHeight: true
            focus: root.menuOnRight || !root._hasMenu
            focusPolicy: Qt.TabFocus

            FLFocus.mode: contentPane.activeFocus || root._canEnterContent() ? FLFocus.Stop : FLFocus.Skip
            FLFocus.actions: [
                FLAction {
                    keys: [Qt.Key_Back, Qt.Key_Escape]
                    label: qsTr("Back")
                    sound: SoundEffects.back
                    enabled: !root.menuOnRight
                    hidden: root.menuOnRight
                    onTriggered: root._focusMenu()
                }
            ]

            Flickable {
                id: contentFlick
                anchors.fill: parent
                anchors.topMargin: AppStyle.spacingLg
                clip: true
                contentWidth: contentFlick.width
                contentHeight: stack.height
                flickableDirection: Flickable.VerticalFlick
                boundsBehavior: Flickable.StopAtBounds

                StackView {
                    id: stack
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: Math.min(root.contentMaxWidth, contentFlick.width - 2 * AppStyle.spacingXl)
                    height: Math.max(contentFlick.height, stack.currentItem?.implicitHeight ?? 0)
                    focus: true

                    property StackTransitions transitions: StackTransitions {
                        view: stack
                        blinksCursor: false
                    }
                }
            }
        }
    }
}
