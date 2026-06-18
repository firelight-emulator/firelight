import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import QtQuick.Layouts 1.0
import Firelight 1.0

FocusScope {
    id: control

    Keys.onEscapePressed: function(event) {
        if (searchField.activeFocus || searchPopup.activeFocus) {
            if (searchField.text !== "") {
                searchField.text = ""
                searchField.forceActiveFocus()
                event.accepted = true
            } else {
                control.Window.window.contentItem.forceActiveFocus()
                event.accepted = true
            }
        }
    }

    Pane {
        anchors.fill: parent
        padding: 4
        topPadding: 3
        focus: true

        background: Rectangle {
            color: Qt.lighter("#12131A", searchField.activeFocus || searchPopup.activeFocus ? 1.4 : 1.32)
            bottomLeftRadius: height / 2
            bottomRightRadius: height / 2
            topLeftRadius: height / 2
            topRightRadius: height / 2

            Behavior on color {
                ColorAnimation {
                    duration: 240
                    easing.type: Easing.Linear
                }
            }

            Rectangle {
                anchors.fill: parent
                color: "transparent"
                border.color: "white"
                border.width: 2
                radius: height / 2
                opacity: searchField.activeFocus || searchPopup.activeFocus ? 0.8 : 0

                Behavior on opacity {
                    NumberAnimation {
                        duration: 240
                        easing.type: Easing.Linear
                    }
                }
            }

        }

        contentItem: RowLayout {
            Button {
                Layout.fillHeight: true
                Layout.preferredWidth: height
                Layout.topMargin: 4
                Layout.margins: 3
                focusPolicy: Qt.NoFocus

                padding: 0

                background: Rectangle {
                    color: "transparent"
                }

                HoverHandler {
                    id: magnifyingGlassHover
                    cursorShape: Qt.PointingHandCursor
                }

                Image {
                    id: searchIcon
                    anchors.fill: parent
                    source: "qrc:/icons/search"
                    sourceSize.width: 32
                    sourceSize.height: 32
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                    opacity: searchField.activeFocus || searchPopup.activeFocus || magnifyingGlassHover.hovered ? 0.9 : 0.5

                    Behavior on opacity {
                        NumberAnimation {
                            duration: 200
                            easing.type: Easing.InOutQuad
                        }
                    }
                }

                onClicked: {
                    searchField.forceActiveFocus(Qt.MouseFocusReason)
                }
            }

            TextField {
                id: searchField
                leftPadding: 0
                focus: true

                Layout.fillHeight: true
                Layout.fillWidth: true

                placeholderText: qsTr("Search (Ctrl + F)")
                font.pointSize: 12
                color: "white"
                font.family: Constants.regularFontFamily
                background: Rectangle {
                    color: "transparent"
                }
            }

            Button {
                Layout.fillHeight: true
                Layout.preferredWidth: height
                Layout.margins: 5
                focusPolicy: Qt.NoFocus

                padding: 0
                visible: searchField.text !== ""

                background: Rectangle {
                    color: "transparent"
                }

                HoverHandler {
                    id: hover
                    cursorShape: Qt.PointingHandCursor
                }

                Image {
                    id: cancelButton
                    anchors.fill: parent
                    source: "qrc:/icons/cancel"
                    sourceSize.width: 32
                    sourceSize.height: 32
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                    opacity: parent.down ? 0.4 : hover.hovered ? 0.7 : 0.5
                }

                onClicked: {
                    searchField.text = ""
                }
            }

        }
    }

    Popup {
        id: searchPopup

        y: control.height + 8
        width: parent.width
        height: 400
        padding: 8
        clip: true

        closePolicy: Popup.NoAutoClose

        visible: searchField.activeFocus || searchPopup.activeFocus

        background: Rectangle {
            color: Qt.lighter("#12131A", 1.4)
            radius: 8
            border.color: Qt.lighter("#12131A", 2)
            border.width: 1
            // bottomRightRadius: 8
            // bottomLeftRadius: 8
        }

        contentItem: ListView {
            model: 20
            delegate: Button {
                required property var index
                required property var model

                width: ListView.view.width
                height: 64

                background: Rectangle {
                    color: "white"
                    opacity: hovered ? 0.08 : 0
                    radius: 4
                }

                contentItem: RowLayout {
                    Rectangle {
                        color: "red"
                        Layout.fillHeight: true
                        Layout.preferredWidth: height
                    }

                    Text {
                        text: "Result " + index
                        font.pointSize: 14
                        font.family: Constants.regularFontFamily
                        font.weight: Font.DemiBold
                        color: "white"
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }
                }

            }
        }

        // enter: Transition {
        //     NumberAnimation { properties: "height"; from: 0; to: 500; duration: 120 }
        // }
        //
        // exit: Transition {
        //     NumberAnimation { properties: "height"; from: height; to: 0; duration: 120 }
        // }
    }
}