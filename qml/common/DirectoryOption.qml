import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.platform

Button {
    id: root

    property string label
    property string description: ""
    property string value: ""
    property bool showGlobalCursor: true
    checkable: true
    horizontalPadding: 12
    verticalPadding: 12

    onValueChanged: function () {
        let folder = root.value
        if (folder.startsWith("file://")) {
            folderDialog.currentFolder = folder
            return
        }

        folder = "file://" + folder
        folderDialog.currentFolder = folder
    }

    // onClicked: {
    //     if (root.checked) {
    //         sfx_player.play("switchon")
    //     } else {
    //         sfx_player.play("switchoff")
    //     }
    // }

    FolderDialog {
        id: folderDialog
        onAccepted: {
            root.value = folder
            // console.log("found it")
        }
    }

    onClicked: function () {
        folderDialog.open()
    }

    // implicitHeight: Math.max(72, theColumn.)
    hoverEnabled: true

    HoverHandler {
        cursorShape: Qt.PointingHandCursor
    }

    background: Rectangle {
        color: "white"
        radius: 2
        opacity: root.hovered ? 0.1 : 0
    }

    contentItem: RowLayout {
        Text {
            id: labelText
            Layout.fillWidth: true
            Layout.preferredHeight: 44
            text: root.label
            color: ColorPalette.neutral100
            font.pixelSize: 16
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            verticalAlignment: Text.AlignVCenter
            font.family: Constants.regularFontFamily
            font.weight: Font.DemiBold
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        Text {
            text: root.value.startsWith("file://") ? root.value.replace("file://", "") : root.value
            font.pixelSize: 16
            Layout.alignment: Qt.AlignRight
            font.family: Constants.regularFontFamily
            // font.weight: Font.DemiBold
            wrapMode: Text.WordWrap
            Layout.leftMargin: 32
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignVCenter
            color: ColorPalette.neutral400
        }

        // Switch {
        //     id: theControl
        //     Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
        //
        //     checked: root.checked
        //
        //     onCheckedChanged: {
        //         root.checked = theControl.checked
        //     }
        //
        //     indicator: Rectangle {
        //         implicitWidth: 50
        //         implicitHeight: 28
        //         x: theControl.leftPadding
        //         y: parent.height / 2 - height / 2
        //         radius: height / 2
        //         color: theControl.checked ? "#17a81a" : "#ffffff"
        //         border.color: theControl.checked ? "#17a81a" : "#cccccc"
        //
        //         Behavior on color {
        //             ColorAnimation {
        //                 duration: 200
        //                 easing.type: Easing.InOutQuad
        //             }
        //         }
        //
        //         Rectangle {
        //             x: theControl.checked ? parent.width - width : 0
        //             y: (parent.height - height) / 2
        //
        //             Behavior on x {
        //                 NumberAnimation {
        //                     duration: 200
        //                     easing.type: Easing.InOutQuad
        //                 }
        //             }
        //
        //             width: 26
        //             height: 26
        //             radius: height / 2
        //             color: theControl.down ? "#cccccc" : "#ffffff"
        //             border.color: theControl.checked ? (theControl.down ? "#17a81a" : "#21be2b") : "#999999"
        //         }
        //     }
        //
        //     HoverHandler {
        //         cursorShape: Qt.PointingHandCursor
        //     }
        // }
    }
}
