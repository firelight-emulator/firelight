import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

FocusScope {
    id: root

    property Component headerLeading: Component {
        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: "Settings"
            color: Theme.textPrimary
            font.family: AppStyle.fontFamily
            font.pixelSize: AppStyle.fontSizeLarge
            font.weight: Font.Medium
        }
    }

    property bool gameRunning: false
    // Slide direction for the content transition (down = later item selected)
    property bool movingDown: true

    // Single source of truth for the nav taxonomy. Each item's `page` references
    // a Component id defined at the bottom of this file. Grouped equivalent of
    // FLTwoColumnMenu's parallel menuItems/routeNames/pages
    //
    // Search terms are NOT here: they're declared per page in the settings
    // catalog, alongside the settings themselves, so one index covers both
    property var sections: [
        {
            "title": "System",
            "items": [
                {
                    "displayName": "Video",
                    "iconName": "display",
                    "route": "system-video",
                    "page": placeholderSettings
                },
                {
                    "displayName": "Audio",
                    "iconName": "display",
                    "route": "system-audio",
                    "page": placeholderSettings
                }
            ]
        },
        {
            "title": "Emulation",
            "items": [
                {
                    "displayName": "General",
                    "iconName": "display",
                    "route": "emulation-general",
                    "page": placeholderSettings
                },
                {
                    "displayName": "Picture",
                    "iconName": "display",
                    "route": "emulation-picture",
                    "page": placeholderSettings
                },
                {
                    "displayName": "Sound",
                    "iconName": "display",
                    "route": "emulation-sound",
                    "page": placeholderSettings
                }
            ]
        },
        {
            "title": "Emulation",
            "items": [
                {
                    "displayName": "Picture",
                    "iconName": "display",
                    "route": "emulation-picture",
                    "page": placeholderSettings
                },
                {
                    "displayName": "Sound",
                    "iconName": "display",
                    "route": "emulation-picture",
                    "page": placeholderSettings
                }
            ]
        },
        {
            "title": "Main stuff",
            "items": [
                {
                    "displayName": "Appearance",
                    "iconName": "palette",
                    "route": "appearance",
                    "page": appearanceSettings
                },
                {
                    "displayName": "System",
                    "iconName": "display",
                    "route": "system",
                    "page": systemSettings
                },
                {
                    "displayName": "Emulation",
                    "iconName": "controller",
                    "route": "emulation",
                    "page": emulationSettings
                },
                {
                    "displayName": "Controllers",
                    "iconName": "controller",
                    "route": "controllers",
                    "page": controllerSettings
                },
                {
                    "displayName": "Notifications",
                    "iconName": "bell",
                    "route": "notifications",
                    "page": notificationSettings
                },
                {
                    "displayName": "Captures",
                    "iconName": "photo-library",
                    "route": "captures",
                    "page": placeholderSettings
                },
                {
                    "displayName": "Achievements",
                    "iconName": "trophy",
                    "route": "retroachievements",
                    "page": retroAchievementSettings
                },
                {
                    "displayName": "About",
                    "iconName": "info",
                    "route": "about",
                    "page": about
                }
            ]
        },
        {
            "title": "testing",
            "items": [
                {
                    "displayName": "Appearance",
                    "iconName": "palette",
                    "route": "appearance",
                    "page": appearanceSettings
                },
                {
                    "displayName": "System",
                    "iconName": "display",
                    "route": "system",
                    "page": systemSettings
                }
            ]
        }
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
                    return {
                        "item": sections[s].items[i],
                        "flatIndex": flat
                    };
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

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: AppStyle.windowPadding
        anchors.rightMargin: AppStyle.windowPadding

        FLScrollColumnLayout {
            Layout.fillHeight: true
            Layout.preferredWidth: Math.round(250 * AppStyle.scale)
            Layout.leftMargin: AppStyle.spacingLg
            Layout.rightMargin: AppStyle.spacingLg

            spacing: AppStyle.spacingMd

            FLSearchField {
                Layout.topMargin: AppStyle.spacingMd + AppStyle.spacingSm
                Layout.fillWidth: true
            }

            FLDivider {
                Layout.topMargin: AppStyle.spacingSm
                Layout.fillWidth: true
            }

            Repeater {
                model: root.sections

                delegate: FLColumnLayout {
                    id: navSection
                    required property var modelData
                    required property int index

                    Layout.fillWidth: true
                    spacing: 0

                    // Divider between sections (not before the first)
                    FLDivider {
                        Layout.topMargin: AppStyle.spacingSm
                        Layout.bottomMargin: AppStyle.spacingSm
                        visible: index > 0
                    }

                    Text {
                        Layout.topMargin: AppStyle.spacingSm
                        Layout.bottomMargin: AppStyle.spacingSm
                        color: Theme.textMuted
                        text: navSection.modelData.title
                        font.pixelSize: AppStyle.fontSizeSmall
                        font.family: AppStyle.fontFamily
                        font.weight: Font.Normal
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignLeft
                    }

                    Repeater {
                        model: navSection.modelData.items
                        delegate: FLButtonBase {
                            id: button
                            required property var modelData
                            required property var index

                            Layout.fillWidth: true
                            Layout.preferredHeight: 50
                            variant: "subtle"
                            rounded: false
                            focus: index === 0

                            FLFocus.focusSound: SoundEffects.menuItemFocus

                            checkedColor: Theme.switch2Color

                            checkable: false
                            checked: modelData.route === root.currentRoute

                            onActiveFocusChanged: {
                                if (activeFocus) {
                                    root.navigateTo(modelData.route)
                                }
                            }

                            onClicked: {
                                root.navigateTo(modelData.route)
                                if (!InputMethodManager.usingMouse) {
                                    contentStack.forceActiveFocus();
                                }
                            }

                            contentItem: Row {
                                spacing: 12
                                Item {
                                    y: AppStyle.spacingMd / 2
                                    height: parent.height - AppStyle.spacingMd
                                    width: 4

                                    Rectangle {
                                        anchors.fill: parent
                                        color: Theme.switch2Color
                                        visible: button.checked
                                    }
                                }

                                Text {
                                    color: button._fg
                                    text: button.modelData.displayName
                                    font.pixelSize: AppStyle.fontSizeMedium
                                    font.family: AppStyle.fontFamily
                                    font.weight: Font.DemiBold
                                    height: parent.height
                                    verticalAlignment: Text.AlignVCenter
                                    horizontalAlignment: Text.AlignLeft
                                }
                            }
                        }
                    }

                    // LibraryNavigationMenu {
                    //     Layout.fillWidth: true
                    //     focus: true
                    //     title: modelData.title
                    //     collapsible: false
                    //     model: modelData.items
                    //
                    //     delegate: LibraryNavigationMenuItem {
                    //         required property var model
                    //         required property int index
                    //
                    //         // Highlight is driven by the active route, not
                    //         // by the list's own selection, so make it
                    //         // non-checkable
                    //         checkable: false
                    //         checked: model.route === root.currentRoute
                    //         iconName: model.iconName
                    //         displayText: model.displayName
                    //         width: ListView.view.width
                    //
                    //         // TODO
                    //         // Current row holds focus so the ring follows
                    //         // arrow/dpad moves; sync back so Tab and arrows
                    //         // stay in step
                    //         focus: ListView.isCurrentItem
                    //         onActiveFocusChanged: if (activeFocus) {
                    //             ListView.view.currentIndex = index;
                    //         }
                    //
                    //         onClicked: root.navigateTo(model.route)
                    //     }
                    // }
                }
            }

            Item {
                Layout.fillHeight: true
                Layout.fillWidth: true
            }
        }

        FLColumnDivider {
            Layout.fillHeight: true
        }

        Flickable {
            id: contentScrollView
            Layout.fillWidth: true
            Layout.fillHeight: true

            Layout.topMargin: AppStyle.spacingLg
            contentWidth: width
            contentHeight: contentStack.height
            flickableDirection: Flickable.VerticalFlick
            boundsBehavior: Flickable.StopAtBounds

            StackView {
                id: contentStack

                anchors.horizontalCenter: parent.horizontalCenter
                width: Math.min(Math.round(720 * AppStyle.scale), parent.width - 64)
                height: Math.max(contentScrollView.height, contentStack.currentItem ? contentStack.currentItem.implicitHeight : 0)

                clip: true
                initialItem: appearanceSettings
                focus: true

                replaceEnter: Transition {
                    NumberAnimation {
                        property: "opacity"
                        from: 0.0
                        to: 1.0
                        duration: AppStyle.durationBase
                    }
                    NumberAnimation {
                        property: "y"
                        from: 30 * (root.movingDown ? 1 : -1)
                        to: 0
                        duration: AppStyle.durationBase
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
                        duration: AppStyle.durationBase
                        easing.type: Easing.InOutQuad
                    }
                }
            }
        }



        // FLScrollColumnLayout {
        //     id: contentPane
        //     Layout.fillWidth: true
        //     Layout.fillHeight: true
        //
        //
        // }

        // FocusScope {
        //     id: contentPane
        //     Layout.fillWidth: true
        //     Layout.fillHeight: true
        //
        //     // Settings read as a single column, capped and centred. Letting them span
        //     // the whole pane flings each label and its control to opposite edges with
        //     // dead space between. The cap scales with the UI so enlarged controls get
        //     // proportional room, but never exceeds the pane (reflow-safe)
        //     ColumnLayout {
        //         id: contentColumn
        //         anchors.top: parent.top
        //         anchors.bottom: parent.bottom
        //         anchors.horizontalCenter: parent.horizontalCenter
        //         width: Math.min(Math.round(720 * AppStyle.scale), parent.width - 64)
        //         spacing: 0
        //
        //
        //     }
        // }
    }

    // Pane {
    //     id: navColumn
    //     anchors.top: parent.top
    //     anchors.bottom: parent.bottom
    //     anchors.left: parent.left
    //     anchors.leftMargin: AppStyle.windowPadding
    //     width: Math.round(310 * AppStyle.scale)
    //     padding: AppStyle.spacingLg
    //     clip: true
    //
    //     background: Item {}
    //
    //
    //
    //     // ColumnLayout {
    //     //     anchors.fill: parent
    //     //     spacing: AppStyle.spacingMd
    //
    //         // FLSearchField {
    //         //     id: searchField
    //         //     Layout.fillWidth: true
    //         //     placeholder: qsTr("Search settings")
    //         //     onTextChanged: searchModel.query = text
    //         //     onAccepted: root.openTopResult()
    //         // }
    //         //
    //         // // Results take over the column while a query is live; the nav comes
    //         // // back the moment it's cleared
    //         // ListView {
    //         //     id: resultList
    //         //     visible: root.searching
    //         //     Layout.fillWidth: true
    //         //     Layout.fillHeight: true
    //         //     clip: true
    //         //     model: searchModel
    //         //     spacing: 2
    //         //     boundsBehavior: Flickable.StopAtBounds
    //         //
    //         //     ScrollBar.vertical: FLScrollBar {
    //         //         anchors.right: parent.right
    //         //         anchors.rightMargin: -4
    //         //         width: 0
    //         //     }
    //         //
    //         //     delegate: ItemDelegate {
    //         //         id: resultDelegate
    //         //         required property var model
    //         //
    //         //         width: ListView.view.width
    //         //         padding: AppStyle.spacingSm
    //         //         leftPadding: AppStyle.spacingMd
    //         //         rightPadding: AppStyle.spacingMd
    //         //         // Content-driven so a scaled two-line result doesn't clip
    //         //         height: Math.max(AppStyle.rowHeight, resultContent.implicitHeight + topPadding + bottomPadding)
    //         //         hoverEnabled: true
    //         //
    //         //         HoverHandler {
    //         //             cursorShape: Qt.PointingHandCursor
    //         //         }
    //         //
    //         //         onClicked: root.openResult(model.route, model.key)
    //         //
    //         //         background: Rectangle {
    //         //             radius: AppStyle.radiusMd
    //         //             color: resultDelegate.hovered ? Theme.surfaceHover : "transparent"
    //         //         }
    //         //
    //         //         contentItem: ColumnLayout {
    //         //             id: resultContent
    //         //             spacing: 0
    //         //
    //         //             Text {
    //         //                 Layout.fillWidth: true
    //         //                 text: resultDelegate.model.label
    //         //                 color: Theme.textPrimary
    //         //                 font.family: AppStyle.fontFamily
    //         //                 font.pixelSize: AppStyle.fontSizeSmall
    //         //                 font.weight: Font.DemiBold
    //         //                 elide: Text.ElideRight
    //         //             }
    //         //
    //         //             // Where the hit lives. A page hit is its own answer, so
    //         //             // it needs no breadcrumb
    //         //             Text {
    //         //                 Layout.fillWidth: true
    //         //                 visible: !resultDelegate.model.isPage
    //         //                 text: resultDelegate.model.groupLabel === "" ? resultDelegate.model.pageLabel : resultDelegate.model.pageLabel + " › " + resultDelegate.model.groupLabel
    //         //                 color: Theme.textMuted
    //         //                 font.family: AppStyle.fontFamily
    //         //                 font.pixelSize: AppStyle.fontSizeSmall - 2
    //         //                 elide: Text.ElideRight
    //         //             }
    //         //         }
    //         //     }
    //         // }
    //         //
    //         // Text {
    //         //     visible: root.searching && searchModel.count === 0
    //         //     Layout.fillWidth: true
    //         //     Layout.topMargin: AppStyle.spacingSm
    //         //     text: qsTr("No settings found")
    //         //     color: Theme.textMuted
    //         //     horizontalAlignment: Text.AlignHCenter
    //         //     font.family: AppStyle.fontFamily
    //         //     font.pixelSize: AppStyle.fontSizeSmall
    //         // }
    //
    //
    //
    //         // Flickable {
    //         //     id: folderList
    //         //     visible: !root.searching
    //         //     Layout.fillWidth: true
    //         //     Layout.fillHeight: true
    //         //     contentHeight: libraryNavColumn.implicitHeight
    //         //     boundsBehavior: Flickable.StopAtBounds
    //         //
    //         //     ScrollBar.vertical: FLScrollBar {
    //         //         anchors.right: parent.right
    //         //         anchors.rightMargin: -4
    //         //         width: 0
    //         //     }
    //         //
    //         //     ColumnLayout {
    //         //         id: libraryNavColumn
    //         //         spacing: 0
    //         //         anchors.fill: parent
    //         //
    //         //         Repeater {
    //         //             model: root.sections
    //         //
    //         //             delegate: ColumnLayout {
    //         //                 required property var modelData
    //         //                 required property int index
    //         //
    //         //                 Layout.fillWidth: true
    //         //                 spacing: 0
    //         //
    //         //                 // Divider between sections (not before the first)
    //         //                 FLDivider {
    //         //                     visible: index > 0
    //         //                 }
    //         //
    //         //                 LibraryNavigationMenu {
    //         //                     Layout.fillWidth: true
    //         //                     focus: true
    //         //                     title: modelData.title
    //         //                     collapsible: false
    //         //                     model: modelData.items
    //         //
    //         //                     delegate: LibraryNavigationMenuItem {
    //         //                         required property var model
    //         //                         required property int index
    //         //
    //         //                         // Highlight is driven by the active route, not
    //         //                         // by the list's own selection, so make it
    //         //                         // non-checkable
    //         //                         checkable: false
    //         //                         checked: model.route === root.currentRoute
    //         //                         iconName: model.iconName
    //         //                         displayText: model.displayName
    //         //                         width: ListView.view.width
    //         //
    //         //                         // TODO
    //         //                         // Current row holds focus so the ring follows
    //         //                         // arrow/dpad moves; sync back so Tab and arrows
    //         //                         // stay in step
    //         //                         focus: ListView.isCurrentItem
    //         //                         onActiveFocusChanged: if (activeFocus) {
    //         //                             ListView.view.currentIndex = index;
    //         //                         }
    //         //
    //         //                         onClicked: root.navigateTo(model.route)
    //         //                     }
    //         //                 }
    //         //             }
    //         //         }
    //         //
    //         //         Item {
    //         //             Layout.fillHeight: true
    //         //             Layout.fillWidth: true
    //         //         }
    //         //     }
    //         // }
    //     // }
    // }



    Component {
        id: emulationSettings

        GlobalEmulationSettings {}
    }

    Component {
        id: about

        AboutPage {}
    }

    Component {
        id: appearanceSettings

        SettingsPage {
            page: "appearance"
        }
    }
    Component {
        id: systemSettings

        SettingsPage {
            page: "system"
        }
    }
    Component {
        id: controllerSettings

        ControllerSettings {}
    }
    Component {
        id: notificationSettings

        SettingsPage {
            page: "notifications"
        }
    }
    Component {
        id: retroAchievementSettings

        RetroAchievementSettings {
            gameRunning: root.gameRunning
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
                font.family: AppStyle.fontFamily
                font.pixelSize: AppStyle.fontSizeMedium
            }
        }
    }
}
