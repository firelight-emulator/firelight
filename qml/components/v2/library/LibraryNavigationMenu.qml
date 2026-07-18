import QtQuick
import QtQuick.Layouts

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

        RowLayout {
            Layout.fillWidth: true

            Text {
                Layout.fillHeight: true
                text: root.title
                color: Theme.textMuted
                font.family: Constants.regularFontFamily
                font.pixelSize: AppStyle.fontSizeMedium
                font.weight: Font.DemiBold
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
            }

            Item {
                Layout.fillWidth: true
                implicitHeight: 1
            }

            FLIcon {
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                size: 22
                icon: "arrow-down"
                color: Theme.textMuted
                rotation: contentContainer.collapsed ? 360 : 180

                Behavior on rotation {
                    NumberAnimation {
                        duration: 160
                        easing.type: Easing.InOutQuad
                    }
                }
                visible: root.collapsible
            }

            TapHandler {
                enabled: root.collapsible
                onTapped: contentContainer.collapsed = !contentContainer.collapsed
            }

            HoverHandler {
                enabled: root.collapsible
                cursorShape: Qt.PointingHandCursor
            }
        }

        Item {
            id: contentContainer
            property bool collapsed: false

            Layout.fillWidth: true
            Layout.topMargin: 8
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
