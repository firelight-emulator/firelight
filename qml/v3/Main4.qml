// TODO: NEEDS REVIEW
import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import QtQuick.Layouts 1.0
import QtQuick.Window
import Firelight 1.0

MainWindow {
    id: window

    TapHandler {
        onTapped: window.contentItem.forceActiveFocus(Qt.MouseFocusReason)
    }

    background: FLUserBackground {
        // mode: AppearanceSettings.backgroundMode
        // color1: AppearanceSettings.backgroundColor
        // color2: AppearanceSettings.backgroundColor2
        // backgroundFile: AppearanceSettings.backgroundFile
        // blurAmount: 0
        // dimAmount: 0
        defaultColor: "#1A1A1C"
    }

    // Hosts overlay routes (e.g. /settings) as a popup over the current view
    // RouteOverlay {
    //     id: routeOverlay
    // }

    // A hotkey has no other feedback: without this, a save state and a binding
    // that silently doesn't work look identical. The wording is decided in C++
    // Toast {
    //     id: shortcutToast
    //     z: 999
    //
    //     Connections {
    //         target: ShortcutDispatcher
    //         function onNotified(message) {
    //             shortcutToast.show(message);
    //         }
    //     }
    // }

    Action {
        id: searchAction
        text: qsTr("&Copy")
        icon.name: "edit-copy"
        shortcut: StandardKey.Find
        onTriggered: actualTitleBar.activateSearch()
    }

    // TODO
    // Dev-only: toggle the component gallery
    Shortcut {
        sequence: "F11"
        context: Qt.ApplicationShortcut
        onActivated: Router.matchedPattern === "/dev/gallery" ? Router.back() : Router.navigate("/dev/gallery")
    }

    // TODO
    // Dev-only: the disc set manager, in a window of its own until it has a home in the UI
    Shortcut {
        sequence: "F10"
        context: Qt.ApplicationShortcut
        onActivated: {
            if (discSetWindow.visible) {
                discSetWindow.close();
            } else {
                DiscSetModel.refresh();
                discSetWindow.show();
            }
        }
    }
    Shortcut {
        sequence: "F9"
        context: Qt.ApplicationShortcut
        onActivated: navigationPopup.visible ? navigationPopup.close() : navigationPopup.open()
    }

    Shortcut {
        sequence: "F8"
        context: Qt.ApplicationShortcut
        onActivated: {
            FLKeyboard.open({
                "text": "",
                "placeholderText": "Enter whatever man I don't care",
                "maxLength": 128,
                "sensitive": false,
                "acceptLabel": "Submit"
            }, function (result) {
                console.log("Got result from keyboard: " + result);
            });
        }
    }

    Window {
        id: discSetWindow

        objectName: "DiscSetManagerWindow"
        width: 1000
        height: 640
        title: qsTr("Disc sets")
        color: Theme.surface

        DiscSetManager {
            anchors.fill: parent
        }
    }

    // TODO
    // Toggles the performance overlay, on the key RetroArch uses for the same thing so the two can
    // be brought up side by side without relearning it
    Shortcut {
        sequence: "F3"
        context: Qt.ApplicationShortcut
        onActivated: {
            PerformanceStats.toggle();
            window.monitorOverlayShown = !window.monitorOverlayShown;
        }
    }

    PerformanceOverlay {
        id: performanceOverlay

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: AppStyle.spacingLg
        z: 9999
    }

    // TODO
    // Whether the new monitor is up as an overlay. Recording runs while any of its three mounts is
    // showing, which the Binding below is the one place that decides
    property bool monitorOverlayShown: false

    Binding {
        target: PerformanceMonitor
        property: "visible"
        value: window.monitorOverlayShown || performanceWindow.visible || Router.matchedPattern === "/dev/monitor"
    }

    Rectangle {
        id: performanceMonitorOverlay

        visible: window.monitorOverlayShown
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: AppStyle.spacingLg
        width: monitorPanel.implicitWidth
        height: Math.min(monitorPanel.implicitHeight, parent.height - AppStyle.spacingLg * 2)
        z: 9999
        color: Qt.rgba(Theme.surface.r, Theme.surface.g, Theme.surface.b, 0.82)
        border.color: Theme.border
        border.width: 1
        radius: AppStyle.spacingXs
        clip: true

        PerformanceMonitorPanel {
            id: monitorPanel

            width: parent.width
        }
    }

    // TODO
    // The same monitor in a window of its own, for a second screen
    Shortcut {
        sequence: "Shift+F3"
        context: Qt.ApplicationShortcut
        onActivated: performanceWindow.visible ? performanceWindow.close() : performanceWindow.show()
    }

    Shortcut {
        sequence: "Ctrl+F3"
        context: Qt.ApplicationShortcut
        onActivated: PerformanceMonitor.togglePaused()
    }

    Window {
        id: performanceWindow

        objectName: "PerformanceMonitorWindow"
        width: 900
        height: 820
        title: qsTr("Performance monitor")
        color: Theme.surface

        PerformanceMonitorPage {
            anchors.fill: parent
        }
    }

    // Pane {
    //     id: navRail
    //     anchors.top: titleBar.bottom
    //     anchors.left: parent.left
    //     anchors.bottom: parent.bottom
    //     width: Math.round(48 * AppStyle.scale)
    //
    //     background: Item {}
    //
    //     ColumnLayout {
    //         anchors.horizontalCenter: parent.horizontalCenter
    //         anchors.top: parent.top
    //         spacing: AppStyle.spacingLg
    //
    //         Repeater {
    //             model: [
    //                 {
    //                     displayName: "Library",
    //                     iconName: "browse",
    //                     route: "/library"
    //                 },
    //                 {
    //                     displayName: "Mod Shop",
    //                     iconName: "shopping-bag",
    //                     route: "/shop"
    //                 },
    //                 {
    //                     displayName: "Controllers",
    //                     iconName: "controller",
    //                     route: "/controllers"
    //                 },
    //                 {
    //                     displayName: "Gallery",
    //                     iconName: "photo-library",
    //                     route: "/gallery"
    //                 },
    //                 {
    //                     displayName: "Activity",
    //                     iconName: "bar-chart",
    //                     route: "/activity"
    //                 },
    //                 {
    //                     displayName: "Online Lobby",
    //                     iconName: "online",
    //                     route: "/netplay"
    //                 }
    //             ]
    //
    //             delegate: FLIconButton {
    //                 id: menuItem
    //                 required property var model
    //
    //                 checkable: false
    //                 checked: Router.isActive(model.route)
    //                 checkedColor: Theme.textPrimary
    //                 iconName: model.iconName
    //                 tooltipText: model.displayName
    //
    //                 Layout.alignment: Qt.AlignVCenter
    //
    //                 onClicked: {
    //                     Router.navigate(model.route);
    //                 }
    //             }
    //         }
    //
    //         Item {
    //             Layout.fillHeight: true
    //             Layout.fillWidth: true
    //         }
    //
    //         FLIconButton {
    //             readonly property string route: "/settings"
    //
    //             checkable: false
    //             checked: Router.isActive(route)
    //             checkedColor: Theme.textPrimary
    //             iconName: "settings"
    //             tooltipText: "Settings"
    //
    //             Layout.alignment: Qt.AlignVCenter
    //
    //             onClicked: {
    //                 Router.navigate(route);
    //             }
    //         }
    //     }
    // }

    GameplayPage {
        id: gameplayPage
        anchors.fill: parent
        // visible: Router.isActive("/quick-menu")
    }

    Item {
        id: contentContainer
        anchors.fill: parent

        FLFocus.actions: [
            FLAction {
                keys: [Qt.Key_Back, Qt.Key_Escape]
                label: qsTr("Back")
                enabled: Router.canGoBack
                hidden: !Router.canGoBack
                sound: SoundEffects.back
                onTriggered: event => {
                    // TODO
                    // A screen may refuse to be left, and a refused press makes no sound
                    if (!Router.back()) {
                        event.accepted = false;
                    }

                    guideBar.refresh();
                }
            }
        ]

        // TODO
        // Arrow presses no container used end up here — the cursor's edge bump.
        // Fires once per continuous hold: the first dead press bumps, repeats
        // stay quiet, and the release (or focus moving) re-arms it
        property bool deadHeld: false

        Keys.onPressed: event => {
            if (contentContainer.FLFocus.dispatch(window.activeFocusItem, event.key, event.modifiers, event.isAutoRepeat)) {
                // TODO
                // A held key changes nothing the bar lists, and answering is not running, so only a
                // fresh press is worth recollecting for
                if (!event.isAutoRepeat) {
                    guideBar.refresh();
                }

                event.accepted = true;
                return;
            }

            if (FocusNavigator.move(window.activeFocusItem, event.key, event.isAutoRepeat) !== FocusNavigator.NoTarget) {
                console.log("Input method detection handler key repeat: " + InputMethodManager.keyRepeating);
                event.accepted = true;
                return;
            }

            if (contentContainer.deadHeld) {
                return;
            }

            contentContainer.deadHeld = FocusCursor.bump(event.key);
        }

        Keys.onReleased: event => {
            if (event.isAutoRepeat) {
                return;
            }

            if (event.key === Qt.Key_Up || event.key === Qt.Key_Down || event.key === Qt.Key_Left || event.key === Qt.Key_Right) {
                contentContainer.deadHeld = false;
            }
        }

        Connections {
            target: focusHighlight
            function onCursorItemChanged() {
                contentContainer.deadHeld = false;
            }
        }

        RouteView {
            id: contentStack
            clip: true

            // layer.enabled: FLDimmer.isVisible
            // layer.effect: MultiEffect {
            //     source: contentStack
            //     anchors.fill: contentStack
            //     blurEnabled: true
            //     blurMultiplier: 0
            //     blurMax: 64
            //     blur: 1.0
            //     autoPaddingEnabled: false
            // }

            objectName: "RouteView"

            anchors.bottomMargin: guideBar.height
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: titleBar.bottom

            Component.onCompleted: Router.navigate(StartupOptions.startupRoute !== "" ? StartupOptions.startupRoute : "/library")
        }

        // Shown while an uncached page is being built asynchronously
        BusyIndicator {
            anchors.centerIn: contentStack
            implicitWidth: 48
            implicitHeight: 48
            running: contentStack.loading
            visible: running
            z: 10
        }

        FLPanel {
            id: navigationPopup
            z: guideBar.z + 1

            height: parent.height
            width: 360

            background: Rectangle {
                color: Theme.surface
            }

            enter: Transition {
                ParallelAnimation {
                    NumberAnimation {
                        property: "x"
                        from: -navigationPopup.width * 1.2
                        to: 0
                        duration: 200
                        easing.type: Easing.InOutQuad
                    }
                    NumberAnimation {
                        target: contentStack
                        property: "scale"
                        from: 1
                        to: 0.975
                        duration: 200
                        easing.type: Easing.InOutQuad
                    }
                }
            }

            exit: Transition {
                ParallelAnimation {
                    NumberAnimation {
                        property: "x"
                        from: 0
                        to: -navigationPopup.width * 1.2
                        duration: 200
                        easing.type: Easing.InOutQuad
                    }

                    NumberAnimation {
                        target: contentStack
                        property: "scale"
                        to: 1
                        duration: 200
                        easing.type: Easing.InOutQuad
                    }
                }
            }

            // TODO
            // A scope rather than the column itself, so focus arriving here is handed on to the row that
            // declares it rather than stopping on something that only lays out
            contentItem: FocusScope {
                // TODO
                // The popup item above this is a focus scope of its own and keeps what it is given, so the
                // surface has to claim it for anything inside to be reached
                focus: true
                FLColumnLayout {
                    anchors.fill: parent

                    FLButton {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 240
                        Layout.margins: AppStyle.spacingSm
                        checked: Router.isActive("/quick-menu")
                        focus: checked

                        background: Rectangle {
                            radius: AppStyle.radiusMd
                            color: Theme.surfaceElevated
                        }

                        Text {
                            anchors.centerIn: parent
                            text: "You're not currently playing anything"
                            color: Theme.textPrimary
                            font.family: AppStyle.fontFamily
                            font.pixelSize: AppStyle.fontSizeSmall
                            horizontalAlignment: Text.AlignHCenter
                        }

                        onClicked: {
                            if (!checked) {
                                Qt.callLater(() => Router.navigate("/quick-menu"));
                                navigationPopup.close();
                            }
                        }
                    }

                    FLDivider {
                        Layout.topMargin: AppStyle.spacingSm
                        Layout.bottomMargin: AppStyle.spacingSm
                        Layout.fillWidth: true
                    }

                    Repeater {
                        model: [
                            {
                                "label": "Library",
                                "route": "/library"
                            },
                            {
                                "label": "Settings",
                                "route": "/settings"
                            }
                        ]

                        delegate: MenuNavigationItem {
                            required property var modelData
                            label: modelData.label
                            checked: Router.isActive(modelData.route)
                            focus: checked

                            onClicked: {
                                if (!checked) {
                                    Qt.callLater(() => Router.navigate(modelData.route));
                                    navigationPopup.close();
                                }
                            }
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }

                    MenuNavigationItem {
                        label: "Power"
                        checkable: false
                    }
                }
            }
        }

        FLGuideBar {
            id: guideBar
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: AppStyle.windowPadding
            anchors.rightMargin: AppStyle.windowPadding
            z: focusHighlight.z - 2
            showDivider: !onScreenKeyboard.visible

            parent: Overlay.overlay
        }

        Pane {
            id: titleBar
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: AppStyle.titleBarHeight
            padding: AppStyle.titleBarPadding
            z: onScreenKeyboard + 1

            background: Item {
                Rectangle {
                    color: Theme.border
                    height: 1
                    anchors.left: parent.left
                    anchors.leftMargin: AppStyle.windowPadding
                    anchors.right: parent.right
                    anchors.rightMargin: AppStyle.windowPadding
                    anchors.bottom: parent.bottom
                }
            }
            TitleBar {
                id: actualTitleBar
                anchors.fill: parent

                page: contentStack.currentItem

                onMaximizeClicked: window.maximize()
                onMinimizeClicked: window.showMinimized()
                onCloseClicked: window.close()
            }
        }

        // GameplayLayer {
        //     id: gameplay
        //     z: 90
        // }

        // TODO
        // Lives outside the content it samples: a blur source drawn inside its own source item
        // renders into the texture it is reading from
        Item {
            id: dimmer

            parent: Overlay.overlay
            anchors.fill: parent

            // TODO
            // Under every popup, which take the overlay's default
            z: -1

            Component.onCompleted: {
                FLDimmer.target = dimmer;
                FLDimmer.blurTarget = contentContainer;
            }
        }

        FLFocusHighlight {
            id: focusHighlight
            parent: Overlay.overlay
            z: 1000000
            target: window.activeFocusItem
            usingMouse: InputMethodManager.usingMouse

            Component.onCompleted: {
                FocusCursor.register(focusHighlight);
                FocusNavigator.watch(focusHighlight);
            }
        }
    }

    LaunchCinematic {
        id: launchCinematic
        parent: Overlay.overlay
        z: 200000
    }

    // TODO
    // Mounted once and raised through the FLKeyboard singleton, which has no scene graph of its own
    FLKeyboardOverlay {
        id: onScreenKeyboard
        z: guideBar.z - 1
        topPadding: 36
        bottomReservedHeight: guideBar.height

        Component.onCompleted: FLKeyboard.overlay = onScreenKeyboard
    }

    Connections {
        target: EmulationService

        function onGameLoadStarted() {
            launchCinematic.launchFrom(FocusCursor.highlight ? FocusCursor.highlight.cursorItem : null);
        }

        function onGameLoadFailed(message) {
            launchCinematic.reveal();
        }
    }

    Connections {
        target: launchCinematic
        function onBlackFull() {
            gameplayPage.startGame();
            launchCinematic.reveal();
        }
    }

    // Connections {
    //     target: gameplay
    //     function onReadyToReveal() {
    //         launchCinematic.reveal();
    //     }
    // }

    // Netplay status has to outlive the /netplay page: both of these sit above
    // the gameplay layer so they stay visible wherever the user has navigated
    // LobbyChip {
    //     anchors.left: parent.left
    //     anchors.bottom: parent.bottom
    //     anchors.margins: AppStyle.spacingLg
    //     z: 900
    // }
    //
    // ReadyCheckToast {
    //     anchors.horizontalCenter: parent.horizontalCenter
    //     anchors.bottom: parent.bottom
    //     anchors.bottomMargin: AppStyle.spacingXl
    //     z: 950
    //
    //     onHostLaunchRequested: {
    //         const entryId = NetworkService.selectedGameEntryId();
    //         if (entryId >= 0) {
    //             NetworkService.confirmLaunch();
    //             EmulationService.loadEntry(entryId);
    //         }
    //     }
    // }

    // A game named on the command line launches once the shell is up. When the
    // CLI also asked for a RetroAchievements login, that has to land first so
    // the session is credited to the right user
    function maybeAutoLaunch() {
        if (StartupOptions.launchEntryId >= 0) {
            Qt.callLater(function () {
                EmulationService.loadEntry(StartupOptions.launchEntryId);
            });
        }
    }

    // The launch still proceeds on a failed login, just without achievements
    function raLoginFailed(reason) {
        shortcutToast.show(qsTr("RetroAchievements login failed: ") + reason);
        window.maybeAutoLaunch();
    }

    function beginRaLogin() {
        if (StartupOptions.raToken.length > 0) {
            achievement_manager.logInUserWithToken(StartupOptions.raUsername, StartupOptions.raToken);
        } else {
            achievement_manager.logInUserWithPassword(StartupOptions.raUsername, StartupOptions.raPassword);
        }
    }

    Connections {
        target: achievement_manager
        enabled: StartupOptions.raPendingLogin

        function onLoginSucceeded() {
            window.maybeAutoLaunch();
        }

        function onLoginFailedWithInvalidCredentials() {
            window.raLoginFailed(qsTr("Invalid username or password"));
        }

        function onLoginFailedWithExpiredToken() {
            window.raLoginFailed(qsTr("Login token has expired"));
        }

        function onLoginFailedWithAccessDenied() {
            window.raLoginFailed(qsTr("Access denied"));
        }

        function onLoginFailedWithInternalError() {
            window.raLoginFailed(qsTr("RetroAchievements is unreachable"));
        }
    }

    // Null unless --single-instance was passed; a second process forwards its
    // launch here instead of opening another window
    Connections {
        target: SingleInstance
        enabled: SingleInstance !== null

        function onLaunchRequested(entryId) {
            if (entryId >= 0) {
                EmulationService.loadEntry(entryId);
            }
        }
    }

    Component.onCompleted: {
        if (StartupOptions.raPendingLogin) {
            window.beginRaLogin();
        } else {
            window.maybeAutoLaunch();
        }
    }

    // FLPROBE-BEGIN
    readonly property bool probeOn: Qt.application.arguments.join(" ").indexOf("fl-probe") >= 0
    property real probeT0: Date.now()
    property string probeLast: ""
    property int probeStep: 0
    property real probeStepAt: 0

    function probeLog(tag, msg) {
        console.warn("FLPROBE " + (Date.now() - window.probeT0) + " " + tag + " " + msg);
    }

    function probeIsAncestor(anc, it) {
        for (let p = it; p; p = p.parent) {
            if (p === anc) {
                return true;
            }
        }
        return false;
    }

    function probePageOf(it) {
        if (!it) {
            return "-";
        }
        const cache = contentStack._cache;
        for (let i = 0; i < cache.length; i++) {
            if (window.probeIsAncestor(cache[i].item, it)) {
                return cache[i].key;
            }
        }
        if (window.probeIsAncestor(navigationPopup.contentItem, it)) {
            return "drawer";
        }
        if (window.probeIsAncestor(contentStack, it)) {
            return "stack";
        }
        return "other";
    }

    function probeDesc(it) {
        if (!it) {
            return "null";
        }
        return String(it).split("(")[0] + "[" + it.objectName + "]@" + window.probePageOf(it);
    }

    function probeRings() {
        const out = [];
        for (let i = 0; i < focusHighlight.children.length; i++) {
            const c = focusHighlight.children[i];
            if (String(c).indexOf("FLFocusRing") === 0) {
                out.push(c.opacity.toFixed(2));
            }
        }
        return out.join("/");
    }

    function probeFind(node, name) {
        if (!node) {
            return null;
        }
        if (node.objectName === name) {
            return node;
        }
        for (let i = 0; i < node.children.length; i++) {
            const f = window.probeFind(node.children[i], name);
            if (f) {
                return f;
            }
        }
        return null;
    }

    function probeSettled(path) {
        return Router.path.indexOf(path) === 0 && !contentStack.loading && !contentStack.busy && !navigationPopup.visible;
    }

    readonly property var probeSteps: [
        {
            name: "ready",
            wait: 2500,
            when: () => window.probeSettled("/library"),
            act: () => {}
        },
        {
            name: "open drawer",
            wait: 300,
            when: () => true,
            act: () => navigationPopup.open()
        },
        {
            name: "focus Settings",
            wait: 600,
            when: () => navigationPopup.opened,
            act: () => window.probeFind(navigationPopup.contentItem, "MenuNavigationItem|Settings").forceActiveFocus()
        },
        {
            name: "click Settings (miss)",
            wait: 400,
            when: () => true,
            act: () => window.probeFind(navigationPopup.contentItem, "MenuNavigationItem|Settings").clicked()
        },
        {
            name: "open drawer",
            wait: 1500,
            when: () => window.probeSettled("/settings"),
            act: () => navigationPopup.open()
        },
        {
            name: "focus Library",
            wait: 600,
            when: () => navigationPopup.opened,
            act: () => window.probeFind(navigationPopup.contentItem, "MenuNavigationItem|Library").forceActiveFocus()
        },
        {
            name: "click Library (hit)",
            wait: 400,
            when: () => true,
            act: () => window.probeFind(navigationPopup.contentItem, "MenuNavigationItem|Library").clicked()
        },
        {
            name: "open drawer",
            wait: 1500,
            when: () => window.probeSettled("/library"),
            act: () => navigationPopup.open()
        },
        {
            name: "focus Settings",
            wait: 600,
            when: () => navigationPopup.opened,
            act: () => window.probeFind(navigationPopup.contentItem, "MenuNavigationItem|Settings").forceActiveFocus()
        },
        {
            name: "click Settings (hit)",
            wait: 400,
            when: () => true,
            act: () => window.probeFind(navigationPopup.contentItem, "MenuNavigationItem|Settings").clicked()
        },
        {
            name: "quit",
            wait: 1500,
            when: () => window.probeSettled("/settings"),
            act: () => Qt.quit()
        }
    ]

    Timer {
        running: window.probeOn && window.probeStep < window.probeSteps.length
        repeat: true
        interval: 10
        onTriggered: {
            const step = window.probeSteps[window.probeStep];
            if (Date.now() - window.probeStepAt < step.wait || !step.when()) {
                return;
            }
            window.probeLog("STEP", step.name);
            window.probeStep += 1;
            window.probeStepAt = Date.now();
            step.act();
        }
    }

    FrameAnimation {
        running: window.probeOn
        onTriggered: {
            const cursor = focusHighlight.cursorItem;
            const s = "path=" + Router.path + " popup=" + navigationPopup.visible + "/" + navigationPopup.opened + " loading=" + contentStack.loading + " busy=" + contentStack.busy + " blink=" + focusHighlight.blinking + " rings=" + window.probeRings() + " cursor=" + window.probeDesc(cursor) + " afi=" + window.probeDesc(window.activeFocusItem) + " mouse=" + InputMethodManager.usingMouse;
            if (s !== window.probeLast) {
                window.probeLast = s;
                window.probeLog("FRAME", s);
            }
        }
    }

    Connections {
        target: window.probeOn ? Router : null
        function onNavigated(transition) {
            window.probeLog("EVENT", "navigated " + Router.path + " transition=" + transition);
        }
    }

    Connections {
        target: window.probeOn ? navigationPopup : null
        function onAboutToShow() {
            window.probeLog("EVENT", "popup aboutToShow");
        }
        function onOpened() {
            window.probeLog("EVENT", "popup opened");
        }
        function onAboutToHide() {
            window.probeLog("EVENT", "popup aboutToHide afi=" + window.probeDesc(window.activeFocusItem));
        }
        function onClosed() {
            window.probeLog("EVENT", "popup closed afi=" + window.probeDesc(window.activeFocusItem));
        }
    }

    Connections {
        target: window.probeOn ? contentStack : null
        function onLoadingChanged() {
            window.probeLog("EVENT", "loading=" + contentStack.loading);
        }
        function onBusyChanged() {
            window.probeLog("EVENT", "busy=" + contentStack.busy);
        }
        function onCurrentItemChanged() {
            window.probeLog("EVENT", "currentItem=" + window.probeDesc(contentStack.currentItem));
        }
    }

    Connections {
        target: window.probeOn ? focusHighlight : null
        function onBlinkingChanged() {
            window.probeLog("EVENT", "blinking=" + focusHighlight.blinking);
        }
        function onCursorItemChanged() {
            window.probeLog("EVENT", "cursorItem=" + window.probeDesc(focusHighlight.cursorItem));
        }
    }

    Connections {
        target: window.probeOn ? window : null
        function onActiveFocusItemChanged() {
            window.probeLog("EVENT", "activeFocusItem=" + window.probeDesc(window.activeFocusItem));
        }
    }
    // FLPROBE-END

    component RoleData: QtObject {
        property string displayName
    }
}
