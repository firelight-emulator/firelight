import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import QtCharts 2.7

Item {
    id: root

    signal playPressed()

    required property int entryId
    property int tabWidth: 150
    property var achievements: null
    property var achievementsSummary: {
        "NumPossibleAchievements": 0,
        "PossibleScore": 0,
        "NumAchieved": 0,
        "ScoreAchieved": 0,
        "NumAchievedHardcore": 0,
        "ScoreAchievedHardcore": 0

    }
    property var entryData: {}

    Component.onCompleted: {
        entryData = library_database.getLibraryEntryJson(entryId)

        root.achievements = achievement_manager.getAchievementsModelForGameId(entryData.game_id)
        achievement_manager.getAchievementsOverview(entryData.game_id)
    }

    Connections {
        target: achievement_manager

        function onAchievementSummaryAvailable(json) {
            root.achievementsSummary = json
        }

        // function onAchievementListAvailable(model) {
        //     root.achievements = model
        // }
    }

    ColumnLayout {
        spacing: 0
        anchors.fill: parent

        RowLayout {
            id: topRow
            Layout.fillWidth: true
            Layout.topMargin: 12
            Layout.minimumHeight: 84
            Layout.maximumHeight: 84
            spacing: 0

            Text {
                Layout.alignment: Qt.AlignTop
                Layout.leftMargin: 48
                padding: 10
                text: root.entryData.display_name
                color: "white"
                font.pointSize: 22
                font.family: Constants.regularFontFamily
                font.weight: Font.DemiBold
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            Button {
                Layout.alignment: Qt.AlignRight | Qt.AlignTop
                Layout.preferredWidth: 140
                Layout.preferredHeight: 50
                Layout.rightMargin: 48
                background: Rectangle {
                    color: parent.hovered ? "#b8b8b8" : "white"
                    radius: 4
                }
                hoverEnabled: true
                contentItem: Text {
                    text: qsTr("Play")
                    color: Constants.colorTestBackground
                    font.family: Constants.regularFontFamily
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pointSize: 12
                }
                onClicked: {
                    root.playPressed()
                }
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true

            spacing: 0

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.horizontalStretchFactor: 1
            }

            ColumnLayout {
                id: theColumn
                Layout.fillWidth: true
                Layout.horizontalStretchFactor: 4
                Layout.minimumWidth: 700
                Layout.maximumWidth: 1200
                Layout.preferredWidth: parent.width * 3 / 4
                Layout.fillHeight: true

                TabBar {
                    id: bar
                    // Layout.topMargin: 16
                    Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
                    Layout.preferredHeight: 40
                    currentIndex: -1

                    onCurrentIndexChanged: function () {
                        view.setCurrentIndex(currentIndex)
                    }

                    background: Rectangle {
                        width: root.tabWidth
                        visible: bar.contentChildren[bar.currentIndex].enabled
                        height: 2
                        radius: 1
                        color: "white"
                        x: root.tabWidth * bar.currentIndex
                        y: bar.height

                        Behavior on x {
                            NumberAnimation {
                                duration: 120
                                easing.type: Easing.InOutQuad
                            }
                        }
                    }

                    TabButton {
                        width: root.tabWidth
                        contentItem: Text {
                            text: "Details"
                            color: parent.enabled ? "#ffffff" : "#666666"
                            font.family: Constants.regularFontFamily
                            font.pointSize: 11
                            font.weight: parent.enabled && parent.checked ? Font.Bold : Font.Normal
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        background: Item {
                        }

                        HoverHandler {
                            cursorShape: Qt.PointingHandCursor
                        }
                    }
                    TabButton {
                        width: root.tabWidth
                        contentItem: Text {
                            text: "Achievements"
                            color: parent.enabled ? "#ffffff" : "#666666"
                            font.family: Constants.regularFontFamily
                            font.pointSize: 11
                            font.weight: parent.enabled && parent.checked ? Font.Bold : Font.Normal
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        background: Item {
                        }

                        HoverHandler {
                            cursorShape: Qt.PointingHandCursor
                        }
                    }
                    TabButton {
                        width: root.tabWidth
                        contentItem: Text {
                            text: "Activity"
                            color: parent.enabled ? "#ffffff" : "#666666"
                            font.family: Constants.regularFontFamily
                            font.pointSize: 11
                            font.weight: parent.enabled && parent.checked ? Font.Bold : Font.Normal
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        background: Item {
                        }

                        HoverHandler {
                            cursorShape: parent.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                        }
                    }
                    TabButton {
                        width: root.tabWidth
                        contentItem: Text {
                            text: "Settings"
                            color: parent.enabled ? "#ffffff" : "#666666"
                            font.family: Constants.regularFontFamily
                            font.pointSize: 11
                            font.weight: parent.enabled && parent.checked ? Font.Bold : Font.Normal
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        background: Item {
                        }

                        HoverHandler {
                            cursorShape: parent.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                        }
                    }
                }
                SwipeView {
                    id: view

                    objectName: "Content Swipe View"
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    currentIndex: 0

                    clip: true

                    onCurrentIndexChanged: function () {
                        bar.setCurrentIndex(currentIndex)
                    }

                    // ChartView {
                    //     title: "Line Chart"
                    //     theme: ChartView.ChartThemeBrownSand
                    //     antialiasing: true
                    //
                    //     BarCategoryAxis {
                    //         id: daysAxis
                    //         categories: ["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"]
                    //     }
                    //
                    //     ValueAxis {
                    //         id: playtimeAxis
                    //         min: 0
                    //         max: 10
                    //         titleText: "Playtime (hours)"
                    //     }
                    //
                    //     BarSeries {
                    //         id: barSeries
                    //         axisX: daysAxis
                    //         axisY: playtimeAxis
                    //
                    //         BarSet {
                    //             label: "Playtime"
                    //             // Sample data
                    //             values: [3, 5, 2, 7, 6, 1, 4]
                    //         }
                    //     }
                    //
                    //     Component.onCompleted: {
                    //         barSeries.append("Playtime", [3, 5, 2, 7, 6, 1, 4])
                    //     }
                    // }
                    ColumnLayout {
                        objectName: "Details Tab Content"
                        Text {
                            Layout.fillWidth: true
                            text: qsTr("Content path")
                            color: "white"
                            font.pointSize: 12
                            font.family: Constants.regularFontFamily
                            font.weight: Font.DemiBold
                        }
                        Pane {
                            id: texxxxt
                            // Layout.fillWidth: true
                            padding: 4

                            background: Rectangle {
                                color: "black"
                                radius: 8
                            }

                            contentItem: TextInput {
                                padding: 4
                                text: root.entryData.content_path
                                font.family: Constants.regularFontFamily
                                font.pointSize: 12
                                color: "white"
                                verticalAlignment: Text.AlignVCenter
                                readOnly: true
                            }
                        }
                        Text {
                            text: "Active save slot: " + root.entryData.active_save_slot
                            color: "white"
                            font.family: Constants.regularFontFamily
                            font.pointSize: 10
                        }
                        Text {
                            text: "Added to library: " + root.entryData.created_at
                            color: "white"
                            font.family: Constants.regularFontFamily
                            font.pointSize: 10
                        }
                        Item {
                            Layout.fillHeight: true
                        }
                    }
                    Flickable {
                        id: flickThing
                        objectName: "Achievements Tab Content"
                        contentHeight: !achievement_manager.loggedIn ? thing.height : achievementsList.height

                        property real scrollMultiplier: 8.0  // Adjust this multiplier for desired scroll speed
                        property real maxScrollSpeed: 1000  // Maximum scroll speed
                        property real smoothScrollSpeed: 0.1 // Adjust this for smoothness

                        // Behavior on contentY {
                        //     NumberAnimation {
                        //         duration: 120
                        //         easing.type: Easing.InOutQuad
                        //     }
                        // }


                        MouseArea {
                            objectName: "Wheel Mouse Handler"
                            anchors.fill: parent
                            acceptedButtons: Qt.NoButton
                            // preventStealing: true
                            onWheel: function (wheel) {
                                var scrollDelta;
                                if (wheel.pixelDelta.y !== 0) {
                                    scrollDelta = wheel.pixelDelta.y;
                                } else {
                                    scrollDelta = wheel.angleDelta.y / 8;
                                }

                                flickThing.contentY -= scrollDelta * flickThing.scrollMultiplier
                                if (flickThing.contentY < 0) {
                                    flickThing.contentY = 0
                                } else if (flickThing.contentY > flickThing.contentHeight - flickThing.height) {
                                    flickThing.contentY = flickThing.contentHeight - flickThing.height
                                }

                                //
                                // // Dynamic scaling factor for fast scrolling
                                // var scaledDelta = scrollDelta * flickThing.scrollMultiplier;
                                // var absDelta = Math.abs(scrollDelta);
                                //
                                // // Increase scroll speed for larger deltas (fast scrolling)
                                // if (absDelta > 10) {
                                //     scaledDelta *= (absDelta / 10);
                                //     scaledDelta = Math.sign(scrollDelta) * Math.min(flickThing.maxScrollSpeed, Math.abs(scaledDelta));
                                // }
                                //
                                // flickThing.contentY = Math.max(0, Math.min(flickThing.contentY + scaledDelta, flickThing.contentHeight - flickThing.height));
                            }
                        }
                        ColumnLayout {
                            id: thing
                            objectName: "Log in column"
                            visible: !achievement_manager.loggedIn
                            Item {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                            }
                            Text {
                                text: "Log in blah blah"
                                color: "white"
                                font.family: Constants.regularFontFamily
                                font.pointSize: 10
                                Layout.alignment: Qt.AlignCenter
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            Button {
                                Layout.alignment: Qt.AlignCenter
                                Layout.preferredWidth: 140
                                Layout.preferredHeight: 50
                                background: Rectangle {
                                    color: parent.hovered ? "#b8b8b8" : "white"
                                    radius: 4
                                }
                                hoverEnabled: true
                                contentItem: Text {
                                    text: qsTr("Log in")
                                    color: Constants.colorTestBackground
                                    font.family: Constants.regularFontFamily
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                    font.pointSize: 12
                                }
                                onClicked: {
                                    Router.navigateTo("settings/achievements")
                                }
                            }
                            Item {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                            }
                        }

                        Column {
                            id: achievementsList

                            objectName: "Achievements List"
                            visible: achievement_manager.loggedIn && root.achievementsSummary.NumPossibleAchievements > 0
                            width: parent.width
                            spacing: 24

                            Item {
                                objectName: "Spacer"
                                width: parent.width
                                height: 24
                            }

                            // Pane {
                            //     width: 540
                            //     height: 106
                            //     padding: 8
                            //
                            //     background: Rectangle {
                            //         color: "#292929"
                            //         radius: 8
                            //     }
                            //
                            //     contentItem: Item {
                            //         Image {
                            //             id: gameIcon
                            //             width: parent.height
                            //             height: parent.height
                            //             fillMode: Image.PreserveAspectFit
                            //             source: "https://media.retroachievements.org/Images/040155.png"
                            //         }
                            //         // Rectangle {
                            //         //     id: gameIcon
                            //         //     color: "red"
                            //         //     width: parent.height
                            //         //     height: parent.height
                            //         // }
                            //         ColumnLayout {
                            //             anchors.leftMargin: 12
                            //             anchors.left: gameIcon.right
                            //             anchors.top: parent.top
                            //             anchors.bottom: parent.bottom
                            //             anchors.right: parent.right
                            //             spacing: 4
                            //
                            //             RowLayout {
                            //                 Layout.fillWidth: true
                            //                 Layout.fillHeight: true
                            //                 Layout.verticalStretchFactor: 3
                            //                 spacing: 0
                            //                 Text {
                            //                     Layout.fillHeight: true
                            //                     text: root.entryData.display_name
                            //                     font.family: Constants.regularFontFamily
                            //                     font.pointSize: 12
                            //                     font.weight: Font.DemiBold
                            //                     verticalAlignment: Text.AlignVCenter
                            //                     color: "white"
                            //                 }
                            //                 Item {
                            //                     Layout.fillWidth: true
                            //                     Layout.fillHeight: true
                            //                 }
                            //                 Text {
                            //                     id: first
                            //                     text: root.achievementsSummary.NumAchievedHardcore
                            //                     font.family: Constants.regularFontFamily
                            //                     font.pointSize: 16
                            //                     font.weight: Font.DemiBold
                            //                     color: "white"
                            //                     verticalAlignment: Text.AlignBottom
                            //                     horizontalAlignment: Text.AlignRight
                            //                     Layout.fillHeight: true
                            //                 }
                            //
                            //                 Text {
                            //                     id: slash
                            //                     text: " /"
                            //                     Layout.fillHeight: true
                            //                     font.family: Constants.regularFontFamily
                            //                     font.pointSize: 15
                            //                     color: "#aaaaaa"
                            //                     verticalAlignment: Text.AlignBottom
                            //                 }
                            //
                            //                 Text {
                            //                     text: root.achievementsSummary.NumPossibleAchievements
                            //                     Layout.fillHeight: true
                            //                     font.family: Constants.regularFontFamily
                            //                     font.pointSize: 12
                            //                     color: "#aaaaaa"
                            //                     verticalAlignment: Text.AlignBottom
                            //                 }
                            //
                            //                 Text {
                            //                     text: " earned"
                            //                     Layout.fillHeight: true
                            //                     font.family: Constants.regularFontFamily
                            //                     font.pointSize: 10
                            //                     color: "#aaaaaa"
                            //                     verticalAlignment: Text.AlignBottom
                            //                 }
                            //             }
                            //
                            //             Item {
                            //                 Layout.fillHeight: true
                            //                 Layout.fillWidth: true
                            //                 Layout.verticalStretchFactor: 1
                            //                 Rectangle {
                            //                     anchors.verticalCenter: parent.verticalCenter
                            //                     width: parent.width
                            //                     height: 20
                            //                     radius: height / 2
                            //                     color: "black"
                            //                 }
                            //
                            //                 Rectangle {
                            //                     anchors.verticalCenter: parent.verticalCenter
                            //                     width: parent.width / 5
                            //                     height: 20
                            //                     radius: height / 2
                            //                     border.color: "black"
                            //                     color: "#bc8c0f"
                            //                 }
                            //
                            //             }
                            //         }
                            //     }
                            // }

                            // Rectangle {
                            //     Rectangle {
                            //         id: gameIcon
                            //         width: 80
                            //         height: 80
                            //         anchors.left: parent.left
                            //         anchors.top: parent.top
                            //         anchors.leftMargin: 12
                            //         anchors.topMargin: 12
                            //         color: "red"
                            //     }
                            //     Text {
                            //         text: root.entryData.display_name
                            //         anchors.left: gameIcon.right
                            //         anchors.top: parent.top
                            //         anchors.leftMargin: 12
                            //         anchors.topMargin: 12
                            //         color: "white"
                            //         font.family: Constants.regularFontFamily
                            //         font.pointSize: 12
                            //         font.weight: Font.DemiBold
                            //     }
                            //
                            //     RowLayout {
                            //         anchors.left: parent.left
                            //         anchors.bottom: parent.bottom
                            //         anchors.right: parent.right
                            //         anchors.top: gameIcon.bottom
                            //         anchors.topMargin: 12
                            //         anchors.leftMargin: 12
                            //         anchors.bottomMargin: 12
                            //         spacing: 0
                            //
                            //         Text {
                            //             id: first
                            //             text: "300"
                            //             font.family: Constants.regularFontFamily
                            //             font.pointSize: 16
                            //             font.weight: Font.DemiBold
                            //             color: "white"
                            //             verticalAlignment: Text.AlignBottom
                            //             horizontalAlignment: Text.AlignRight
                            //             Layout.fillHeight: true
                            //         }
                            //
                            //         Text {
                            //             id: slash
                            //             text: " /"
                            //             Layout.fillHeight: true
                            //             font.family: Constants.regularFontFamily
                            //             font.pointSize: 15
                            //             color: "#aaaaaa"
                            //             verticalAlignment: Text.AlignBottom
                            //         }
                            //
                            //         Text {
                            //             text: "200"
                            //             Layout.fillHeight: true
                            //             font.family: Constants.regularFontFamily
                            //             font.pointSize: 12
                            //             color: "#aaaaaa"
                            //             verticalAlignment: Text.AlignBottom
                            //         }
                            //
                            //         Rectangle {
                            //             Layout.fillWidth: true
                            //             Layout.preferredHeight: 20
                            //             Layout.alignment: Qt.AlignCenter
                            //             radius: height / 2
                            //         }
                            //     }
                            //
                            //     // Text {
                            //     //     text: root.achievementsSummary.NumAchievedHardcore
                            //     //     color: "white"
                            //     //     font.family: Constants.regularFontFamily
                            //     //     font.pointSize: 16
                            //     //     font.weight: Font.DemiBold
                            //     //     horizontalAlignment: Text.AlignHCenter
                            //     //     verticalAlignment: Text.AlignVCenter
                            //     // }
                            // }

                            ListView {

                                objectName: "Achievements List View"
                                model: root.achievements
                                spacing: 12
                                width: parent.width
                                height: contentHeight
                                interactive: false

                                // section.property: root.achievements.sortType === "title" ? "name" : "earned"
                                // section.criteria: ViewSection.FirstCharacter
                                // section.delegate: ListViewSectionDelegate {
                                //     required property string section
                                //     text: section === "t" || section === "f" ? (section === "t" ? "Earned" : "Not earned") : section
                                // }

                                header: Pane {
                                    width: achievementsList.width
                                    background: Item {
                                    }
                                    verticalPadding: 12
                                    horizontalPadding: 0
                                    contentItem: RowLayout {
                                        Item {
                                            Layout.fillWidth: true
                                            Layout.fillHeight: true
                                        }
                                        Text {
                                            Layout.fillHeight: true
                                            verticalAlignment: Text.AlignVCenter
                                            text: "Sort by:"
                                            color: "white"
                                            font.family: Constants.regularFontFamily
                                            font.pointSize: 10
                                        }

                                        MyComboBox {
                                            id: sortBox
                                            Layout.fillHeight: true
                                            Layout.fillWidth: false
                                            textRole: "text"
                                            valueRole: "value"

                                            model: [
                                                {text: "Default", value: "default"},
                                                {text: "A-Z", value: "title"},
                                                {text: "Earned date", value: "earned_date"},
                                                {text: "Points", value: "points"}
                                            ]

                                            Connections {
                                                target: root

                                                function onAchievementsChanged() {
                                                    console.log("current sort type: " + root.achievements.sortType)
                                                    sortBox.currentIndex = sortBox.indexOfValue(root.achievements.sortType)
                                                }
                                            }

                                            onActivated: function () {
                                                root.achievements.sortType = currentValue
                                            }
                                        }
                                    }
                                }

                                delegate: AchievementListItem {
                                    objectName: "Achievement List Item (" + model.achievement_id + ")"
                                    width: ListView.view.width
                                }
                            }
                        }


                    }

                    Text {
                        text: "activity stuff"
                        color: "white"
                        font.family: Constants.regularFontFamily
                        font.pointSize: 10
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    Text {
                        text: "settings stuff"
                        color: "white"
                        font.family: Constants.regularFontFamily
                        font.pointSize: 10
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.horizontalStretchFactor: 1
            }
        }
    }
}