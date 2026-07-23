import QtQuick

// TODO
// Box art (or a platform-icon fallback) with a title scrim that reveals on
// hover/focus, all clipped to one set of rounded corners
Item {
    id: root

    property url source: ""
    property int size: 140
    property real radius: AppStyle.radiusLg
    property string title: ""
    property bool titleVisible: false
    required property int platformId

    implicitWidth: size
    implicitHeight: size

    Item {
        id: rounded
        anchors.fill: parent

        layer.enabled: root.radius > 0
        layer.smooth: true
        layer.effect: FLRoundedMask {
            radius: root.radius
        }

        Rectangle {
            anchors.fill: parent
            visible: root.source === ""
            color: Theme.surfaceElevated

            FLPlatformIcon {
                anchors.fill: parent
                anchors.margins: AppStyle.spacingLg * 1.5
                platformId: root.platformId
            }
        }

        FLRoundedImage {
            anchors.fill: parent
            visible: root.source !== ""
            source: root.source
            radius: 0
            fillMode: Image.PreserveAspectCrop
            background: Theme.surfaceElevated
        }

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: Math.round(parent.height * 0.55)
            opacity: root.titleVisible ? 1 : 0
            gradient: Gradient {
                GradientStop {
                    position: 0.0
                    color: "transparent"
                }
                GradientStop {
                    position: 1.0
                    color: "#dd000000"
                }
            }
            Behavior on opacity {
                NumberAnimation {
                    duration: AppStyle.durationFast
                    easing.type: Easing.OutQuad
                }
            }

            Text {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: AppStyle.spacingSm
                text: root.title
                color: "white"
                font.family: AppStyle.fontFamily
                font.pixelSize: AppStyle.fontSizeSmall
                font.weight: Font.DemiBold
                elide: Text.ElideRight
                maximumLineCount: 2
                wrapMode: Text.WordWrap
            }
        }
    }
}
