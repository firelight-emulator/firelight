import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

// Two-column Help browser: a topic list on the left, the rendered Markdown
// article on the right. Mirrors the Settings screen's layout. Topics are
// declared inline; each points at a bundled qrc:/help/*.md file
Pane {
    id: root

    property int currentIndex: 0

    readonly property var articles: [
        {
            title: "Getting Started",
            source: "qrc:/help/getting-started.md"
        },
        {
            title: "The Library",
            source: "qrc:/help/the-library.md"
        },
        {
            title: "Playing Games",
            source: "qrc:/help/playing-games.md"
        },
        {
            title: "Controllers",
            source: "qrc:/help/controllers.md"
        },
        {
            title: "Achievements",
            source: "qrc:/help/achievements.md"
        },
        {
            title: "Artwork & Metadata",
            source: "qrc:/help/artwork-and-metadata.md"
        },
        {
            title: "Cheats",
            source: "qrc:/help/cheats.md"
        },
        {
            title: "Screenshots & Clips",
            source: "qrc:/help/captures.md"
        },
        {
            title: "Settings",
            source: "qrc:/help/settings.md"
        },
        {
            title: "Troubleshooting",
            source: "qrc:/help/troubleshooting.md"
        }
    ]

    background: Item {}
    horizontalPadding: AppStyle.windowPadding
    verticalPadding: 16

    RowLayout {
        anchors.fill: parent
        spacing: 24

        ListView {
            id: menu
            Layout.preferredWidth: 260
            Layout.fillHeight: true

            interactive: true
            keyNavigationEnabled: true
            focus: true
            currentIndex: root.currentIndex
            model: root.articles

            delegate: FirelightMenuItem {
                required property int index
                required property var modelData

                width: ListView.view.width
                height: 48
                labelText: modelData.title
                checked: ListView.isCurrentItem

                onClicked: root.currentIndex = index
                onToggled: {
                    if (checked) {
                        root.currentIndex = index;
                    }
                }
            }
        }

        HelpArticle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            source: root.articles[root.currentIndex].source
        }
    }
}
