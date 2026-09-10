// TODO: NEEDS REVIEW
.pragma library

// Pure routing logic: path parsing, pattern matching, transition inference, and
// a history reducer. No QML/engine state here so it stays unit-testable.

// Canonical route table. `stacksOn` marks a drill-down that pushes over its
// parent; `ownsSubtree` marks a screen that handles its own sub-paths internally
// (the top-level view doesn't change as you move within it).
var ROUTES = [
    { pattern: "/library", ownsSubtree: true },
    { pattern: "/library/entries/:entryId", stacksOn: "/library" },
    { pattern: "/library/reorder-collections", stacksOn: "/library" },
    { pattern: "/shop" },
    { pattern: "/shop/mods/:modId", stacksOn: "/shop" },
    { pattern: "/settings", ownsSubtree: true },
    { pattern: "/controllers" },
    { pattern: "/controllers/profiles/:playerNumber", stacksOn: "/controllers" },
    { pattern: "/controllers/manage", stacksOn: "/controllers" },
    { pattern: "/gallery" },
    { pattern: "/gallery/games/:gameContentHash", stacksOn: "/gallery" },
    { pattern: "/activity" },
    { pattern: "/netplay" },
    { pattern: "/help" },
    { pattern: "/dev/gallery" },
    { pattern: "/quick-menu", overlay: true }
];

// TODO
// Per-pair page transitions. A rule matches one concrete move (fromPath -> toPath); either side may
// be "*". `preset` names an enter/exit pair in the PageTransitions catalog. Both directions are
// declared, so going back matches its own reversed-pair rule.
// A rule with "*" on both sides would answer every move on every stack; nothing should declare one
var TRANSITIONS = [
    { from: "/library", to: "/library/collections", preset: "panelForward" },
    { from: "/library/collections", to: "/library", preset: "panelBack" },
    { from: "/library/collections", to: "/library/collections/:collectionId", preset: "panelForward" },
    { from: "/library/collections/:collectionId", to: "/library/collections", preset: "panelBack" },
    { from: "/library", to: "/library/collections/:collectionId", preset: "panelForward" },
    { from: "/library/collections/:collectionId", to: "/library", preset: "panelBack" }
];

// TODO
// Every preset name a rule may use. The catalog's keys mirror this
var TRANSITION_PRESETS = ["none", "push", "pop", "replace", "panelForward", "panelBack"];

function patterns(routes) {
    var out = [];
    for (var i = 0; i < routes.length; i++) {
        out.push(routes[i].pattern);
    }
    return out;
}

function routeFor(pattern, routes) {
    for (var i = 0; i < routes.length; i++) {
        if (routes[i].pattern === pattern) {
            return routes[i];
        }
    }
    return null;
}

// Whether a route is presented as an overlay (a popup over the current view)
// rather than replacing the whole content area.
function routeIsOverlay(pattern, routes) {
    var route = routeFor(pattern, routes);
    return route !== null && route.overlay === true;
}

// --- path + query parsing ---

function segments(path) {
    var out = [];
    var raw = (path || "").split("/");
    for (var i = 0; i < raw.length; i++) {
        if (raw[i].length) {
            out.push(raw[i]);
        }
    }
    return out;
}

function parseQuery(queryString) {
    var out = {};
    if (!queryString) {
        return out;
    }
    var parts = queryString.split("&");
    for (var i = 0; i < parts.length; i++) {
        if (!parts[i]) {
            continue;
        }
        var kv = parts[i].split("=");
        out[decodeURIComponent(kv[0])] = kv.length > 1 ? decodeURIComponent(kv[1]) : "";
    }
    return out;
}

// Normalizes a raw path to a single leading slash, drops empty/trailing slashes,
// decodes segments, and splits off any ?query. Returns { path, query }.
function parse(raw) {
    var path = raw || "";
    var query = {};
    var q = path.indexOf("?");
    if (q >= 0) {
        query = parseQuery(path.substring(q + 1));
        path = path.substring(0, q);
    }
    var segs = segments(path);
    for (var i = 0; i < segs.length; i++) {
        segs[i] = decodeURIComponent(segs[i]);
    }
    return { path: "/" + segs.join("/"), query: query };
}

// --- matching ---

// Matches one pattern against a path. `score` is 1 (literal) / 0 (param) per
// segment, used for precedence.
function matchOne(pattern, path) {
    var ps = segments(pattern);
    var xs = segments(path);
    if (ps.length !== xs.length) {
        return { matched: false, params: {}, score: [] };
    }
    var params = {};
    var score = [];
    for (var i = 0; i < ps.length; i++) {
        if (ps[i].charAt(0) === ":") {
            params[ps[i].substring(1)] = xs[i];
            score.push(0);
        } else if (ps[i] === xs[i]) {
            score.push(1);
        } else {
            return { matched: false, params: {}, score: [] };
        }
    }
    return { matched: true, params: params, score: score };
}

// A literal segment beats a param at the first differing position.
function betterScore(a, b) {
    for (var i = 0; i < a.length; i++) {
        if (a[i] !== b[i]) {
            return a[i] > b[i];
        }
    }
    return false;
}

// Matches a path against patterns honoring precedence (literal beats param, ties
// broken by list order). Returns { matched, pattern, params }.
function match(path, patternList) {
    var bestScore = null;
    var bestPattern = "";
    var bestParams = {};
    for (var i = 0; i < patternList.length; i++) {
        var m = matchOne(patternList[i], path);
        if (!m.matched) {
            continue;
        }
        if (bestScore === null || betterScore(m.score, bestScore)) {
            bestScore = m.score;
            bestPattern = patternList[i];
            bestParams = m.params;
        }
    }
    return { matched: bestScore !== null, pattern: bestPattern, params: bestParams };
}

// True when `prefix` is `path` or an ancestor of it at a segment boundary.
function isActive(path, prefix) {
    return path === prefix || path.indexOf(prefix + "/") === 0;
}

// --- resolution: which component + params a path maps to ---

function subtreeOwner(path, routes) {
    var m = match(path, patterns(routes));
    for (var i = 0; i < routes.length; i++) {
        if (routes[i].ownsSubtree && isActive(path, routes[i].pattern)) {
            // TODO
            // A path with a declared route of its own resolves to that route rather than the owner
            return (m.matched && m.pattern !== routes[i].pattern) ? "" : routes[i].pattern;
        }
    }
    return "";
}

// The component key (pattern) and params to mount for a path. Paths inside a
// subtree-owning route resolve to that owner; the owner screen reads the rest.
function resolve(path, routes) {
    var owner = subtreeOwner(path, routes);
    if (owner) {
        return { pattern: owner, params: {} };
    }
    var m = match(path, patterns(routes));
    return { pattern: m.matched ? m.pattern : "", params: m.params };
}

// --- transition inference ---

// Returns "none" | "push" | "replace" | "pop" for a move from one path to
// another, so call sites don't have to decide.
function inferTransition(fromPath, toPath, routes) {
    if (fromPath === toPath) {
        return "none";
    }

    var toOwner = subtreeOwner(toPath, routes);
    if (toOwner && subtreeOwner(fromPath, routes) === toOwner) {
        return "none";
    }

    var pats = patterns(routes);
    var fromMatch = match(fromPath, pats);
    var toMatch = match(toPath, pats);
    var toRoute = toMatch.matched ? routeFor(toMatch.pattern, routes) : null;
    var fromRoute = fromMatch.matched ? routeFor(fromMatch.pattern, routes) : null;

    if (toRoute && toRoute.stacksOn && fromMatch.matched && toRoute.stacksOn === fromMatch.pattern) {
        return "push";
    }
    if (fromRoute && fromRoute.stacksOn && toMatch.matched && fromRoute.stacksOn === toMatch.pattern) {
        return "pop";
    }
    return "replace";
}

// TODO
// How specific one side of a rule is against a path: 2 all literal, 1 holds a param, 0 the wildcard,
// -1 no match
function transitionSideRank(side, path) {
    if (side === "*") {
        return 0;
    }

    var m = matchOne(side, path);

    if (!m.matched) {
        return -1;
    }

    for (var i = 0; i < m.score.length; i++) {
        if (m.score[i] === 0) {
            return 1;
        }
    }

    return 2;
}

// TODO
// The preset naming how a move should animate, or "" when no rule covers it. The most specific rule
// wins rather than the first declared; a tie goes to the more specific destination, then to
// declaration order
function transitionFor(fromPath, toPath, table) {
    var best = "";
    var bestSum = -1;
    var bestTo = -1;

    for (var i = 0; i < table.length; i++) {
        var fromRank = transitionSideRank(table[i].from, fromPath);
        var toRank = transitionSideRank(table[i].to, toPath);

        if (fromRank < 0 || toRank < 0) {
            continue;
        }

        var sum = fromRank + toRank;

        if (sum > bestSum || (sum === bestSum && toRank > bestTo)) {
            best = table[i].preset;
            bestSum = sum;
            bestTo = toRank;
        }
    }

    return best;
}

// --- history reducer --- state = { entries: [path...], cursor }

function initialHistory() {
    return { entries: [], cursor: -1 };
}

function currentPath(state) {
    return state.cursor >= 0 ? state.entries[state.cursor] : "";
}

function pushHistory(state, path) {
    if (currentPath(state) === path) {
        return state;
    }
    var entries = state.entries.slice(0, state.cursor + 1);
    entries.push(path);
    return { entries: entries, cursor: entries.length - 1 };
}

function replaceHistory(state, path) {
    if (state.cursor < 0) {
        return pushHistory(state, path);
    }
    var entries = state.entries.slice();
    entries[state.cursor] = path;
    return { entries: entries, cursor: state.cursor };
}

function backHistory(state) {
    if (state.cursor <= 0) {
        return state;
    }
    return { entries: state.entries, cursor: state.cursor - 1 };
}

function forwardHistory(state) {
    if (state.cursor >= state.entries.length - 1) {
        return state;
    }
    return { entries: state.entries, cursor: state.cursor + 1 };
}

function canGoBack(state) {
    return state.cursor > 0;
}

function canGoForward(state) {
    return state.cursor < state.entries.length - 1;
}

// --- leave guard --- state = { guard, pending }

// A screen holding unsaved work registers a guard; every move asks it before going anywhere.
// A blocked move is kept so the screen can let it through once the user has answered

function initialGuard() {
    return { guard: null, pending: null };
}

function setGuard(state, fn) {
    return { guard: fn === undefined ? null : fn, pending: null };
}

// Asks the guard whether the move may happen, remembering the move in the same step that refuses it
function attemptLeave(state, kind, arg) {
    if (state.guard === null || state.guard()) {
        return { state: state, allowed: true };
    }

    return { state: { guard: state.guard, pending: { kind: kind, arg: arg } }, allowed: false };
}

// Drops the guard and hands back the move it was blocking
function releaseGuard(state) {
    return { state: initialGuard(), pending: state.pending };
}

// Forgets the blocked move; the guard stays
function forgetPending(state) {
    return { guard: state.guard, pending: null };
}
