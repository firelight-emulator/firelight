import QtQuick

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

    // Anchor-margin convention: negative inflates the ring outward
    property real defaultAnchorMargins: 0

    // TODO
    // Space kept between the focused item and its flickable's edges when the
    // cursor drives scrolling
    property real scrollMarginTop: 100
    property real scrollMarginBottom: 100

    // Extra room around the ring so its band and antialiasing never clip
    readonly property real ringPad: AppStyle.focusRingWidth + 6

    z: 1000

    // The item the ring should surround: the focus target (or its opted-in
    // proxy), or null when nothing focusable is focused or the mouse is in use
    readonly property Item cursorItem: {
        if (!target || usingMouse) {
            return null;
        }

        if (!(target.hasOwnProperty("showGlobalCursor") && target.showGlobalCursor)) {
            return null;
        }

        if (target.hasOwnProperty("globalCursorProxy") && target.globalCursorProxy) {
            return target.globalCursorProxy;
        }

        return target;
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

    function computeScrollTarget(): real {
        const itemTop = cursorItem.mapToItem(_flick.contentItem, 0, 0).y;
        let desired = _flick.contentY;

        if (itemTop - scrollMarginTop < desired) {
            desired = itemTop - scrollMarginTop;
        } else if (itemTop + cursorItem.height + scrollMarginBottom > desired + _flick.height) {
            desired = itemTop + cursorItem.height + scrollMarginBottom - _flick.height;
        }

        const lo = _flick.originY;
        const hi = Math.max(lo, lo + _flick.contentHeight - _flick.height);
        return Math.max(lo, Math.min(hi, desired));
    }

    function applyScrollNow() {
        if (_scrollPending) {
            _flick.contentY = _scrollTo;
            _scrollPending = false;
        }
    }

    // Outward gap between the item and the ring, overridable per item
    function spacingFor(item: Item): real {
        if (item !== null && item.hasOwnProperty("globalCursorSpacing")) {
            return item.globalCursorSpacing;
        }

        return -root.defaultAnchorMargins;
    }

    // Ring corner radius: the item's (or its background's) radius plus the gap
    function radiusFor(item: Item): real {
        const gap = Math.abs(spacingFor(item));

        if (item === null) {
            return gap;
        }

        if (item.hasOwnProperty("radius") && item.radius !== undefined) {
            return item.radius + gap;
        }

        if (item.hasOwnProperty("background") && item.background
                && item.background.hasOwnProperty("radius")) {
            return item.background.radius + gap;
        }

        return gap;
    }

    // Desired ring geometry in overlay space. Mapping the full rect (not just
    // the origin) keeps the ring correct under ancestor scale — focus pop etc.
    function desiredRect() {
        const c = cursorItem;
        const s = spacingFor(c) + ringPad;
        return c.mapToItem(root, -s, -s, c.width + 2 * s, c.height + 2 * s);
    }

    function snapNow() {
        const r = desiredRect();
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
    readonly property real glideRate: 4000 / AppStyle.durationFast

    // TODO
    // The ring being moved and tracked; in "jump" mode the two alternate so the
    // outgoing one can fade at the old item while this one appears on the new
    property FLFocusRing activeRing: ringA

    onCursorItemChanged: {
        if (cursorItem === null) {
            ringA.opacity = 0;
            ringB.opacity = 0;
            return;
        }

        _flick = findFlickableAncestor(cursorItem);
        _scrollPending = _flick !== null;
        if (_scrollPending) {
            _scrollTo = computeScrollTarget();
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
            const k = Math.min(1, frameTime * root.glideRate);

            // TODO
            // The scroll animates here in both modes; in "glide" it runs at the
            // ring's rate so a scrolled step leaves the ring holding still while
            // the content slides under it
            if (root._scrollPending) {
                if (Math.abs(root._scrollTo - root._flick.contentY) > 0.5) {
                    root._flick.contentY += (root._scrollTo - root._flick.contentY) * k;
                } else {
                    root.applyScrollNow();
                }
            }

            if (!root.gliding) {
                const r0 = root.desiredRect();
                root.activeRing.x = r0.x;
                root.activeRing.y = r0.y;
                root.activeRing.width = r0.width;
                root.activeRing.height = r0.height;
                root.activeRing.radiusPx = root.radiusFor(root.cursorItem);
                return;
            }

            let r = root.desiredRect();
            if (root._scrollPending) {
                r.y -= root._scrollTo - root._flick.contentY;
            }
            const rad = root.radiusFor(root.cursorItem);

            root.activeRing.x += (r.x - root.activeRing.x) * k;
            root.activeRing.y += (r.y - root.activeRing.y) * k;
            root.activeRing.width += (r.width - root.activeRing.width) * k;
            root.activeRing.height += (r.height - root.activeRing.height) * k;
            root.activeRing.radiusPx += (rad - root.activeRing.radiusPx) * k;

            const remaining = Math.max(
                Math.abs(r.x - root.activeRing.x), Math.abs(r.y - root.activeRing.y),
                Math.abs(r.width - root.activeRing.width), Math.abs(r.height - root.activeRing.height),
                root._scrollPending ? Math.abs(root._scrollTo - root._flick.contentY) : 0);

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
        phase: root.spinPhase

        opacity: 0
        visible: opacity > 0

        Behavior on opacity {
            NumberAnimation {
                duration: AppStyle.durationFast
            }
        }
    }

    FLFocusRing {
        id: ringB

        colorA: Theme.focusRingBlue
        colorB: Theme.focusRingPurple
        ringWidthPx: AppStyle.focusRingWidth
        padPx: root.ringPad
        phase: root.spinPhase

        opacity: 0
        visible: opacity > 0

        Behavior on opacity {
            NumberAnimation {
                duration: AppStyle.durationFast
            }
        }
    }
}
