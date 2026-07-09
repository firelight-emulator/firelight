import QtQuick
import QtTest
// Absolute qrc path so the import resolves both when this file runs from the
// compiled module and when QuickTest loads it directly from the source tree.
import "qrc:/qt/qml/QMLFirelightTest/routing.js" as R

TestCase {
    name: "RoutingLogicTests"

    property var routes: R.ROUTES

    // --- path + query parsing ---

    function test_parse_normalizes_slashes() {
        compare(R.parse("library").path, "/library");
        compare(R.parse("/library/").path, "/library");
        compare(R.parse("//library//entries//5//").path, "/library/entries/5");
        compare(R.parse("").path, "/");
    }

    function test_parse_splits_query() {
        var p = R.parse("/gallery?filter=snes&sort=name");
        compare(p.path, "/gallery");
        compare(p.query.filter, "snes");
        compare(p.query.sort, "name");
    }

    function test_parse_decodes_segments() {
        compare(R.parse("/library/entries/a%20b").path, "/library/entries/a b");
    }

    // --- matching + params ---

    function test_match_extracts_named_params() {
        var m = R.match("/library/entries/42", R.patterns(routes));
        verify(m.matched);
        compare(m.pattern, "/library/entries/:entryId");
        compare(m.params.entryId, "42");
    }

    function test_match_arity_mismatch_fails() {
        var m = R.match("/library/entries", R.patterns(routes));
        verify(!m.matched);
    }

    function test_match_unknown_fails() {
        var m = R.match("/bogus/path", R.patterns(routes));
        verify(!m.matched);
    }

    function test_route_overlay_flag() {
        verify(R.routeIsOverlay("/settings", routes));
        verify(!R.routeIsOverlay("/library", routes));
        verify(!R.routeIsOverlay("/bogus", routes));
    }

    function test_match_literal_beats_param() {
        var pats = ["/gallery/games/:contentHash", "/gallery/games/new"];
        var m = R.match("/gallery/games/new", pats);
        verify(m.matched);
        compare(m.pattern, "/gallery/games/new");
    }

    // --- isActive segment boundaries ---

    function test_isActive_boundaries() {
        verify(R.isActive("/settings", "/settings"));
        verify(R.isActive("/settings/appearance", "/settings"));
        verify(!R.isActive("/settings-foo", "/settings"));
        verify(!R.isActive("/set", "/settings"));
    }

    // --- resolution (subtree owner) ---

    function test_resolve_subtree_child_maps_to_owner() {
        var r = R.resolve("/settings/appearance", routes);
        compare(r.pattern, "/settings");
    }

    function test_resolve_drilldown_keeps_params() {
        var r = R.resolve("/library/entries/7", routes);
        compare(r.pattern, "/library/entries/:entryId");
        compare(r.params.entryId, "7");
    }

    // --- transition inference ---

    function test_transition_peer_is_replace() {
        compare(R.inferTransition("/library", "/shop", routes), "replace");
    }

    function test_transition_drilldown_is_push() {
        compare(R.inferTransition("/library", "/library/entries/5", routes), "push");
    }

    function test_transition_backup_is_pop() {
        compare(R.inferTransition("/library/entries/5", "/library", routes), "pop");
    }

    function test_transition_within_subtree_is_none() {
        compare(R.inferTransition("/settings", "/settings/appearance", routes), "none");
        compare(R.inferTransition("/settings/appearance", "/settings/audio", routes), "none");
    }

    function test_transition_initial_navigation_is_replace() {
        compare(R.inferTransition("", "/library", routes), "replace");
    }

    function test_transition_cross_section_deeplink_is_replace() {
        compare(R.inferTransition("/library", "/settings/appearance", routes), "replace");
    }

    function test_transition_controller_profile_drilldown() {
        compare(R.inferTransition("/controllers", "/controllers/profiles/2", routes), "push");
        compare(R.inferTransition("/controllers/profiles/2", "/controllers", routes), "pop");
        compare(R.inferTransition("/controllers", "/controllers/manage", routes), "push");
    }

    // --- history reducer ---

    function test_history_navigate_and_back_forward() {
        var s = R.initialHistory();
        s = R.pushHistory(s, "/library");
        s = R.pushHistory(s, "/shop");
        s = R.pushHistory(s, "/settings");
        compare(R.currentPath(s), "/settings");
        verify(R.canGoBack(s));
        verify(!R.canGoForward(s));

        s = R.backHistory(s);
        compare(R.currentPath(s), "/shop");
        verify(R.canGoForward(s));

        s = R.forwardHistory(s);
        compare(R.currentPath(s), "/settings");
    }

    function test_history_navigate_truncates_forward() {
        var s = R.initialHistory();
        s = R.pushHistory(s, "/library");
        s = R.pushHistory(s, "/shop");
        s = R.backHistory(s);
        s = R.pushHistory(s, "/gallery");
        compare(R.currentPath(s), "/gallery");
        verify(!R.canGoForward(s));
        // /shop was truncated by the new branch
        s = R.backHistory(s);
        compare(R.currentPath(s), "/library");
    }

    function test_history_replace_does_not_grow() {
        var s = R.initialHistory();
        s = R.pushHistory(s, "/settings/appearance");
        s = R.replaceHistory(s, "/settings/audio");
        compare(R.currentPath(s), "/settings/audio");
        verify(!R.canGoBack(s));
    }

    function test_history_same_path_is_noop() {
        var s = R.initialHistory();
        s = R.pushHistory(s, "/library");
        var before = s.entries.length;
        s = R.pushHistory(s, "/library");
        compare(s.entries.length, before);
    }

    function test_history_back_at_root_noop() {
        var s = R.initialHistory();
        s = R.pushHistory(s, "/library");
        verify(!R.canGoBack(s));
        s = R.backHistory(s);
        compare(R.currentPath(s), "/library");
    }
}
