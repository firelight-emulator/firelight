.pragma library

// Pure routing logic: path parsing, pattern matching, transition inference, and
// a history reducer. No QML/engine state here so it stays unit-testable.

// Canonical route table. `stacksOn` marks a drill-down that pushes over its
// parent; `ownsSubtree` marks a screen that handles its own sub-paths internally
// (the top-level view doesn't change as you move within it).
var ROUTES = [
    { pattern: "/library" },
    { pattern: "/library/entries/:entryId", stacksOn: "/library" },
    { pattern: "/shop" },
    { pattern: "/shop/mods/:modId", stacksOn: "/shop" },
    { pattern: "/settings", ownsSubtree: true, overlay: true },
    { pattern: "/controllers" },
    { pattern: "/controllers/profiles/:playerNumber", stacksOn: "/controllers" },
    { pattern: "/controllers/manage", stacksOn: "/controllers" },
    { pattern: "/gallery" },
    { pattern: "/gallery/games/:gameContentHash", stacksOn: "/gallery" },
    { pattern: "/activity" },
    { pattern: "/netplay" },
    { pattern: "/help" },
    { pattern: "/quick-menu", overlay: true }
];

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
    for (var i = 0; i < routes.length; i++) {
        if (routes[i].ownsSubtree && isActive(path, routes[i].pattern)) {
            return routes[i].pattern;
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
