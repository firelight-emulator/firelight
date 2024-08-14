import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

FocusScope {
    id: root

    implicitHeight: imageContainer.height + previewList.height + 12

    Rectangle {
        id: imageContainer
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: parent.width * 9 / 16
        radius: 8
        color: "black"

        Image {
            mipmap: true
            height: parent.height
            anchors.horizontalCenter: parent.horizontalCenter
            source: previewList.model.get(previewList.currentIndex).source
            fillMode: Image.PreserveAspectFit
        }
    }

    ListView {
        id: previewList
        // Layout.leftMargin: 40
        spacing: 8
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: 12
        anchors.top: imageContainer.bottom
        height: 60

        clip: true
        orientation: ListView.Horizontal
        // Layout.fillWidth: true
        // Layout.preferredHeight: 80
        // model: 10
        delegate: Rectangle {
            required property var model
            required property var index
            width: ListView.view.height * 16 / 9
            height: ListView.view.height
            color: "black"
            radius: 4
            TapHandler {
                onTapped: {
                    previewList.currentIndex = index
                }
            }
            Image {
                mipmap: true
                height: parent.height
                asynchronous: true
                anchors.horizontalCenter: parent.horizontalCenter
                source: model.source
                smooth: false
                fillMode: Image.PreserveAspectFit
            }
        }
        model: ListModel {
            ListElement {
                source: "file:system/_img/cursedmirror1.png"
            }
            ListElement {
                source: "file:system/_img/cursedmirror2.png"
            }
            ListElement {
                source: "file:system/_img/cursedmirror3.png"
            }
            ListElement {
                source: "file:system/_img/cursedmirror4.png"
            }
            ListElement {
                source: "file:system/_img/cursedmirror5.png"
            }
            ListElement {
                source: "file:system/_img/cursedmirror6.png"
            }
        }
        // delegate: Image {
        //     required property var model
        //     height: 70
        //     width: 93
        //     mipmap: true
        //     source: model.source
        //     fillMode: Image.PreserveAspectFit
        // }
    }
}