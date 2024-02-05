import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FirelightStyle 1.0

Rectangle {
    property string fontFamilyName

    signal entryClicked(string entryId)

    color: "transparent"
    Pane {
        id: content
        background: Rectangle {
            color: Constants.colorTestSurface
            radius: 12
        }

        // padding: 12
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        width: 300

        // minimumWidth: 200
        // maximumWidth: 400


        Rectangle {
            id: buttonsAtTop2Lol
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 48
            color: "transparent"

            RowLayout {
                anchors.fill: parent
                Button {
                    Layout.alignment: Qt.AlignVCenter
                    text: "All"
                    background: Rectangle {
                        color: "white"
                        radius: 24
                    }
                }
                Button {
                    Layout.alignment: Qt.AlignVCenter
                    text: "Favorites"
                    background: Rectangle {
                        color: "white"
                        radius: 24
                    }
                }
                Button {
                    Layout.alignment: Qt.AlignVCenter
                    text: "Recently Played"
                    background: Rectangle {
                        color: "white"
                        radius: 24
                    }
                }
            }
        }

        Rectangle {
            id: horizontalRule2
            width: parent.width
            height: 1
            anchors.top: buttonsAtTop2Lol.bottom
            color: Constants.colorTestTextMuted
        }

        // Rectangle {
        //     id: testItem
        //     width: 200
        //     height: 160
        //     color: Constants.colorTestSurfaceContainerLowest
        //     radius: 12
        // }
    }

    Pane {
        id: contentRight
        background: Rectangle {
            color: Constants.colorTestSurface
            radius: 12
        }

        // padding: 12

        anchors.leftMargin: 12
        anchors.left: content.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        Rectangle {
            id: buttonsAtTopLol
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right

            height: 36
            color: "transparent"

            RowLayout {
                anchors.fill: parent
                Button {
                    Layout.alignment: Qt.AlignTop | Qt.AlignRight
                    Layout.fillHeight: true
                    text: "Recently Played"
                    background: Rectangle {
                        color: "white"
                        radius: 24
                    }
                }
            }
        }

        Rectangle {
            id: horizontalRule
            width: parent.width
            height: 1
            anchors.top: buttonsAtTopLol.bottom
            anchors.topMargin: 12
            color: Constants.colorTestTextMuted
        }


        ListView {
            id: libraryList
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: horizontalRule.bottom
            anchors.topMargin: 12
            focus: true
            clip: true

            ScrollBar.vertical: ScrollBar {
                width: 15
                interactive: true
            }

            currentIndex: 0

            model: library_short_model
            boundsBehavior: Flickable.StopAtBounds
            delegate: gameListItem
            // preferredHighlightBegin: height / 3
            // preferredHighlightEnd: 2 * (height / 3) + currentItem.height
        }

        Component {
            id: gameListItem

            Rectangle {
                id: wrapper

                width: ListView.view.width
                height: 60
                radius: 12

                color: mouseArea.containsMouse ? Constants.colorTestCard : "transparent"

                MouseArea {
                    id: mouseArea
                    hoverEnabled: true
                    anchors.fill: parent

                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        entryClicked(model.id)
                        // gameLoader.loadGame(model.id)
                    }
                }

                // Item {
                //     id: picture
                //     width: 100
                //     height: parent.height
                // }

                Text {
                    id: label
                    anchors.top: parent.top
                    anchors.topMargin: 5
                    anchors.left: parent.left
                    anchors.leftMargin: 10

                    height: parent.height / 2
                    font.pointSize: 12
                    font.family: fontFamilyName
                    text: model.display_name
                    color: Constants.colorTestText
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                }

                Text {
                    id: bottomLabel
                    anchors.top: label.bottom
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 10
                    anchors.left: parent.left
                    anchors.leftMargin: 10

                    // font.family: lexendLight.name
                    font.pointSize: 10
                    text: "Nintendo 64"
                    color: "#989898"
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }
    }
}