import QtQuick
import QtQuick.Controls
import QtQuick.Layouts 1.0
import Firelight 1.0

FocusScope {
    id: control

    required property int gameId
    required property bool achievementsEnabled
    required property bool loggedIn

    required property var achievementsModel

    ListView {
        visible: control.loggedIn
        objectName: "Achievements List View"
        model: control.achievementsModel
        spacing: 12
        anchors.fill: parent
        focus: true

        header: Pane {
            width: parent.width
            background: Item {}
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
                    font.pixelSize: AppStyle.fontSizeSmall
                }

                MyComboBox {
                    id: sortBox
                    Layout.fillHeight: true
                    Layout.fillWidth: false
                    textRole: "text"
                    valueRole: "value"

                    model: [
                        {
                            text: "Default",
                            value: "default"
                        },
                        {
                            text: "A-Z",
                            value: "title"
                        },
                        {
                            text: "Earned date",
                            value: "earned_date"
                        },
                        {
                            text: "Points",
                            value: "points"
                        }
                    ]

                    // onActivated: function () {
                    //     root.achievements.sortType = currentValue
                    // }
                }
            }
        }

        delegate: AchievementListItem {
            objectName: "Achievement List Item (" + model.achievement_id + ")"
            width: ListView.view.width
        }
    }

    ColumnLayout {
        id: loginColumn
        objectName: "Log in column"
        visible: !control.loggedIn
        anchors.fill: parent
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
        Text {
            text: "Log in blah blah"
            color: "white"
            font.family: Constants.regularFontFamily
            font.pixelSize: AppStyle.fontSizeSmall
            Layout.alignment: Qt.AlignCenter
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
        Button {
            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: 140
            Layout.preferredHeight: 50
            focus: true
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
                font.pixelSize: AppStyle.fontSizeMedium
            }
            onClicked: {
                Router.navigate("/settings/achievements");
            }
        }
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
