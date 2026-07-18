import QtQuick
import QtQuick.Controls
import QtQuick.Layouts 1.0

FocusScope {
    id: root

    objectName: "LibraryNavigationMenuSection|" + title

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
            // Hidden in the collapsed rail (too narrow for a section label).
            visible: root.title !== "" && root.width > Math.round(64 * AppStyle.scale)

            RowLayout {
                anchors.fill: parent
                spacing: 2

                Text {
                    Layout.fillHeight: true
                    text: root.title
                    color: Theme.textPrimary
                    opacity: 0.7
                    font.family: Constants.regularFontFamily
                    font.pixelSize: AppStyle.fontSizeSmall
                    font.weight: Font.Medium
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    visible: root.title !== ""
                }

                FLIcon {
                    icon: "chevron-back"
                    Layout.topMargin: -3
                    Layout.bottomMargin: -4
                    Layout.fillHeight: true
                    size: 15
                    visible: root.title !== "" && root.collapsible
                    color: Theme.textPrimary
                    opacity: 0.7
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
                    margin: 8
                }

                HoverHandler {
                    enabled: root.collapsible
                    cursorShape: Qt.PointingHandCursor
                    margin: 8
                }
            }
        }

        Item {
            id: contentContainer
            property bool collapsed: false

            Layout.fillWidth: true
            Layout.topMargin: 0
            Layout.bottomMargin: collapsed ? 0 : 8
            Layout.preferredHeight: collapsed ? 0 : list.implicitHeight
            // clip: true

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

                Keys.onPressed: (event) => {
                    if (event.key === Qt.Key_Up) {
                        if (list.currentIndex > 0) {
                            list.currentIndex -= 1
                            event.accepted = true
                        }
                    } else if (event.key === Qt.Key_Down) {
                        if (list.currentIndex < list.count - 1) {
                            list.currentIndex += 1
                            event.accepted = true
                        }
                    }
                }
            }
        }
    }
}
