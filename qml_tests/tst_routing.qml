// TODO: NEEDS REVIEW
import QtQuick
import QtTest
// Absolute qrc path so the import resolves both when this file runs from the
// compiled module and when QuickTest loads it directly from the source tree
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

    // TODO
    // /library owns its subtree, so a collection path resolves to the library screen, which
    // reads the rest of the path itself
    function test_resolve_library_collection_maps_to_owner() {
        var r = R.resolve("/library/collections/7", routes);
        compare(r.pattern, "/library");
    }

    // TODO
    // The entries drill-down is declared, so subtree ownership does not claim it
    function test_resolve_declared_drilldown_beats_subtree_owner() {
        var r = R.resolve("/library/entries/7", routes);
        compare(r.pattern, "/library/entries/:entryId");
        compare(r.params.entryId, "7");
    }

    // TODO
    // Declaring a pattern under /library waives that subtree's ownership, which is the only reason
    // a sibling screen can be routed to at all
    function test_resolve_declared_sibling_beats_subtree_owner() {
        var r = R.resolve("/library/reorder-collections", routes);
        compare(r.pattern, "/library/reorder-collections");
    }

    function test_resolve_declared_param_sibling_beats_subtree_owner() {
        var r = R.resolve("/library/create-collection/smart", routes);
        compare(r.pattern, "/library/create-collection/:kind");
        compare(r.params.kind, "smart");

        var collection = R.resolve("/library/collections/7", routes);
        compare(collection.pattern, "/library");
    }

    // The hazard the path shape avoids: a tail that is not a number would resolve as a collection id
    function test_reorder_path_is_not_read_as_a_collection() {
        var m = R.match("/library/reorder-collections", ["/library/collections/:collectionId"]);
        verify(!m.matched);
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

    function test_transition_within_library_subtree_is_none() {
        compare(R.inferTransition("/library", "/library/collections", routes), "none");
        compare(R.inferTransition("/library/collections/7", "/library/collections", routes), "none");
    }

    function test_transition_into_a_library_sibling_is_replace() {
        compare(R.inferTransition("/library/collections", "/library/reorder-collections", routes), "replace");
        compare(R.inferTransition("/library/reorder-collections", "/library/collections", routes), "replace");
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

    // --- per-pair transitions ---

    property var seedTransitions: R.TRANSITIONS

    function test_transition_rule_wildcard_matches_anything() {
        var table = [
            {
                from: "*",
                to: "*",
                preset: "wild"
            }
        ];

        compare(R.transitionFor("/library", "/shop", table), "wild");
        compare(R.transitionFor("", "/library", table), "wild");
    }

    function test_transition_rule_most_specific_wins_regardless_of_order() {
        var general = {
            from: "*",
            to: "*",
            preset: "wild"
        };
        var half = {
            from: "*",
            to: "/shop",
            preset: "half"
        };
        var exact = {
            from: "/library",
            to: "/shop",
            preset: "exact"
        };

        compare(R.transitionFor("/library", "/shop", [general, half, exact]), "exact");
        compare(R.transitionFor("/library", "/shop", [exact, half, general]), "exact");
        compare(R.transitionFor("/gallery", "/shop", [general, half, exact]), "half");
        compare(R.transitionFor("/gallery", "/activity", [general, half, exact]), "wild");
    }

    function test_transition_rule_matches_a_param_pattern() {
        var table = [
            {
                from: "*",
                to: "/library/collections/:collectionId",
                preset: "param"
            }
        ];

        compare(R.transitionFor("/library", "/library/collections/42", table), "param");
        compare(R.transitionFor("/library", "/library/collections", table), "");
    }

    function test_transition_rule_literal_beats_param() {
        var table = [
            {
                from: "*",
                to: "/library/collections/:collectionId",
                preset: "param"
            },
            {
                from: "*",
                to: "/library/collections/new",
                preset: "literal"
            }
        ];

        compare(R.transitionFor("/library", "/library/collections/new", table), "literal");
        compare(R.transitionFor("/library", "/library/collections/42", table), "param");
    }

    function test_transition_rule_tie_goes_to_the_destination() {
        var table = [
            {
                from: "/library",
                to: "*",
                preset: "fromExact"
            },
            {
                from: "*",
                to: "/shop",
                preset: "toExact"
            }
        ];

        compare(R.transitionFor("/library", "/shop", table), "toExact");
    }

    function test_transition_rule_even_tie_goes_to_declaration_order() {
        var table = [
            {
                from: "*",
                to: "/shop",
                preset: "first"
            },
            {
                from: "*",
                to: "/shop",
                preset: "second"
            }
        ];

        compare(R.transitionFor("/library", "/shop", table), "first");
    }

    function test_transition_rule_no_match_is_empty() {
        compare(R.transitionFor("/library", "/shop", []), "");
        compare(R.transitionFor("/library", "/shop", [
            {
                from: "/gallery",
                to: "*",
                preset: "nope"
            }
        ]), "");
    }

    // TODO
    // How the route layer composes the two: a rule wins, otherwise the structural inference stands
    function test_transition_rule_falls_back_to_inference() {
        compare(R.transitionFor("/library", "/shop", seedTransitions) || R.inferTransition("/library", "/shop", routes), "replace");
        compare(R.transitionFor("/controllers", "/controllers/manage", seedTransitions) || R.inferTransition("/controllers", "/controllers/manage", routes), "push");
    }

    // TODO
    // The seeded rules are what keep the library panels animating, so an edit to them fails here
    // rather than silently going still
    function test_seeded_rules_cover_every_library_move() {
        compare(R.transitionFor("/library", "/library/collections", seedTransitions), "panelForward");
        compare(R.transitionFor("/library/collections", "/library/collections/7", seedTransitions), "panelForward");
        compare(R.transitionFor("/library", "/library/collections/7", seedTransitions), "panelForward");
        compare(R.transitionFor("/library/collections", "/library", seedTransitions), "panelBack");
        compare(R.transitionFor("/library/collections/7", "/library/collections", seedTransitions), "panelBack");
        compare(R.transitionFor("/library/collections/7", "/library", seedTransitions), "panelBack");
    }

    function test_seeded_rules_name_known_presets() {
        for (var i = 0; i < seedTransitions.length; i++) {
            verify(R.TRANSITION_PRESETS.indexOf(seedTransitions[i].preset) !== -1, "unknown preset: " + seedTransitions[i].preset);
        }
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

    // A blocked move must be distinguishable from one that happened, which is what lets a key
    // decline its own sound
    function test_attempt_reports_whether_the_move_happened() {
        var allowed = R.attemptLeave(R.initialGuard(), "back", null);
        verify(allowed.allowed);

        var refusing = R.setGuard(R.initialGuard(), function () {
            return false;
        });
        verify(!R.attemptLeave(refusing, "back", null).allowed);
    }

    // --- leave guard ---

    function test_guard_absent_allows_everything() {
        var g = R.initialGuard();
        var r = R.attemptLeave(g, "back", null);
        verify(r.allowed);
        compare(r.state.pending, null);
    }

    function test_guard_that_agrees_allows_and_remembers_nothing() {
        var g = R.setGuard(R.initialGuard(), function () {
            return true;
        });
        var r = R.attemptLeave(g, "navigate", "/shop");
        verify(r.allowed);
        compare(r.state.pending, null);
    }

    function test_guard_that_refuses_blocks_and_remembers_the_move() {
        var g = R.setGuard(R.initialGuard(), function () {
            return false;
        });
        var r = R.attemptLeave(g, "navigate", "/shop");
        verify(!r.allowed);
        compare(r.state.pending.kind, "navigate");
        compare(r.state.pending.arg, "/shop");
        // the guard survives, so a second attempt is refused too
        verify(!R.attemptLeave(r.state, "back", null).allowed);
    }

    // Releasing is what the confirm dialog does on Discard: the guard goes and the move comes back
    function test_release_drops_the_guard_and_returns_the_move() {
        var g = R.setGuard(R.initialGuard(), function () {
            return false;
        });
        var blocked = R.attemptLeave(g, "back", null).state;

        var released = R.releaseGuard(blocked);
        compare(released.pending.kind, "back");
        compare(released.state.guard, null);
        compare(released.state.pending, null);
        // nothing refuses any more
        verify(R.attemptLeave(released.state, "back", null).allowed);
    }

    // Cancelling keeps the guard, so the next attempt is refused again
    function test_cancel_forgets_the_move_but_keeps_the_guard() {
        var g = R.setGuard(R.initialGuard(), function () {
            return false;
        });
        var blocked = R.attemptLeave(g, "navigate", "/shop").state;

        var cancelled = R.forgetPending(blocked);
        compare(cancelled.pending, null);
        verify(!R.attemptLeave(cancelled, "back", null).allowed);
    }

    function test_setting_a_guard_clears_any_remembered_move() {
        var g = R.setGuard(R.initialGuard(), function () {
            return false;
        });
        var blocked = R.attemptLeave(g, "back", null).state;

        compare(R.setGuard(blocked, null).pending, null);
        compare(R.setGuard(blocked, null).guard, null);
    }

    // Releasing when nothing was blocked must not invent a move to replay
    function test_release_with_nothing_pending_returns_null() {
        compare(R.releaseGuard(R.initialGuard()).pending, null);
    }

    function test_history_back_at_root_noop() {
        var s = R.initialHistory();
        s = R.pushHistory(s, "/library");
        verify(!R.canGoBack(s));
        s = R.backHistory(s);
        compare(R.currentPath(s), "/library");
    }
}
