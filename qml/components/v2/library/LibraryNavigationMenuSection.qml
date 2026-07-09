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

    // onActiveFocusChanged: {
    //     console.log("Called active focus changed for section " + root.title + ", activeFocus: " + activeFocus + ", contentContainer.collapsed: " + contentContainer.collapsed)
    //     if (activeFocus && contentContainer.collapsed) {
    //         var next = root.KeyNavigation.down
    //         var prev = root.KeyNavigation.up
    //         if (next) next.forceActiveFocus(Qt.TabFocusReason)
    //         else if (prev) prev.forceActiveFocus(Qt.BacktabFocusReason)
    //         else focus = false
    //     }
    // }

    implicitWidth: container.implicitWidth
    implicitHeight: container.implicitHeight

    ColumnLayout {
        id: container
        anchors.fill: parent
        spacing: 0

        Pane {
            Layout.fillWidth: true
            Layout.preferredHeight: 32
            background: Rectangle {
                color: Theme.surfaceHover
            }
            visible: root.title !== ""

            RowLayout {
                anchors.fill: parent
                spacing: 8

                FLIcon {
                    icon: "arrow"
                    Layout.topMargin: -3
                    Layout.bottomMargin: -4
                    Layout.fillHeight: true
                    size: 15
                    visible: root.title !== ""
                    color: Theme.textPrimary
                    opacity: 0.8
                    rotation: contentContainer.collapsed ? 180 : 270
                    Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

                    Behavior on rotation {
                        NumberAnimation {
                            duration: 140
                            easing.type: Easing.InOutQuad
                        }
                    }
                }

                Text {
                    Layout.fillHeight: true
                    text: root.title
                    color: Theme.textPrimary
                    opacity: 0.8
                    font.family: Constants.regularFontFamily
                    font.pointSize: 11
                    font.weight: Font.DemiBold
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    visible: root.title !== ""
                }

                Item {
                    Layout.fillWidth: true
                    implicitHeight: 1
                }

                // FLIcon {
                //     Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                //     size: 22
                //     icon: "arrow-down"
                //     color: "#a1a1a1"
                //     rotation: contentContainer.collapsed ? 360 : 180
                //
                //     Behavior on rotation {
                //         NumberAnimation {
                //             duration: 160
                //             easing.type: Easing.InOutQuad
                //         }
                //     }
                //     visible: root.collapsible
                // }

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
            Layout.topMargin: collapsed ? 0 : 8
            Layout.leftMargin: 8
            Layout.rightMargin: 8
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
