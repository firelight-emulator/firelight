import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

FocusScope {
    id: root

    property bool gameRunning: false
    // Slide direction for the content transition (down = later item selected)
    property bool movingDown: true

    // TODO
    // Single source of truth for the nav taxonomy. Each item's `page` references
    // a Component id defined at the bottom of this file. Grouped equivalent of
    // FLTwoColumnMenu's parallel menuItems/routeNames/pages
    //
    // Search terms are NOT here: they're declared per page in the settings
    // catalog, alongside the settings themselves, so one index covers both
    property var sections: [
        {
            "title": "",
            "items": [
                { "displayName": "Appearance", "iconName": "palette", "route": "appearance", "page": appearanceSettings },
                { "displayName": "System", "iconName": "display", "route": "system", "page": systemSettings },
                { "displayName": "Emulation", "iconName": "controller", "route": "emulation", "page": emulationSettings },
                { "displayName": "Controllers", "iconName": "controller", "route": "controllers", "page": controllerSettings },
                { "displayName": "Notifications", "iconName": "bell", "route": "notifications", "page": notificationSettings },
                { "displayName": "Captures", "iconName": "photo-library", "route": "captures", "page": placeholderSettings },
                { "displayName": "Achievements", "iconName": "trophy", "route": "retroachievements", "page": retroAchievementSettings },
                { "displayName": "About", "iconName": "info", "route": "about", "page": about }
            ]
        }
        // TODO
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

    // The currently selected item, driven by clicks and by the URL
    property string currentRoute: ""
    property string currentTitle: ""
    property int currentFlatIndex: -1

    // The setting a search result asked us to reveal, handed to the page it
    // lives on. Only auto-rendered pages can act on it
    property string highlightKey: ""

    // Searches every declared page AND setting, so "vsync" finds the Sync
    // method row itself, not just the page it happens to sit on
    SettingsSearchModel {
        id: searchModel
    }

    readonly property bool searching: searchModel.query.trim() !== ""

    // A hit's route is a full path ("/settings/emulation"); letting the Router
    // own the change keeps one path in and out of this screen
    function openResult(route, key) {
        root.highlightKey = key;
        searchField.text = "";
        if (Router.isActive("/settings") && route !== "") {
            Router.replace(route);
        }
    }

    function openTopResult() {
        if (searchModel.count > 0) {
            openResult(searchModel.topRoute(), searchModel.topKey());
        }
    }

    Component.onCompleted: {
        const route = routeFromPath();
        // initialItem already shows the first item; only deep-links need a swap
        applyRoute(route, route !== sections[0].items[0].route);
        syncFromRoute();
    }

    // Resolve the route the URL is asking for, falling back to the first item
    function routeFromPath() {
        const m = Router.match(Router.path, ["/settings/:section"]);
        if (m.matched && itemForRoute(m.params.section)) {
            return m.params.section;
        }
        return sections[0].items[0].route;
    }

    // Find an item + its flat index (across all sections) by route
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

    // Select a route; swaps the content page (with transition) when animate
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

    // A user selection: swap the page and reflect it in the URL
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

    // Keep the selected page and the /settings/<section> URL in sync
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
        width: Math.round(250 * AppStyle.scale)
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

            FLSearchField {
                id: searchField
                Layout.fillWidth: true
                placeholder: qsTr("Search settings")
                onTextChanged: searchModel.query = text
                onAccepted: root.openTopResult()
            }

            // Results take over the column while a query is live; the nav comes
            // back the moment it's cleared
            ListView {
                id: resultList
                visible: root.searching
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: searchModel
                spacing: 2
                boundsBehavior: Flickable.StopAtBounds

                ScrollBar.vertical: FLScrollBar {
                    anchors.right: parent.right
                    anchors.rightMargin: -4
                    width: 0
                }

                delegate: ItemDelegate {
                    id: resultDelegate
                    required property var model

                    width: ListView.view.width
                    padding: AppStyle.spacingSm
                    leftPadding: AppStyle.spacingMd
                    rightPadding: AppStyle.spacingMd
                    // Content-driven so a scaled two-line result doesn't clip
                    height: Math.max(AppStyle.rowHeight, resultContent.implicitHeight + topPadding + bottomPadding)
                    hoverEnabled: true

                    HoverHandler { cursorShape: Qt.PointingHandCursor }

                    onClicked: root.openResult(model.route, model.key)

                    background: Rectangle {
                        radius: AppStyle.radiusMd
                        color: Theme.textPrimary
                        opacity: resultDelegate.hovered ? 0.10 : 0
                    }

                    contentItem: ColumnLayout {
                        id: resultContent
                        spacing: 0

                        Text {
                            Layout.fillWidth: true
                            text: resultDelegate.model.label
                            color: Theme.textPrimary
                            font.family: Constants.regularFontFamily
                            font.pixelSize: AppStyle.fontSizeSmall
                            font.weight: Font.DemiBold
                            elide: Text.ElideRight
                        }

                        // Where the hit lives. A page hit is its own answer, so
                        // it needs no breadcrumb
                        Text {
                            Layout.fillWidth: true
                            visible: !resultDelegate.model.isPage
                            text: resultDelegate.model.groupLabel === ""
                                  ? resultDelegate.model.pageLabel
                                  : resultDelegate.model.pageLabel + " › " + resultDelegate.model.groupLabel
                            color: Theme.textMuted
                            font.family: Constants.regularFontFamily
                            font.pixelSize: AppStyle.fontSizeSmall - 2
                            elide: Text.ElideRight
                        }
                    }
                }
            }

            Text {
                visible: root.searching && searchModel.count === 0
                Layout.fillWidth: true
                Layout.topMargin: 8
                text: qsTr("No settings found")
                color: Theme.textMuted
                horizontalAlignment: Text.AlignHCenter
                font.family: Constants.regularFontFamily
                font.pixelSize: AppStyle.fontSizeSmall
            }

            Flickable {
                id: folderList
                visible: !root.searching
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
                        model: root.sections

                        delegate: ColumnLayout {
                            required property var modelData
                            required property int index

                            Layout.fillWidth: true
                            spacing: 0

                            // Divider between sections (not before the first)
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

                                    // TODO
                                    // Highlight is driven by the active route, not
                                    // by the list's own selection, so make it
                                    // non-checkable
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

        // Pinned to the pane, not the content column, so it stays in the corner
        FLIconButton {
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.topMargin: 12
            anchors.rightMargin: 16
            z: 1
            iconName: "close"
            tooltipText: "Close"
            onClicked: {
                Router.back()
            }
        }

        // TODO
        // Settings read as a single column, capped and centred. Letting them span
        // the whole pane flings each label and its control to opposite edges with
        // dead space between. The cap scales with the UI so enlarged controls get
        // proportional room, but never exceeds the pane (reflow-safe)
        ColumnLayout {
            id: contentColumn
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            width: Math.min(Math.round(720 * AppStyle.scale), parent.width - 64)
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
        id: systemSettings

        SystemSettingsPage {
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

    // Stand-in for nav items whose real page doesn't exist yet
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
