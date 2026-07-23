import QtQuick
import QtQuick.Controls

// Renders a bundled Markdown help article. `source` is a qrc:/help/*.md URL;
// the file is read into a string and shown with Qt's native Markdown rendering
// In-app route links (e.g. /settings) navigate via the Router; web links open
// in the browser
Flickable {
    id: root

    property url source

    contentWidth: width
    contentHeight: article.implicitHeight + 32
    clip: true
    boundsBehavior: Flickable.StopAtBounds

    ScrollBar.vertical: FLScrollBar {
        width: 8
    }

    Text {
        id: article
        x: 4
        y: 8
        width: Math.min(root.width - 32, 820)

        textFormat: Text.MarkdownText
        text: String(root.source) !== "" ? FilesystemUtils.readTextFile(root.source) : ""
        wrapMode: Text.WordWrap
        color: "white"
        font.family: AppStyle.fontFamily
        font.pixelSize: AppStyle.fontSizeMedium
        lineHeight: 1.3
        linkColor: ColorPalette.hyperlinkColor

        onLinkActivated: function (link) {
            if (link.startsWith("http://") || link.startsWith("https://")) {
                Qt.openUrlExternally(link);
            } else {
                Router.navigateTo(link);
            }
        }
    }
}
