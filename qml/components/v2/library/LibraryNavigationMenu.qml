import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// TODO
// A collapsible sidebar section: a title header with a chevron over a
// non-interactive list of items. Label hides in the narrow collapsed rail
FocusScope {
    id: root

    objectName: "LibraryNavigationMenu|" + title

    property string title: ""
    property alias model: list.model
    property alias delegate: list.delegate
    property alias currentIndex: list.currentIndex
    property alias count: list.count
    property alias collapsed: contentContainer.collapsed
    property bool collapsible: true

    implicitWidth: container.implicitWidth
    implicitHeight: container.implicitHeight

    ColumnLayout {
        id: container
        anchors.fill: parent
        spacing: 0

        Pane {
            Layout.fillWidth: true
            Layout.preferredHeight: 32
            horizontalPadding: 0
            background: Item {}
            visible: root.title !== "" && root.width > AppStyle.sidebarLabelThreshold

            RowLayout {
                anchors.fill: parent
                spacing: 2

                Text {
                    Layout.fillHeight: true
                    text: root.title.toUpperCase()
                    color: Theme.textMuted
                    font.family: AppStyle.fontFamily
                    font.pixelSize: AppStyle.fontSizeXSmall
                    font.weight: Font.Medium
                    font.letterSpacing: 0.5
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    visible: root.title !== ""
                }

                Icon {
                    name: "chevron-back"
                    Layout.topMargin: -3
                    Layout.bottomMargin: -4
                    Layout.fillHeight: true
                    size: AppStyle.iconSizeSm
                    visible: root.title !== "" && root.collapsible
                    color: Theme.textMuted
                    rotation: contentContainer.collapsed ? 180 : 270
                    Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

                    Behavior on rotation {
                        NumberAnimation {
                            duration: 140
                            easing.type: Easing.InOutQuad
                        }
                    }
                }

                Item {
                    Layout.fillWidth: true
                    implicitHeight: 1
                }

                TapHandler {
                    enabled: root.collapsible
                    onTapped: contentContainer.collapsed = !contentContainer.collapsed
                    margin: AppStyle.spacingSm
                }

                HoverHandler {
                    enabled: root.collapsible
                    cursorShape: Qt.PointingHandCursor
                    margin: AppStyle.spacingSm
                }
            }
        }

        Item {
            id: contentContainer
            property bool collapsed: false

            Layout.fillWidth: true
            Layout.topMargin: 0
            Layout.bottomMargin: collapsed ? 0 : AppStyle.spacingSm
            Layout.preferredHeight: collapsed ? 0 : list.implicitHeight

            Behavior on Layout.preferredHeight {
                NumberAnimation {
                    duration: 160
                    easing.type: Easing.InOutQuad
                }
            }

            ListView {
                id: list
                width: parent.width
                implicitHeight: contentHeight
                interactive: false
                visible: !contentContainer.collapsed
                focus: true

                Keys.onPressed: event => {
                    if (event.key === Qt.Key_Up) {
                        if (list.currentIndex > 0) {
                            list.currentIndex -= 1;
                            event.accepted = true;
                        }
                    } else if (event.key === Qt.Key_Down) {
                        if (list.currentIndex < list.count - 1) {
                            list.currentIndex += 1;
                            event.accepted = true;
                        }
                    }
                }
            }
        }
    }
}
