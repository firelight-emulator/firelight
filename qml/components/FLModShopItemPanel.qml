import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

ColumnLayout {
    id: root

    required property var modId
    required property bool landscape
    // property bool landscape: parent.width > parent.height
    spacing: 8

    ModInfo {
        id: modInfo
        modId: root.modId
    }

    Text {
        Layout.fillWidth: true
        color: ColorPalette.neutral100
        font.family: Constants.regularFontFamily
        font.pixelSize: 36
        font.weight: Font.Black
        horizontalAlignment: Text.AlignLeft
        text: modInfo.modName
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.WordWrap
    }

    Text {
        Layout.fillWidth: true
        Layout.bottomMargin: 12
        color: ColorPalette.neutral300
        font.family: Constants.regularFontFamily
        font.pixelSize: 15
        font.weight: Font.Normal
        text: modInfo.platformName
        wrapMode: Text.WordWrap
    }

    ColumnLayout {
        visible: !root.landscape
        focus: visible
        Layout.fillWidth: true

        FLImageCarousel {
            Layout.fillHeight: true
            Layout.fillWidth: true
            imageUrls: modInfo.mediaUrls
        }
        Text {
            Layout.fillWidth: true
            Layout.topMargin: 16
            color: ColorPalette.neutral400
            font.family: Constants.regularFontFamily
            font.pixelSize: 15
            font.weight: Font.DemiBold
            leftPadding: 12
            text: "Author"
            wrapMode: Text.WordWrap
        }
        Pane {
            Layout.fillWidth: true

            background: Rectangle {
                color: ColorPalette.neutral800
                radius: 12
            }
            contentItem: Text {
                color: ColorPalette.neutral100
                font.family: Constants.regularFontFamily
                font.pixelSize: 16
                text: modInfo.authorName
                // font.weight: Font.DemiBold
                wrapMode: Text.WordWrap
            }
        }
        FirelightButton {
            Layout.alignment: Qt.AlignHCenter
            focus: true
            label: "Install"

            onClicked: {
                console.log("Install button clicked");
            }
        }
        Text {
            Layout.fillWidth: true
            bottomPadding: 24
            color: "white"
            font.family: Constants.regularFontFamily
            font.pixelSize: 16
            font.weight: Font.DemiBold
            lineHeight: 1.2
            text: modInfo.tagline
            topPadding: 24
            wrapMode: Text.WordWrap
        }
        Text {
            Layout.fillWidth: true
            color: ColorPalette.neutral400
            font.family: Constants.regularFontFamily
            font.pixelSize: 16
            font.weight: Font.DemiBold
            horizontalAlignment: Text.AlignLeft
            lineHeight: 1.2
            text: modInfo.description
            textFormat: Text.RichText
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.WordWrap
        }
    }

    RowLayout {
        visible: root.landscape
        focus: visible
        Layout.fillWidth: true

        ColumnLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true

            FLImageCarousel {
                id: thing

                Layout.fillHeight: true
                Layout.fillWidth: true
                imageUrls: modInfo.mediaUrls
            }
            Text {
                Layout.fillWidth: true
                bottomPadding: 24
                color: "white"
                font.family: Constants.regularFontFamily
                font.pixelSize: 16
                font.weight: Font.DemiBold
                lineHeight: 1.2
                text: modInfo.tagline
                topPadding: 24
                wrapMode: Text.WordWrap
            }
            Text {
                Layout.fillWidth: true
                color: ColorPalette.neutral400
                font.family: Constants.regularFontFamily
                font.pixelSize: 16
                font.weight: Font.DemiBold
                horizontalAlignment: Text.AlignLeft
                lineHeight: 1.2
                text: modInfo.description
                textFormat: Text.RichText
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WordWrap
            }
        }
        ColumnLayout {
            Layout.fillHeight: true
            Layout.leftMargin: 24
            Layout.maximumWidth: 350
            Layout.minimumWidth: 350
            focus: true
            Layout.preferredWidth: 350

            Image {
                Layout.fillWidth: true
                fillMode: Image.PreserveAspectFit
                source: modInfo.clearLogoUrl
            }
            FirelightButton {
                Layout.alignment: Qt.AlignHCenter
                label: "Install"
                focus: true

                onClicked: {
                    console.log("Install button clicked");
                }
            }
            Text {
                Layout.fillWidth: true
                Layout.topMargin: 16
                color: ColorPalette.neutral400
                font.family: Constants.regularFontFamily
                font.pixelSize: 15
                font.weight: Font.DemiBold
                leftPadding: 12
                text: "Author"
                wrapMode: Text.WordWrap
            }
            Pane {
                Layout.fillWidth: true

                background: Rectangle {
                    color: ColorPalette.neutral800
                    radius: 12
                }
                contentItem: Text {
                    color: ColorPalette.neutral100
                    font.family: Constants.regularFontFamily
                    font.pixelSize: 16
                    text: modInfo.authorName
                    // font.weight: Font.DemiBold
                    wrapMode: Text.WordWrap
                }
            }
            Item {
                Layout.fillHeight: true
                Layout.fillWidth: true
            }
        }
    }
    Pane {
        Layout.fillWidth: true
        Layout.preferredHeight: 48
        Layout.topMargin: 24
        Layout.bottomMargin: 24

        background: Item {
        }
        contentItem: Text {
            color: ColorPalette.neutral400
            font.family: Constants.regularFontFamily
            font.pixelSize: 14
            font.weight: Font.DemiBold
            horizontalAlignment: Text.AlignHCenter
            text: "Trademarks, tradenames, and copyrights are property of their respective owners."
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.WordWrap
        }
    }
}
