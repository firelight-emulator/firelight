import QtQuick
import Firelight 1.0

// Global controller-focus ring: one overlay-positioned FLFocusRing that glides
// between focused items and tracks 1:1 while content scrolls under it. Item
// scale/lift is handled by the items themselves, not here.
Item {
    id: root

    required property Item target
    required property bool usingMouse

    // TODO
    // How the ring moves between items: "jump" crossfades out of the old item
    // and in on the new one (the flickable then scrolls under it), "glide"
    // flies the ring across
    property string moveMode: "jump"

    // TODO
    // Space kept between the focused item and its flickable's edges when the
    // cursor drives scrolling
    property real scrollMarginTop: 100
    property real scrollMarginBottom: 100

    // Extra room around the ring so its band and antialiasing never clip
    readonly property real ringPad: AppStyle.focusRingWidth + 8

    // TODO
    // Landings closer together than this scroll as one continuous run rather
    // than as separate steps
    readonly property real cruiseInterval: 0.25

    z: 1000

    // TODO
    // Blink: the ring fades out and stays gone until the blink ends, then
    // reappears wherever focus is by then — so focus moving behind a closing
    // popup or a page transition doesn't drag a ring across the screen
    readonly property bool blinking: _blinking
    property bool _blinking: false

    // TODO
    // Hides the ring until endBlink()
    function startBlink() {
        blinkTimer.stop();
        root._blinking = true;
        ringA.opacity = 0;
        ringB.opacity = 0;
    }

    // TODO
    // Brings the ring back on whatever holds focus now, with no travel from
    // where it used to be
    function endBlink() {
        blinkTimer.stop();

        if (!root._blinking) {
            return;
        }

        root._blinking = false;

        if (root.cursorItem === null) {
            return;
        }

        root.applyScrollNow();
        root.snapNow();
        root.activeRing.opacity = 1;
    }

    // TODO
    // Hides the ring for `duration` ms, or durationBase when none is given
    function blink(duration: int) {
        root.startBlink();
        blinkTimer.interval = duration > 0 ? duration : AppStyle.durationBase;
        blinkTimer.start();
    }

    Timer {
        id: blinkTimer
        onTriggered: root.endBlink()
    }

    // TODO
    // Edge bump: a damped spring offset added on top of the ring geometry. A
    // bump kicks it outward along the pressed direction; the spring carries it
    // out, back through rest, one overshoot past it, and settles. The whole
    // ring translates; the leading edge travels bumpStretch further (0 = even)
    property real bumpStretch: 0.3
    property real _bumpPos: 0
    property real _bumpVel: 0
    property real _bumpDirX: 0
    property real _bumpDirY: 0

    function _bump(dx: real, dy: real) {
        if (cursorItem === null) {
            return;
        }

        _bumpDirX = dx;
        _bumpDirY = dy;
        _bumpVel = AppStyle.focusBumpKick;
        SoundEffects.cursorBump.play();
    }

    function bumpLeft() {
        _bump(-1, 0);
    }

    function bumpRight() {
        _bump(1, 0);
    }

    function bumpUp() {
        _bump(0, -1);
    }

    function bumpDown() {
        _bump(0, 1);
    }

    // TODO
    // What the focus target declares about itself, or null when it declares
    // nothing. Deliberately a function rather than a bound property: cursorItem
    // and the geometry read it too, and a binding they all depend on is not
    // guaranteed to be re-evaluated before they are — which showed up as an item
    // playing the sound of whatever held focus before it
    function targetInfo(): FLFocus {
        return root.target ? root.FLFocus.find(root.target) : null;
    }

    // The item the ring should surround: the focus target (or its opted-in
    // proxy), or null when nothing focusable is focused or the mouse is in use
    readonly property Item cursorItem: {
        if (!target || usingMouse) {
            return null;
        }

        const info = root.targetInfo();

        if (info === null || !info.showCursor) {
            return null;
        }

        return info.proxy !== null ? info.proxy : root.target;
    }

    // TODO
    // Scroll state: the focused item's flickable ancestor and the contentY that
    // puts the item within the scroll margins. In "jump" mode the ring lands on
    // the item first and rides it while the scroll animates; in "glide" mode the
    // ring chases the item's FINAL position (live rect corrected by the
    // remaining scroll), so it never dives toward a pre-scroll position
    property Item _flick: null
    property real _scrollTo: 0
    property bool _scrollPending: false

    // TODO
    // Tween state for the single-step scroll (fixed-duration quadratic ease-out)
    // and cruise state for held repeats (constant velocity at the repeat
    // cadence, landing with a velocity-matched tween)
    property real _scrollFrom: 0
    property real _scrollT: 0
    property bool _cruising: false
    property real _cruiseVel: 0
    property real _cruiseInterval: 0
    property real _clock: 0
    property real _lastChangeAt: -1

    function findFlickableAncestor(item: Item): Item {
        var p = item.parent;
        while (p) {
            if (p.hasOwnProperty("flickableDirection") && p.hasOwnProperty("interactive") && p.interactive) {
                return p;
            }
            p = p.parent;
        }
        return null;
    }

    // TODO
    // Where the container has to scroll to for the cursor to sit comfortably.
    // Arriving from outside it asks for no room at all: the item landed on is
    // already fully on screen, and dragging the container along is not what
    // moving into it should do
    function computeScrollTarget(withMargins: bool): real {
        const above = withMargins ? scrollMarginTop : 0;
        const below = withMargins ? scrollMarginBottom : 0;
        const itemTop = cursorItem.mapToItem(_flick.contentItem, 0, 0).y;
        let desired = _flick.contentY;

        if (itemTop - above < desired) {
            desired = itemTop - above;
        } else if (itemTop + cursorItem.height + below > desired + _flick.height) {
            desired = itemTop + cursorItem.height + below - _flick.height;
        }

        const lo = _flick.originY;
        const hi = Math.max(lo, lo + _flick.contentHeight - _flick.height);
        return Math.max(lo, Math.min(hi, desired));
    }

    function applyScrollNow() {
        if (_scrollPending || _cruising) {
            _flick.contentY = _scrollTo;
            _scrollPending = false;
            _cruising = false;
        }
    }

    // TODO
    // Gap between the item and the ring's inner edge, 0 for a flush outer
    // border. Taken from whatever declared it rather than from `item`, since the
    // ring may be drawn around a proxy that declared nothing
    function spacingFor(item: Item): real {
        const info = root.targetInfo();

        if (info !== null && !isNaN(info.spacing)) {
            return info.spacing;
        }

        return 0;
    }

    // TODO
    // How far the band's centreline sits outside the item: the gap asked for,
    // plus half the band, plus the half pixel the shader fades over so none of
    // that fade lands on the item. Whole pixels, so the ring sits on the grid
    function ringOffsetFor(item: Item): real {
        return Math.ceil(spacingFor(item) + AppStyle.focusRingWidth / 2 + 0.5);
    }

    // TODO
    // What to paint between the item and the ring, from whatever declared it,
    // else the theme's default. Transparent means there is nothing to paint
    function fillFor(item: Item): color {
        const info = root.targetInfo();

        if (info !== null && info.fill.a > 0) {
            return info.fill;
        }

        return Theme.focusRingFill;
    }

    // Ring corner radius: the item's (or its background's) radius grown by how
    // far the band sits outside it, which is the offset curve of that shape
    function radiusFor(item: Item): real {
        const gap = ringOffsetFor(item);
        const info = root.targetInfo();

        if (info !== null && !isNaN(info.radius)) {
            return info.radius + gap;
        }

        if (item === null) {
            return gap;
        }

        if (item.hasOwnProperty("radius") && item.radius !== undefined) {
            return item.radius + gap;
        }

        if (item.hasOwnProperty("background") && item.background && item.background.hasOwnProperty("radius")) {
            return item.background.radius + gap;
        }

        return gap;
    }

    // Desired ring geometry in overlay space. Mapping the full rect (not just
    // the origin) keeps the ring correct under ancestor scale — focus pop etc.
    function desiredRect() {
        const c = cursorItem;
        const s = ringOffsetFor(c) + ringPad;
        return c.mapToItem(root, -s, -s, c.width + 2 * s, c.height + 2 * s);
    }

    // TODO
    // Rounds by both edges rather than by position and size separately, which
    // lets one side land a pixel off the other and shows as a gap down one edge
    function snapRect(r: rect): rect {
        const left = Math.round(r.x);
        const top = Math.round(r.y);

        return Qt.rect(left, top, Math.round(r.x + r.width) - left, Math.round(r.y + r.height) - top);
    }

    // TODO
    // Settled geometry lands on whole pixels so the band isn't split across
    // pixels and softened; motion stays fractional for smoothness
    function snapNow() {
        const r = root.snapRect(desiredRect());
        activeRing.x = r.x;
        activeRing.y = r.y;
        activeRing.width = r.width;
        activeRing.height = r.height;
        activeRing.radiusPx = radiusFor(cursorItem);
        gliding = false;
    }

    // True only during a focus transition; scroll-driven updates track 1:1
    property bool gliding: false

    // Exponential follow rate tuned so a glide settles in ~durationFast
    readonly property real glideRate: 6000 / AppStyle.durationBase

    // TODO
    // The ring being moved and tracked; in "jump" mode the two alternate so the
    // outgoing one can fade at the old item while this one appears on the new
    property FLFocusRing activeRing: ringA

    // TODO
    // Whether the cursor was on anything before this change
    property bool _hadCursor: false

    onCursorItemChanged: {
        // TODO
        // Whether the cursor came from somewhere rather than out of nothing —
        // the window regaining focus, or a page arriving, is not a move
        const arriving = !root._hadCursor;
        root._hadCursor = cursorItem !== null;

        if (cursorItem === null) {
            ringA.opacity = 0;
            ringB.opacity = 0;
            return;
        }

        const prevFlick = _flick;
        const interval = _lastChangeAt >= 0 ? _clock - _lastChangeAt : -1;
        _lastChangeAt = _clock;

        // TODO
        // The arrival sound the item asked for, quietened when the key that
        // moved focus is a hold repeating rather than a fresh press
        const arrivalInfo = root.targetInfo();

        if (root.visible && !root._blinking && !arriving && arrivalInfo !== null && arrivalInfo.focusSound) {
            arrivalInfo.focusSound.play(InputMethodManager.keyRepeating);
        }

        _flick = findFlickableAncestor(cursorItem);
        if (_flick === null) {
            _scrollPending = false;
            _cruising = false;
        } else {
            const scrollTarget = computeScrollTarget(_flick === prevFlick);
            const inFlight = (_scrollPending || _cruising) && _flick === prevFlick;
            const sameDestination = inFlight && Math.abs(scrollTarget - _scrollTo) < 0.5;

            if (!sameDestination) {
                if (inFlight && interval > 0 && interval < root.cruiseInterval) {
                    // TODO
                    // Held repeats: cruise closes the whole remaining gap just
                    // slower than the repeat cadence, so a coverage deficit
                    // repays itself and the cruise never runs dry mid-stream
                    _scrollTo = scrollTarget;
                    _cruiseVel = Math.abs(_scrollTo - _flick.contentY) / (interval * 1.25);
                    _cruiseInterval = interval;
                    _cruising = true;
                    _scrollPending = false;
                } else {
                    _scrollFrom = _flick.contentY;
                    _scrollTo = scrollTarget;
                    _scrollT = 0;
                    _cruising = false;
                    _scrollPending = Math.abs(scrollTarget - _flick.contentY) > 0.5;
                }
            }
        }

        // TODO
        // Focus may move while blinked (a popup closing hands it back); the
        // scroll bookkeeping above still applies, but the ring stays hidden
        // until endBlink() shows it in its new place
        if (_blinking) {
            return;
        }

        if (ringA.opacity === 0 && ringB.opacity === 0) {
            // Appearing: no flight in from a stale position
            applyScrollNow();
            snapNow();
            activeRing.opacity = 1;
            return;
        }

        if (moveMode !== "glide") {
            // TODO
            // Jump: crossfade onto the new item at its current spot; the pending
            // scroll then animates while the ring rides the item 1:1
            const outgoing = activeRing;
            activeRing = activeRing === ringA ? ringB : ringA;
            snapNow();
            outgoing.opacity = 0;
            activeRing.opacity = 1;
            return;
        }

        // TODO
        // Short layout hops glide; long (cross-container) jumps snap. The hop is
        // measured against the item's post-scroll resting place
        let r = desiredRect();
        if (_scrollPending) {
            r.y -= _scrollTo - _flick.contentY;
        }
        const dx = (r.x + r.width / 2) - (activeRing.x + activeRing.width / 2);
        const dy = (r.y + r.height / 2) - (activeRing.y + activeRing.height / 2);
        const hop = Math.sqrt(dx * dx + dy * dy);
        gliding = hop <= 2.5 * Math.max(r.width, r.height);

        if (!gliding) {
            applyScrollNow();
            snapNow();
        }
    }

    FrameAnimation {
        running: root.cursorItem !== null
        onTriggered: {
            root._clock += frameTime;

            // TODO
            // Underdamped bump spring, constants fitted to the measured Switch 2
            // bump (peak ~17ms, 12% undershoot, settled ~110ms); dt is clamped so
            // a slow frame can't destabilise the integration
            if (root._bumpPos !== 0 || root._bumpVel !== 0) {
                const sdt = Math.min(frameTime, 0.02);
                root._bumpVel += (-5700 * root._bumpPos - 84 * root._bumpVel) * sdt;
                root._bumpPos += root._bumpVel * sdt;

                if (Math.abs(root._bumpPos) < 0.1 && Math.abs(root._bumpVel) < 4) {
                    root._bumpPos = 0;
                    root._bumpVel = 0;
                }
            }
            const k = Math.min(1, frameTime * root.glideRate);
            const T = AppStyle.durationBase / 1000;

            // TODO
            // Scroll motion, Switch-style: a cruise covers held repeats at
            // constant velocity, then hands off to the fixed-duration quadratic
            // ease-out tween at matched velocity; single steps tween directly
            if (root._cruising) {
                const d = root._scrollTo - root._flick.contentY;
                // TODO
                // Quiet = the repeat stream has stopped; only then may the
                // cruise decelerate, otherwise every mid-stream slowdown bleeds
                // coverage and the item drifts off-screen
                const quiet = root._clock - root._lastChangeAt > root._cruiseInterval * 1.2;

                if (Math.abs(d) <= 0.5) {
                    root._flick.contentY = root._scrollTo;
                    if (quiet) {
                        root._cruising = false;
                    }
                } else if (quiet && Math.abs(d) <= root._cruiseVel * T / 2) {
                    root._scrollFrom = root._flick.contentY;
                    root._scrollT = 0;
                    root._cruising = false;
                    root._scrollPending = true;
                } else {
                    root._flick.contentY += (d > 0 ? 1 : -1) * Math.min(Math.abs(d), root._cruiseVel * frameTime);
                }
            }

            if (root._scrollPending) {
                root._scrollT += frameTime;

                if (root._scrollT >= T) {
                    root.applyScrollNow();
                } else {
                    const u = root._scrollT / T;
                    root._flick.contentY = root._scrollFrom + (root._scrollTo - root._scrollFrom) * (1 - (1 - u) * (1 - u));
                }
            }

            if (!root.gliding) {
                const live = root.desiredRect();
                const still = !root._scrollPending && !root._cruising && root._bumpPos === 0;
                const r0 = still ? root.snapRect(live) : live;
                const bwx = Math.abs(root._bumpDirX) * root._bumpPos * root.bumpStretch;
                const bwy = Math.abs(root._bumpDirY) * root._bumpPos * root.bumpStretch;
                root.activeRing.x = r0.x + root._bumpPos * root._bumpDirX - (root._bumpDirX < 0 ? bwx : 0);
                root.activeRing.y = r0.y + root._bumpPos * root._bumpDirY - (root._bumpDirY < 0 ? bwy : 0);
                root.activeRing.width = r0.width + bwx;
                root.activeRing.height = r0.height + bwy;
                root.activeRing.radiusPx = root.radiusFor(root.cursorItem);
                return;
            }

            let r = root.desiredRect();
            if (root._scrollPending || root._cruising) {
                r.y -= root._scrollTo - root._flick.contentY;
            }
            const gbwx = Math.abs(root._bumpDirX) * root._bumpPos * root.bumpStretch;
            const gbwy = Math.abs(root._bumpDirY) * root._bumpPos * root.bumpStretch;
            r.x += root._bumpPos * root._bumpDirX - (root._bumpDirX < 0 ? gbwx : 0);
            r.y += root._bumpPos * root._bumpDirY - (root._bumpDirY < 0 ? gbwy : 0);
            r.width += gbwx;
            r.height += gbwy;
            const rad = root.radiusFor(root.cursorItem);

            root.activeRing.x += (r.x - root.activeRing.x) * k;
            root.activeRing.y += (r.y - root.activeRing.y) * k;
            root.activeRing.width += (r.width - root.activeRing.width) * k;
            root.activeRing.height += (r.height - root.activeRing.height) * k;
            root.activeRing.radiusPx += (rad - root.activeRing.radiusPx) * k;

            const remaining = Math.max(Math.abs(r.x - root.activeRing.x), Math.abs(r.y - root.activeRing.y), Math.abs(r.width - root.activeRing.width), Math.abs(r.height - root.activeRing.height), root._scrollPending || root._cruising ? Math.abs(root._scrollTo - root._flick.contentY) : 0);

            if (remaining < 0.5) {
                root.applyScrollNow();
                root.gliding = false;
            }
        }
    }

    property real spinPhase: 0
    NumberAnimation on spinPhase {
        from: 0
        to: 1
        loops: Animation.Infinite
        duration: AppStyle.durationSpin
        running: root.cursorItem !== null
    }

    FLFocusRing {
        id: ringA

        colorA: Theme.focusRingBlue
        colorB: Theme.focusRingPurple
        ringWidthPx: AppStyle.focusRingWidth
        padPx: root.ringPad
        fillPx: root.ringOffsetFor(root.cursorItem)
        fillColor: root.fillFor(root.cursorItem)
        phase: root.spinPhase

        opacity: 0
        visible: opacity > 0

        Behavior on opacity {
            NumberAnimation {
                duration: AppStyle.durationSnap
            }
        }
    }

    FLFocusRing {
        id: ringB

        colorA: Theme.focusRingBlue
        colorB: Theme.focusRingPurple
        ringWidthPx: AppStyle.focusRingWidth
        padPx: root.ringPad
        fillPx: root.ringOffsetFor(root.cursorItem)
        fillColor: root.fillFor(root.cursorItem)
        phase: root.spinPhase

        opacity: 0
        visible: opacity > 0

        Behavior on opacity {
            NumberAnimation {
                duration: AppStyle.durationSnap
            }
        }
    }
}
