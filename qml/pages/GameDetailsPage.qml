import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import QtCharts 2.7

FocusScope {
    id: root

    signal playPressed()

    required property int entryId
    property var achievements: null
    property var entryData: {}

    Component.onCompleted: {
        entryData = library_database.getLibraryEntryJson(entryId)

        root.achievements = achievement_manager.getAchievementsModelForGameId(entryData.game_id)
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

                NavigationTabBar {
                    id: bar
                    tabs: ["Details", "Achievements", "Activity", "Settings"]
                    tabWidth: 150
                    Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
                    Layout.preferredHeight: 40

                    onCurrentIndexChanged: function () {
                        view.setCurrentIndex(currentIndex)
                    }
                }

                SwipeView {
                    id: view

                    objectName: "Content Swipe View"
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    focus: true

                    currentIndex: 0

                    clip: true

                    onCurrentIndexChanged: function () {
                        bar.setCurrentIndex(currentIndex)
                    }

                    EntrySummaryPage {
                        entryData: root.entryData
                    }

                    AchievementList {
                        gameId: 0
                        achievementsEnabled: achievement_manager.loggedIn
                        loggedIn: achievement_manager.loggedIn
                        achievementsModel: root.achievements
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