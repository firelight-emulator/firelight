// TODO: NEEDS REVIEW
import QtQuick
import QtQuick.Layouts
import QtQuick.Window
import Firelight 1.0

import "focus_nav.js" as Nav

// TODO
// A rail of controls beside a list-or-grid that cross-fades when what it shows changes.
// Children declared on this are the rail's contents, top to bottom
FocusScope {
    id: root

    default property alias toolbarContent: toolbarColumn.data

    property Component gridComponent: null
    property Component listComponent: null

    // TODO
    // Shown instead of the grid or list while isEmpty. Which empty state that is belongs to
    // whoever knows why the view is empty
    property Component emptyComponent: null
    property bool isEmpty: false

    property real toolbarTopMargin: AppStyle.gameViewHeaderHeight - AppStyle.spacingSm
    property int toolbarWidth: 48

    // TODO
    // Which of grid and list is on screen. Written by the fade rather than bound, so a caller
    // asks for a change through requestViewMode or setViewMode rather than assigning
    property string viewMode: "grid"

    // TODO
    // Emitted while the view is invisible, for whatever has to change at the moment nothing
    // can be seen changing
    signal commit

    focus: true

    // TODO
    // Changes the view mode behind the fade
    function requestViewMode(mode: string) {
        root._pendingViewType = mode;
        refreshAnimation.restart();
    }

    // TODO
    // Changes the view mode in place, for a change that did not come from anything on screen
    function setViewMode(mode: string) {
        root.viewMode = mode;
    }

    // TODO
    // Puts the view back to the top, cursor included, for when what it shows is replaced rather
    // than narrowed
    function resetPosition() {
        const view = viewLoader.item;

        if (view === null) {
            return;
        }

        if (view.resetCursor !== undefined) {
            view.resetCursor();
        }
    }

    // TODO
    // Runs the fade, coalescing anything asked for while one is already running
    function queueRefresh() {
        if (refreshAnimation.running) {
            if (root._refreshApplied) {
                root._refreshQueued = true;
            }

            return;
        }

        root._refreshApplied = false;
        refreshAnimation.start();
    }

    // TODO
    // Where the cursor lands when the view is entered: what it shows, or the rail beside it when
    // there is nothing to land on
    onActiveFocusChanged: {
        if (root.activeFocus) {
            Qt.callLater(root._ensureCursor);
        }
    }

    // TODO
    // The rail button the cursor was last on, so returning to the rail returns to where it was
    property Item _lastRailItem: null

    // TODO
    // Puts the cursor on the rail: at the top when a press asks for the top, otherwise back where
    // it was
    function _focusRail(top: bool) {
        const kids = toolbarColumn.children;
        let target = null;

        if (!top && root._lastRailItem !== null && Nav.isFocusable(root._lastRailItem)) {
            target = root._lastRailItem;
        } else {
            const index = Nav.firstFocusable(kids, 0);
            target = index >= 0 ? kids[index] : null;
        }

        if (target !== null) {
            target.forceActiveFocus();
        }
    }

    Connections {
        target: root.Window.window

        function onActiveFocusItemChanged() {
            const focused = root.Window.window ? root.Window.window.activeFocusItem : null;

            for (let at = focused; at; at = at.parent) {
                if (at.parent === toolbarColumn) {
                    root._lastRailItem = at;
                    return;
                }
            }
        }
    }

    function _ensureCursor() {
        if (!root.activeFocus || !root.Window.window) {
            return;
        }

        if (root.Window.window.activeFocusItem === viewLoader) {
            root._focusRail(false);
        }
    }

    function focusFirstItem() {
        if (viewLoader.item !== null && viewLoader.item.focusFirstItem) {
            viewLoader.item.focusFirstItem();
            return;
        }

        root._focusRail(false);
    }

    function enterFocus() {
        const view = viewLoader.item;

        if (view !== null && view.focusCurrentItem !== undefined) {
            view.focusCurrentItem();
            return;
        }

        root._focusRail(false);
    }

    Keys.onPressed: event => {
        if (event.accepted || event.key !== Qt.Key_Left || viewLoader.item.currentIndex !== 0) {
            return;
        }

        // TODO
        // Leaving the very first cell goes to the top of the rail rather than to whatever lies
        // beside it
        root._focusRail(true);
        event.accepted = true;
    }

    function _isOnFirstItem(): bool {
        const view = viewLoader.item;

        if (view === null || view.indexAt === undefined || !root.Window.window) {
            return false;
        }

        return Nav.focusedDelegate(view, root.Window.window.activeFocusItem) === 0;
    }

    property string _pendingViewType: "grid"
    property bool _refreshApplied: false
    property bool _refreshQueued: false

    // TODO
    // The fade commits this, so at rest it must equal what is on screen or a refresh would
    // apply a view mode nobody asked for
    onViewModeChanged: root._pendingViewType = root.viewMode

    FLColumnLayout {
        id: toolbarColumn
        focus: false
        anchors.left: parent.left
        anchors.leftMargin: AppStyle.spacingLg
        anchors.top: parent.top
        anchors.topMargin: root.toolbarTopMargin
        anchors.bottom: parent.bottom
        width: root.toolbarWidth
        spacing: AppStyle.spacingSm
    }

    ColumnLayout {
        id: mainColumn
        anchors.left: toolbarColumn.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.rightMargin: AppStyle.spacingLg
        anchors.leftMargin: AppStyle.spacingXl
        spacing: 0

        Loader {
            id: viewLoader
            Layout.fillHeight: true
            Layout.fillWidth: true
            focus: true
            sourceComponent: {
                if (root.isEmpty) {
                    return root.emptyComponent;
                }

                return root.viewMode === "grid" ? root.gridComponent : root.listComponent;
            }
        }
    }

    SequentialAnimation {
        id: refreshAnimation

        running: false

        onRunningChanged: {
            if (running || !root._refreshQueued) {
                return;
            }

            root._refreshQueued = false;
            root._refreshApplied = false;
            refreshAnimation.start();
        }

        ParallelAnimation {
            NumberAnimation {
                target: viewLoader
                property: "opacity"
                from: 1
                to: 0
                duration: AppStyle.durationSlow
                easing.type: Easing.InOutQuad
            }
            NumberAnimation {
                target: viewLoader
                property: "y"
                from: 0
                to: 12
                duration: AppStyle.durationBase
                easing.type: Easing.InOutQuad
            }
        }

        ScriptAction {
            script: {
                root._refreshApplied = true;
                root.commit();
                root.viewMode = root._pendingViewType;
                root.resetPosition();
            }
        }

        ParallelAnimation {
            NumberAnimation {
                target: viewLoader
                property: "opacity"
                from: 0
                to: 1
                duration: AppStyle.durationSlow
                easing.type: Easing.InOutQuad
            }
            NumberAnimation {
                target: viewLoader
                property: "y"
                from: 12
                to: 0
                duration: AppStyle.durationBase
                easing.type: Easing.InOutQuad
            }
        }
    }
}
