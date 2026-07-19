import QtQuick

Item {
    id: root

    property color borderColor: "#ff9f38"
    property real borderWidth: 2
    property real bounceAmplitude: 16
    property real defaultAnchorMargins: -4
    property int animationDuration: 50
    property real scrollMarginTop: 100
    property real scrollMarginBottom: 100
    z: 1000

    required property Item target
    required property bool usingMouse

    visible: !usingMouse

    // Track which highlight is currently "active"
    property bool useHighlightA: true

    function calculateRadius(item: Item): real {
        if (item === null) {
            return Math.abs(root.defaultAnchorMargins);
        }

        if (item.hasOwnProperty('background') && item.background && item.background.hasOwnProperty('radius')) {
            return item.background.radius + Math.abs(root.defaultAnchorMargins);
        } else if (item.hasOwnProperty('radius')) {
            return item.radius + Math.abs(root.defaultAnchorMargins);
        } else {
            return Math.abs(root.defaultAnchorMargins);
        }
    }

    function getMargins(item: Item): real {
        if (item !== null && item.hasOwnProperty('globalCursorSpacing')) {
            return -item.globalCursorSpacing;
        }
        return root.defaultAnchorMargins;
    }

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

    // Two highlights that crossfade via re-parenting
    Rectangle {
        id: highlightA
        opacity: 0
        visible: !root.usingMouse
        anchors.fill: parent
        anchors.margins: parent ? root.getMargins(parent) : root.defaultAnchorMargins
        border.color: root.borderColor
        border.width: root.borderWidth
        color: "transparent"
        radius: root.calculateRadius(parent)
        z: 1000

        Behavior on opacity {
            NumberAnimation {
                duration: root.animationDuration
                easing.type: Easing.OutQuad
            }
        }
    }

    Rectangle {
        id: highlightB
        opacity: 0
        visible: !root.usingMouse
        anchors.fill: parent
        anchors.margins: parent ? root.getMargins(parent) : root.defaultAnchorMargins
        border.color: root.borderColor
        border.width: root.borderWidth
        color: "transparent"
        radius: root.calculateRadius(parent)
        z: 1000

        Behavior on opacity {
            NumberAnimation {
                duration: root.animationDuration
                easing.type: Easing.OutQuad
            }
        }
    }

    onTargetChanged: {
        if (target === null) {
            highlightA.opacity = 0;
            highlightB.opacity = 0;
            return;
        }

        if (!root.usingMouse) {
            var flickable = findFlickableAncestor(target);
            if (flickable !== null) {
                var pos = target.mapToItem(flickable.contentItem, 0, 0);
                var targetTop = pos.y;
                var targetBottom = targetTop + target.height;
                if (targetTop - root.scrollMarginTop < flickable.contentY) {
                    flickable.contentY = Math.max(0, targetTop - root.scrollMarginTop);
                } else if (targetBottom + root.scrollMarginBottom > flickable.contentY + flickable.height) {
                    flickable.contentY = Math.min(targetBottom + root.scrollMarginBottom - flickable.height, flickable.contentHeight - flickable.height);
                }
            }
        }

        if (target.hasOwnProperty('showGlobalCursor') && target.showGlobalCursor) {
            var newParent;
            if (target.hasOwnProperty("globalCursorProxy") && target.globalCursorProxy) {
                newParent = target.globalCursorProxy;
            } else {
                newParent = target;
            }

            // Crossfade: fade out old highlight (stays with old parent), fade in new one
            if (useHighlightA) {
                highlightA.parent = newParent;
                highlightA.opacity = 1;
                highlightB.opacity = 0;
            } else {
                highlightB.parent = newParent;
                highlightB.opacity = 1;
                highlightA.opacity = 0;
            }

            useHighlightA = !useHighlightA;
            return;
        }

        // Target doesn't have showGlobalCursor
        highlightA.opacity = 0;
        highlightB.opacity = 0;
    }
}
