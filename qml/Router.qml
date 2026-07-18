pragma Singleton

import QtQuick
import "routing.js" as Routing

// TODO
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

    property var _state: Routing.initialHistory()

    function navigate(rawPath) {
        var parsed = Routing.parse(rawPath);
        if (Routing.currentPath(_state) === parsed.path) {
            return;
        }
        var transition = Routing.inferTransition(Routing.currentPath(_state), parsed.path, routes);
        _state = Routing.pushHistory(_state, parsed.path);
        _apply(parsed, transition);
    }

    function replace(rawPath) {
        var parsed = Routing.parse(rawPath);
        var transition = Routing.inferTransition(Routing.currentPath(_state), parsed.path, routes);
        _state = Routing.replaceHistory(_state, parsed.path);
        _apply(parsed, transition);
    }

    function back() {
        if (!Routing.canGoBack(_state)) {
            return;
        }
        var from = Routing.currentPath(_state);
        _state = Routing.backHistory(_state);
        var to = Routing.currentPath(_state);
        _apply(Routing.parse(to), Routing.inferTransition(from, to, routes));
    }

    function forward() {
        if (!Routing.canGoForward(_state)) {
            return;
        }
        var from = Routing.currentPath(_state);
        _state = Routing.forwardHistory(_state);
        var to = Routing.currentPath(_state);
        _apply(Routing.parse(to), Routing.inferTransition(from, to, routes));
    }

    function isActive(prefix) {
        return Routing.isActive(path, prefix);
    }

    // Pure matcher exposed so nested outlets can resolve their own sub-paths
    function match(rawPath, patternList) {
        return Routing.match(Routing.parse(rawPath).path, patternList);
    }

    function reset() {
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

    // Compatibility shims for call sites not yet migrated. Removed in Phase 4.
    readonly property string currentRoute: path
    function navigateTo(rawPath) {
        navigate(rawPath);
    }
    function goBack() {
        back();
    }
}
