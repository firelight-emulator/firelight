import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FirelightStyle 1.0

ColumnLayout {
    id: root
    spacing: 0

    Image {
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
        source: "file:system/_img/RA_Logo10.png"
        fillMode: Image.PreserveAspectFit
    }

    ColumnLayout {
        Layout.fillWidth: true

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 48
            Text {
                anchors.left: parent.left
                visible: !achievement_manager.loggedIn
                text: qsTr("Not logged in to RetroAchievements")
                font.pointSize: 12
                font.family: Constants.regularFontFamily
                color: "white"
            }

            Text {
                anchors.left: parent.left
                visible: achievement_manager.loggedIn
                text: qsTr("Logged in as %1").arg(achievement_manager.displayName)
                font.pointSize: 12
                font.family: Constants.regularFontFamily
                color: "white"
            }
        }


        TextField {
            id: user
            visible: !achievement_manager.loggedIn
            Layout.fillWidth: true
            placeholderText: qsTr("Username")
        }
        TextField {
            id: pass
            visible: !achievement_manager.loggedIn
            Layout.fillWidth: true
            placeholderText: qsTr("Password")
            echoMode: TextInput.Password
        }

        Button {
            Layout.alignment: Qt.AlignRight
            visible: !achievement_manager.loggedIn
            text: qsTr("Login")
            onClicked: {
                achievement_manager.logInUserWithPassword(user.text, pass.text)
            }
        }

        Button {
            Layout.alignment: Qt.AlignRight
            visible: achievement_manager.loggedIn
            text: qsTr("Log out")
            onClicked: {
                achievement_manager.logout()
            }
        }
    }

    Item {
        Layout.fillWidth: true
        Layout.fillHeight: true

    }
}