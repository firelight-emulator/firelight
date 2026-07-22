import QtQuick
import QtQuick.Controls

// Renders the current route as a screen and mirrors Router's history. Recently
// visited screens are kept alive in an LRU cache so navigating back returns to a
// page exactly as it was left (scroll position, selection, sub-tab). Router owns
// history; this view only reacts to the transition it hands us
//
// StackView never destroys items it didn't create itself, so we build the
// screens ourselves and manage their lifecycle here: the stack only ever holds
// the current one (depth 1); the rest live hidden in the cache until reused or
// evicted. First-time builds are asynchronous (incubated across frames) so
// opening a heavy page doesn't freeze the UI; cached revisits are instant
//
// Pages that do ongoing work (timers, animations, media) can pause it while
// off-screen by declaring `property bool routeActive: true` — this view sets it
// false when the page isn't current. e.g. `Timer { running: root.routeActive }`
StackView {
    id: stack

    padding: 0
    initialItem: Item {}

    // How many screen instances to keep alive (current + recent). Older ones are
    // destroyed and rebuilt fresh when revisited
    property int cacheSize: 5

    // True while an uncached page is being built asynchronously, so the shell can
    // show a loading indicator
    property bool loading: false

    // Emitted when a screen fails to build
    signal routeError(string path, string error)

    // Route pattern -> screen Component. Params (Router.params) are passed as
    // initial properties, so a pattern's :name captures must match the screen's
    // property names (e.g. :entryId -> FLGameDetailsPanel.entryId)
    readonly property var routes: ({
            "/library": libraryComponent,
            "/library/entries/:entryId": gameDetailsComponent,
            "/shop": shopComponent,
            "/shop/mods/:modId": shopItemComponent,
            "/settings": settingsComponent,
            "/controllers": controllersComponent,
            "/controllers/profiles/:playerNumber": controllerProfileComponent,
            "/controllers/manage": profileManagementComponent,
            "/gallery": galleryComponent,
            "/gallery/games/:gameContentHash": galleryComponent,
            "/activity": activityComponent,
            "/netplay": netplayComponent,
            "/help": helpComponent,
            "/dev/gallery": devGalleryComponent
        })

    Component {
        id: libraryComponent
        LibraryPageV2 {}
    }
    Component {
        id: gameDetailsComponent
        FLGameDetailsPanel {}
    }
    Component {
        id: shopComponent
        ShopLandingPage {}
    }
    Component {
        id: shopItemComponent
        ShopItemPage {}
    }
    Component {
        id: settingsComponent
        SettingsScreen {}
    }
    Component {
        id: controllersComponent
        ControllersPage {}
    }
    Component {
        id: controllerProfileComponent
        ControllerProfilePage {}
    }
    Component {
        id: profileManagementComponent
        ProfileManagementPage {}
    }
    Component {
        id: galleryComponent
        GalleryPage {}
    }
    Component {
        id: activityComponent
        ActivityPageV2 {}
    }
    Component {
        id: netplayComponent
        NetplayPage {}
    }
    Component {
        id: helpComponent
        HelpScreen {}
    }
    Component {
        id: devGalleryComponent
        ComponentGallery {}
    }

    Component {
        id: notFoundComponent
        Item {
            objectName: "RouteNotFound"

            Text {
                anchors.centerIn: parent
                color: "#FFFFFF"
                font.family: Constants.regularFontFamily
                font.pixelSize: AppStyle.fontSizeMedium
                text: "Page not found: " + Router.path
            }
        }
    }

    pushEnter: Transition {
        ParallelAnimation {
            PropertyAnimation {
                property: "opacity"
                from: 0
                to: 1
                duration: 160
                easing.type: Easing.InOutQuad
            }
            PropertyAnimation {
                property: "x"
                from: 20
                to: 0
                duration: 160
                easing.type: Easing.InOutQuad
            }
        }
    }
    pushExit: Transition {
        ParallelAnimation {
            PropertyAnimation {
                property: "opacity"
                from: 1
                to: 0
                duration: 160
                easing.type: Easing.InOutQuad
            }
            PropertyAnimation {
                property: "x"
                from: 0
                to: -20
                duration: 160
                easing.type: Easing.InOutQuad
            }
        }
    }

    popEnter: Transition {
        ParallelAnimation {
            PropertyAnimation {
                property: "opacity"
                from: 0
                to: 1
                duration: 160
                easing.type: Easing.InOutQuad
            }
            PropertyAnimation {
                property: "x"
                from: -20
                to: 0
                duration: 160
                easing.type: Easing.InOutQuad
            }
        }
    }

    popExit: Transition {
        ParallelAnimation {
            PropertyAnimation {
                property: "opacity"
                from: 1
                to: 0
                duration: 160
                easing.type: Easing.InOutQuad
            }
            PropertyAnimation {
                property: "x"
                from: 0
                to: 20
                duration: 160
                easing.type: Easing.InOutQuad
            }
        }
    }

    replaceEnter: Transition {}
    replaceExit: Transition {}

    // Off-screen, invisible parent where pages are built asynchronously before
    // they're shown. Sized like the stack so layout matches their final home
    Item {
        id: incubationHolder
        visible: false
        width: stack.width
        height: stack.height
    }

    // LRU cache of { key, item }, most-recently-used last
    property var _cache: []
    // Builds currently in flight, keyed by mount key, so a route isn't built twice
    property var _incubators: ({})

    // A route pattern that owns its subtree (e.g. /settings) keeps one instance
    // for all its sub-paths; everything else is keyed by full path so distinct
    // entries get distinct pages
    function _isSubtree(pattern) {
        for (var i = 0; i < Router.routes.length; i++) {
            if (Router.routes[i].pattern === pattern) {
                return Router.routes[i].ownsSubtree === true;
            }
        }
        return false;
    }

    function _mountKey() {
        return _isSubtree(Router.matchedPattern) ? Router.matchedPattern : Router.path;
    }

    // Path params plus query params, as initial properties for the page
    // (e.g. /controllers/profiles/2?profileId=5 -> {playerNumber, profileId})
    function _props() {
        var props = {};
        var key;
        for (key in Router.params) {
            props[key] = Router.params[key];
        }
        for (key in Router.query) {
            props[key] = Router.query[key];
        }
        return props;
    }

    function _cacheIndex(key) {
        for (var i = 0; i < _cache.length; i++) {
            if (_cache[i].key === key) {
                return i;
            }
        }
        return -1;
    }

    function _cacheTouch(key, item) {
        var idx = _cacheIndex(key);
        if (idx >= 0) {
            _cache.splice(idx, 1);
        }
        _cache.push({
            key: key,
            item: item
        });
        _evict();
    }

    function _evict() {
        while (_cache.length > cacheSize) {
            var victim = _cache[0];
            if (victim.item === stack.currentItem) {
                break; // never destroy what's on screen
            }
            _cache.shift();
            if (victim.item) {
                victim.item.destroy();
            }
        }
    }

    function _setActive(item, active) {
        if (item && item.routeActive !== undefined) {
            item.routeActive = active;
        }
    }

    function _show(item, transition) {
        stack.loading = false;
        // Keep the depth-1 invariant: if a screen ever pushes onto our stack
        // directly (outside the router), drop those extras so nothing lingers
        // behind the next page
        while (stack.depth > 1) {
            stack.popCurrentItem(StackView.Immediate);
        }
        if (stack.currentItem === item) {
            _setActive(item, true);
            return;
        }
        var previous = stack.currentItem;
        var op = transition === "push" ? StackView.PushTransition : transition === "pop" ? StackView.PopTransition : StackView.ReplaceTransition;
        stack.replaceCurrentItem(item, {}, op);
        _setActive(previous, false);
        _setActive(item, true);
    }

    Connections {
        target: Router
        function onNavigated(transition) {
            if (Router.overlay || transition === "none") {
                // Overlay routes render on top (handled by RouteOverlay); a subtree
                // screen updates itself in place. Either way, leave content as-is
                return;
            }

            var key = stack._mountKey();
            var idx = stack._cacheIndex(key);
            if (idx >= 0) {
                var item = stack._cache[idx].item;
                stack._cache.splice(idx, 1);
                stack._cache.push({
                    key: key,
                    item: item
                });
                stack._show(item, transition);
                return;
            }

            // Cache miss: build the page asynchronously so navigation stays smooth
            stack.loading = true;
            if (stack._incubators[key]) {
                return; // already building
            }
            var component = stack.routes[Router.matchedPattern] || notFoundComponent;
            var incubator = component.incubateObject(incubationHolder, stack._props(), Qt.Asynchronous);
            stack._incubators[key] = incubator;

            var finish = function () {
                if (incubator.status === Component.Ready) {
                    delete stack._incubators[key];
                    stack._cacheTouch(key, incubator.object);
                    if (stack._mountKey() === key) {
                        stack._show(incubator.object, transition);
                    }
                } else if (incubator.status === Component.Error) {
                    delete stack._incubators[key];
                    stack.loading = false;
                    var reason = component.errorString();
                    console.error("Route failed to build:", Router.path, reason);
                    stack.routeError(Router.path, reason);
                }
            };

            if (incubator.status === Component.Ready || incubator.status === Component.Error) {
                finish();
            } else {
                incubator.onStatusChanged = finish;
            }
        }
    }
}
