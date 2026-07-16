import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

FocusScope {
    id: root

    property bool gameRunning: false
    // Slide direction for the content transition (down = later item selected).
    property bool movingDown: true

    // Single source of truth for the nav taxonomy. Each item's `page` references
    // a Component id defined at the bottom of this file. Grouped equivalent of
    // FLTwoColumnMenu's parallel menuItems/routeNames/pages.
    property var sections: [
        {
            "title": "",
            "items": [
                { "displayName": "Appearance", "iconName": "palette", "route": "appearance", "page": appearanceSettings,
                  "keywords": ["theme", "color", "colour", "accent", "background", "wallpaper", "blur", "tint", "glass"] },
                { "displayName": "Display", "iconName": "display", "route": "display", "page": videoSettings,
                  "keywords": ["video", "resolution", "fullscreen", "aspect ratio", "vsync", "sync", "scaling", "frame rate"] },
                { "displayName": "Sound", "iconName": "speaker", "route": "sound", "page": soundSettings,
                  "keywords": ["audio", "volume", "output", "device", "mute", "latency"] },
                { "displayName": "Notifications", "iconName": "bell", "route": "notifications", "page": notificationSettings,
                  "keywords": ["alerts", "toasts", "popups"] },
                // { "displayName": "Game Folders", "iconName": "add", "route": "game-folders", "page": directorySettings },
                // { "displayName": "Scanning", "iconName": "add", "route": "scanning", "page": librarySettings },
                // { "displayName": "Metadata Scraping", "iconName": "add", "route": "metadata", "page": placeholderSettings },
                { "displayName": "Captures", "iconName": "photo-library", "route": "captures", "page": placeholderSettings,
                  "keywords": ["screenshot", "clip", "recording", "instant replay", "gallery", "video"] },
                { "displayName": "Achievements", "iconName": "trophy", "route": "retroachievements", "page": retroAchievementSettings,
                  "keywords": ["retroachievements", "hardcore", "account", "login", "trophies"] },
                { "displayName": "About", "iconName": "info", "route": "about", "page": about,
                  "keywords": ["version", "license", "credits", "update"] }
            ]
        }
        // {
        //     "title": "Library",
        //     "items": [
        //     ]
        // },
        // {
        //     "title": "Emulation",
        //     "items": [
        //     ]
        // },
        // {
        //     "title": "RetroAchievements",
        //     "items": [
        //     ]
        // },
        // {
        //     "title": "",
        //     "items": [
        //     ]
        // }
    ]

    // The currently selected item, driven by clicks and by the URL.
    property string currentRoute: ""
    property string currentTitle: ""
    property int currentFlatIndex: -1

    // Nav search. Matches a page's name or its keywords, so "vsync" finds
    // Display even though no nav item says that word.
    property string filterText: ""

    function matchesFilter(item) {
        const query = filterText.trim().toLowerCase();
        if (query === "") {
            return true;
        }
        if (item.displayName.toLowerCase().indexOf(query) !== -1) {
            return true;
        }
        const words = item.keywords;
        for (let i = 0; words && i < words.length; i++) {
            if (words[i].toLowerCase().indexOf(query) !== -1) {
                return true;
            }
        }
        return false;
    }

    // `sections` filtered down to matches; sections with nothing left drop out.
    readonly property var visibleSections: {
        if (filterText.trim() === "") {
            return sections;
        }
        const out = [];
        for (let s = 0; s < sections.length; s++) {
            const kept = [];
            for (let i = 0; i < sections[s].items.length; i++) {
                if (matchesFilter(sections[s].items[i])) {
                    kept.push(sections[s].items[i]);
                }
            }
            if (kept.length > 0) {
                out.push({ "title": sections[s].title, "items": kept });
            }
        }
        return out;
    }

    function selectFirstMatch() {
        const found = visibleSections;
        if (found.length > 0 && found[0].items.length > 0) {
            navigateTo(found[0].items[0].route);
        }
    }

    Component.onCompleted: {
        const route = routeFromPath();
        // initialItem already shows the first item; only deep-links need a swap.
        applyRoute(route, route !== sections[0].items[0].route);
        syncFromRoute();
    }

    // Resolve the route the URL is asking for, falling back to the first item.
    function routeFromPath() {
        const m = Router.match(Router.path, ["/settings/:section"]);
        if (m.matched && itemForRoute(m.params.section)) {
            return m.params.section;
        }
        return sections[0].items[0].route;
    }

    // Find an item + its flat index (across all sections) by route.
    function itemForRoute(route) {
        var flat = 0;
        for (var s = 0; s < sections.length; s++) {
            for (var i = 0; i < sections[s].items.length; i++) {
                if (sections[s].items[i].route === route) {
                    return { "item": sections[s].items[i], "flatIndex": flat };
                }
                flat++;
            }
        }
        return null;
    }

    // Select a route; swaps the content page (with transition) when animate.
    function applyRoute(route, animate) {
        const found = itemForRoute(route);
        if (!found) {
            return;
        }
        root.movingDown = found.flatIndex >= root.currentFlatIndex;
        root.currentFlatIndex = found.flatIndex;
        root.currentRoute = route;
        root.currentTitle = found.item.displayName;
        if (animate) {
            contentStack.replaceCurrentItem(found.item.page);
        }
    }

    // A user selection: swap the page and reflect it in the URL.
    function navigateTo(route) {
        if (route === root.currentRoute) {
            return;
        }
        applyRoute(route, true);
        if (Router.isActive("/settings")) {
            const target = "/settings/" + route;
            if (Router.path !== target) {
                Router.replace(target);
            }
        }
    }

    // Keep the selected page and the /settings/<section> URL in sync.
    function syncFromRoute() {
        if (!Router.isActive("/settings")) {
            return;
        }
        const m = Router.match(Router.path, ["/settings/:section"]);
        if (m.matched) {
            navigateTo(m.params.section);
        } else if (Router.path === "/settings" && root.currentRoute) {
            Router.replace("/settings/" + root.currentRoute);
        }
    }

    Connections {
        target: Router
        function onPathChanged() {
            root.syncFromRoute();
        }
    }

    Pane {
        id: navColumn
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        width: 200
        padding: 16
        clip: true

        background: Rectangle {
            color: Theme.surface
            topLeftRadius: 8
            bottomLeftRadius: 8
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: 10

            TextField {
                id: searchField
                Layout.fillWidth: true
                implicitHeight: 32
                placeholderText: qsTr("Search settings")
                color: Theme.textPrimary
                placeholderTextColor: Theme.textMuted
                font.family: Constants.regularFontFamily
                font.pixelSize: AppStyle.fontSizeSmall
                leftPadding: 12
                rightPadding: 12

                onTextChanged: root.filterText = text
                onAccepted: root.selectFirstMatch()
                Keys.onEscapePressed: searchField.text = ""

                background: Rectangle {
                    radius: height / 2
                    color: searchField.activeFocus ? Theme.surfaceHover : Theme.surfaceElevated
                    border.width: 1
                    border.color: searchField.activeFocus ? Theme.accent : Theme.border
                }
            }

            Flickable {
                id: folderList
                Layout.fillWidth: true
                Layout.fillHeight: true
                contentHeight: libraryNavColumn.implicitHeight
                boundsBehavior: Flickable.StopAtBounds

                ScrollBar.vertical: FLScrollBar {
                    anchors.right: parent.right
                    anchors.rightMargin: -4
                    width: 0
                }

                ColumnLayout {
                    id: libraryNavColumn
                    spacing: 0
                    anchors.fill: parent

                    Repeater {
                        model: root.visibleSections

                        delegate: ColumnLayout {
                            required property var modelData
                            required property int index

                            Layout.fillWidth: true
                            spacing: 0

                            // Divider between sections (not before the first).
                            Rectangle {
                                visible: index > 0
                                Layout.fillWidth: true
                                height: 1
                                color: "white"
                                opacity: 0.05
                            }

                            LibraryNavigationMenuSection {
                                Layout.fillWidth: true
                                title: modelData.title
                                collapsible: false
                                model: modelData.items

                                delegate: LibraryNavigationMenuItem {
                                    required property var model

                                    // Highlight is driven by the active route, not
                                    // by the list's own selection, so make it
                                    // non-checkable.
                                    checkable: false
                                    checked: model.route === root.currentRoute
                                    iconName: model.iconName
                                    displayText: model.displayName
                                    width: ListView.view.width

                                    onClicked: root.navigateTo(model.route)
                                }
                            }
                        }
                    }

                    Item {
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                    }
                }
            }
        }
    }

    Item {
        id: contentPane
        anchors.left: navColumn.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        // Pinned to the pane, not the content column, so it stays in the corner.
        IconButton {
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.topMargin: 12
            anchors.rightMargin: 16
            height: 32
            width: 32
            z: 1
            iconName: "close"
            onClicked: {
                Router.back()
            }
        }

        // Settings read as a single column, capped and centred. Letting them span
        // the whole pane flings each label and its control to opposite edges with
        // dead space between.
        ColumnLayout {
            id: contentColumn
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            width: Math.min(720, parent.width - 64)
            spacing: 0

            StackView {
                id: contentStack
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.topMargin: 14
                clip: true
                initialItem: appearanceSettings

                replaceEnter: Transition {
                    NumberAnimation {
                        property: "opacity"
                        from: 0.0
                        to: 1.0
                        duration: 200
                    }
                    NumberAnimation {
                        property: "y"
                        from: 30 * (root.movingDown ? 1 : -1)
                        to: 0
                        duration: 200
                        easing.type: Easing.InOutQuad
                    }
                }
                replaceExit: Transition {
                    NumberAnimation {
                        property: "opacity"
                        from: 1.0
                        to: 0.0
                        duration: 20
                    }
                    NumberAnimation {
                        property: "y"
                        from: 0
                        to: 30 * (root.movingDown ? -1 : 1)
                        duration: 200
                        easing.type: Easing.InOutQuad
                    }
                }
            }
        }
    }

    Component {
        id: emulationSettings

        GlobalEmulationSettings {
        }
    }

    Component {
        id: about

        AboutPage {}
    }

    Component {
        id: appearanceSettings

        AppearanceSettingsPage {
        }
    }
    Component {
        id: platformSettings

        PlatformSettingsPage {
        }
    }
    Component {
        id: directorySettings

        DirectorySettings {
        }
    }
    Component {
        id: soundSettings

        SoundSettings {
        }
    }
    Component {
        id: controllerSettings

        ControllerSettings {
        }
    }
    Component {
        id: librarySettings

        LibrarySettings {
        }
    }
    Component {
        id: notificationSettings

        NotificationSettings {
        }
    }
    Component {
        id: videoSettings

        VideoSettings {
        }
    }
    Component {
        id: retroAchievementSettings

        RetroAchievementSettings {
            gameRunning: root.gameRunning
        }
    }

    Component {
        id: debugSettings

        DebugPage {
        }
    }

    // Stand-in for nav items whose real page doesn't exist yet.
    Component {
        id: placeholderSettings

        Item {
            Text {
                anchors.centerIn: parent
                text: "Coming soon"
                color: Theme.textMuted
                font.family: Constants.regularFontFamily
                font.pixelSize: AppStyle.fontSizeMedium
            }
        }
    }
}
