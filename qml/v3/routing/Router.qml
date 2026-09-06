// TODO: NEEDS REVIEW
pragma Singleton

import QtQuick
import "routing.js" as Routing

// App-wide URL navigation. Single source of truth for the current path, params,
// and history; the view layer (RouteView, nav popup) mirrors it. All logic lives
// in routing.js — this only holds state and emits change notifications
QtObject {
    id: root

    readonly property var routes: Routing.ROUTES

    property string path: ""
    property var params: ({})
    property var query: ({})
    property string matchedPattern: ""
    property bool canGoBack: false
    property bool canGoForward: false

    // True when the current route is presented as an overlay/popup over whatever
    // view is behind it, rather than replacing the content area
    readonly property bool overlay: Routing.routeIsOverlay(matchedPattern, routes)

    // "none" | "push" | "replace" | "pop" — how the view should animate the move
    signal navigated(string transition)

    property var _guard: Routing.initialGuard()

    // TODO
    // A screen with unsaved work registers a function returning false to refuse to be left. Every
    // move asks it, whichever control asked for the move
    function setLeaveGuard(fn) {
        root._guard = Routing.setGuard(root._guard, fn);
    }

    function clearLeaveGuard() {
        root._guard = Routing.setGuard(root._guard, null);
    }

    function _allowed(kind, arg) {
        const result = Routing.attemptLeave(root._guard, kind, arg);

        root._guard = result.state;
        return result.allowed;
    }

    // TODO
    // Drops the guard and lets the blocked move happen
    function resumePending() {
        const released = Routing.releaseGuard(root._guard);

        root._guard = released.state;

        if (released.pending === null) {
            return;
        }

        if (released.pending.kind === "navigate") {
            root.navigate(released.pending.arg);
        } else if (released.pending.kind === "replace") {
            root.replace(released.pending.arg);
        } else if (released.pending.kind === "back") {
            root.back();
        } else if (released.pending.kind === "forward") {
            root.forward();
        }
    }

    // TODO
    // Forgets the blocked move; the screen stays where it is and keeps its guard
    function cancelPending() {
        root._guard = Routing.forgetPending(root._guard);
    }

    property var _state: Routing.initialHistory()

    // TODO
    // Each of these answers whether the move happened, so a caller can tell a refusal from a move
    function navigate(rawPath) {
        var parsed = Routing.parse(rawPath);
        if (Routing.currentPath(_state) === parsed.path) {
            return false;
        }
        if (!root._allowed("navigate", rawPath)) {
            return false;
        }
        var transition = Routing.inferTransition(Routing.currentPath(_state), parsed.path, routes);
        _state = Routing.pushHistory(_state, parsed.path);
        _apply(parsed, transition);
        return true;
    }

    function replace(rawPath) {
        if (!root._allowed("replace", rawPath)) {
            return false;
        }
        var parsed = Routing.parse(rawPath);
        var transition = Routing.inferTransition(Routing.currentPath(_state), parsed.path, routes);
        _state = Routing.replaceHistory(_state, parsed.path);
        _apply(parsed, transition);
        return true;
    }

    function back() {
        if (!Routing.canGoBack(_state)) {
            return false;
        }
        if (!root._allowed("back", null)) {
            return false;
        }
        var from = Routing.currentPath(_state);
        _state = Routing.backHistory(_state);
        var to = Routing.currentPath(_state);
        _apply(Routing.parse(to), Routing.inferTransition(from, to, routes));
        return true;
    }

    function forward() {
        if (!Routing.canGoForward(_state)) {
            return false;
        }
        if (!root._allowed("forward", null)) {
            return false;
        }
        var from = Routing.currentPath(_state);
        _state = Routing.forwardHistory(_state);
        var to = Routing.currentPath(_state);
        _apply(Routing.parse(to), Routing.inferTransition(from, to, routes));
        return true;
    }

    function isActive(prefix) {
        return Routing.isActive(path, prefix);
    }

    // Pure matcher exposed so nested outlets can resolve their own sub-paths
    function match(rawPath, patternList) {
        return Routing.match(Routing.parse(rawPath).path, patternList);
    }

    function reset() {
        _guard = Routing.initialGuard();
        _state = Routing.initialHistory();
        path = "";
        params = ({});
        query = ({});
        matchedPattern = "";
        canGoBack = false;
        canGoForward = false;
    }

    function _apply(parsed, transition) {
        var resolved = Routing.resolve(parsed.path, routes);
        path = parsed.path;
        query = parsed.query;
        params = resolved.params;
        matchedPattern = resolved.pattern;
        canGoBack = Routing.canGoBack(_state);
        canGoForward = Routing.canGoForward(_state);
        navigated(transition);
    }

    // Compatibility shims for call sites not yet migrated. Removed in Phase 4
    readonly property string currentRoute: path
    function navigateTo(rawPath) {
        return navigate(rawPath);
    }
    function goBack() {
        return back();
    }
}
