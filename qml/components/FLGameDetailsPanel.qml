import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import Firelight 1.0

ColumnLayout {
    id: root

    required property var entryId

    spacing: 8

    LibraryEntry {
        id: libEntry
        entryId: root.entryId
    }

    AchievementSet {
        id: achievements
        setId: libEntry.achievementSetId
    }

    GameActivity {
        id: gameActivity
        contentHash: libEntry.contentHash
    }

    RowLayout {
        Layout.maximumHeight: 60
        Layout.minimumHeight: 60
        spacing: 12
        Image {
            Layout.preferredHeight: 60
            sourceSize.height: height
            source: libEntry.icon1x1SourceUrl
        }

        Text {
            text: libEntry.name
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "white"
            font.family: Constants.regularFontFamily
            font.weight: Font.Bold
            font.pixelSize: 36
            verticalAlignment: Qt.AlignVCenter
        }
    }

    FLTwoColumnMenu {
        Layout.fillWidth: true
        Layout.fillHeight: true

        menuItems: ["Achievements", "Activity"]
        pages: [thing, activity]
    }

    Component {
        id: activity
        ListView {
            model: gameActivity.playSessions
            delegate: RowLayout {
                id: item
                required property var model
                required property var index

                height: 32
                width: ListView.view.width

                spacing: 8

                Text {
                    text: {
                        let date = new Date(item.model.startTime)
                        return date.toLocaleString()
                    }
                    font.pixelSize: 17
                    font.weight: Font.Medium
                    font.family: Constants.regularFontFamily
                    color: "white"
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignLeft
                }

                Text {
                    text: {
                        let date = new Date(item.model.endTime)
                        return date.toLocaleString()
                    }
                    font.pixelSize: 17
                    font.weight: Font.Medium
                    font.family: Constants.regularFontFamily
                    color: "white"
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignLeft
                }

                Text {
                    text: model.numUnpausedSeconds
                    font.pixelSize: 17
                    font.weight: Font.Medium
                    font.family: Constants.regularFontFamily
                    color: "white"
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignLeft
                }


            }
        }
    }

    // RowLayout {
    //     Layout.fillWidth: true
    //     Layout.fillHeight: true
    //     ListView {
    //         Layout.fillHeight: true
    //         Layout.preferredWidth: 200
    //         model: ListModel {
    //             ListElement {
    //                 name: "Details"
    //             }
    //             ListElement {
    //                 name: "Achievements"
    //             }
    //             ListElement {
    //                 name: "Activity"
    //             }
    //             ListElement {
    //                 name: "Settings"
    //             }
    //         }
    //         delegate: Text {
    //             required property var model
    //             text: model.name
    //             color: "white"
    //             font.family: Constants.regularFontFamily
    //             font.weight: Font.Medium
    //             font.pixelSize: 24
    //             verticalAlignment: Qt.AlignVCenter
    //         }
    //     }

    //     Rectangle {
    //         color: "red"
    //         Layout.fillWidth: true
    //         Layout.fillHeight: true
    //     }
    // }

    Component {
        id: thing
        FocusScope {

            clip: true
            // Text {
            //     anchors.fill: parent
            //     text: "lol no achievements"
            //     // visible: achievements.numAchievements === 0
            //     font.pixelSize: 24
            //     font.weight: Font.Medium
            //     font.family: Constants.regularFontFamily
            //     color: "white"
            //     verticalAlignment: Qt.AlignVCenter
            //     horizontalAlignment: Qt.AlignHCenter
            // }
            ListView {
                id: theList
                // anchors.left: gameNavList.right
                anchors.fill: parent
                anchors.leftMargin: 4
                anchors.rightMargin: 4
                anchors.topMargin: 4
                anchors.bottomMargin: 4
                focus: true
                // visible: achievements.numAchievements > 0
                highlightMoveDuration: 80
                highlightMoveVelocity: -1
                highlightRangeMode: InputMethodManager.usingMouse ? ListView.NoHighlightRange : ListView.ApplyRange
                preferredHighlightBegin: 64
                preferredHighlightEnd: height - 64
                model: achievements.achievements

                ScrollBar.vertical: ScrollBar { }

                spacing: 0

                // Timer {
                //     id: wheelTimer
                //     interval: 500
                //     repeat: false
                // }
                //
                // WheelHandler {
                //     onWheel: function(event) {
                //         if (wheelTimer.running) {
                //             if (theList.currentIndex === 0 && event.angleDelta.y >= 0) {
                //                 wheelTimer.restart()
                //                 return
                //             } else if (theList.currentIndex === theList.count - 1 && event.angleDelta.y < 0) {
                //                 wheelTimer.restart()
                //                 return
                //             }
                //         }
                //
                //         if (event.angleDelta.y >= 0) {
                //             // theList.highlightRangeMode = ListView.ApplyRange
                //             theList.decrementCurrentIndex()
                //         } else {
                //             // theList.highlightRangeMode = ListView.ApplyRange
                //             theList.incrementCurrentIndex()
                //         }
                //
                //         wheelTimer.restart()
                //     }
                // }

                delegate: Button {
                    id: tttt
                    required property var model
                    required property var index
                    width: ListView.view.width

                    property bool showGlobalCursor: true

                    RightClickMenu {
                        id: rightClick

                        RightClickMenuItem {
                            text: "See on RetroAchievements.org"
                            externalLink: true
                            onTriggered: {
                                Qt.openUrlExternally("https://retroachievements.org/achievement/" + tttt.model.achievement_id)
                            }
                        }

                    }

                    TapHandler {
                        acceptedButtons: Qt.LeftButton | Qt.RightButton

                        onSingleTapped: function(event, button) {
                            if (button === 2) {
                                rightClick.popupNormal()
                            }
                            if (tttt.ListView.view.currentIndex !== tttt.index) {
                                // theList.highlightRangeMode = ListView.NoHighlightRange
                                tttt.ListView.view.currentIndex = tttt.index
                            }
                        }
                    }

                    HoverHandler {
                        id: hoverHandler
                        cursorShape: Qt.PointingHandCursor
                    }

                    height: 80
                    padding: 8
                    horizontalPadding: 12

                    background: Rectangle {
                        color: "white"
                        opacity: tttt.activeFocus ? 0.2 : (hoverHandler.hovered ? 0.1 : 0.0)
                        // color: "#333333"
                        //
                        // gradient: Gradient {
                        //     orientation: Gradient.Horizontal
                        //     GradientStop { position: 0.0; color: "#252525" }
                        //     GradientStop { position: 1.0; color: "#363636" }
                        // }
                        radius: 6
                        // opacity: 0.95
                        // visible: tttt.ListView.isCurrentItem
                    }

                    contentItem: RowLayout {
                        spacing: 16
                        Image {
                            source: tttt.model.image_url
                            Layout.preferredHeight: parent.height
                            Layout.preferredWidth: parent.height
                            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                            sourceSize.width: parent.height
                            sourceSize.height: parent.height
                            smooth: false
                            fillMode: Image.PreserveAspectFit
                        }
                        ColumnLayout {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            Layout.topMargin: 4
                            Layout.bottomMargin: 4
                            spacing: 0
                            Text {
                                text: tttt.model.name
                                font.pixelSize: 17
                                font.weight: Font.DemiBold
                                font.family: Constants.regularFontFamily
                                // color: tttt.ListView.isCurrentItem ? "black" : "white"
                                color: "white"
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                                maximumLineCount: 1
                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignVCenter
                            }
                            Text {
                                text: tttt.model.description
                                font.pixelSize: 15
                                font.weight: Font.Normal
                                font.family: Constants.regularFontFamily
                                // color: tttt.ListView.isCurrentItem ? "black" : "white"
                                color: "#dadada"
                                Layout.fillHeight: true
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                                maximumLineCount: 2
                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignVCenter
                                wrapMode: Text.WordWrap
                            }
                        }

                        Text {
                            text: tttt.model.earned ? tttt.model.earned_date : "Not earned"
                            font.pixelSize: 17
                            font.weight: Font.Medium
                            font.family: Constants.regularFontFamily
                            // color: tttt.ListView.isCurrentItem ? "black" : "white"
                            color: "white"
                            Layout.fillHeight: true
                            Layout.preferredWidth: 300
                            elide: Text.ElideRight
                            maximumLineCount: 1
                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                        }

                        Text {
                            text: tttt.model.points
                            font.pixelSize: 17
                            font.weight: Font.Medium
                            font.family: Constants.regularFontFamily
                            // color: tttt.ListView.isCurrentItem ? "black" : "white"
                            color: "white"
                            Layout.fillHeight: true
                            horizontalAlignment: Text.AlignRight
                            verticalAlignment: Text.AlignVCenter
                            Layout.preferredWidth: 100
                        }
                        Item {
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                        }
                    }
                }
            }
        }

    }

}
