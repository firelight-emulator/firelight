import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ComboBox {
    id: control
    implicitContentWidthPolicy: ComboBox.WidestText

    indicator: Text {
        x: control.mirrored ? control.padding : control.width - width - control.padding
        y: control.topPadding + (control.availableHeight - height) / 2
        text: "\ue5c5"
        font.family: Constants.symbolFontFamily
        font.pixelSize: 32
        padding: 4
        color: "#ececec"
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
    }

    contentItem: TextInput {
        topPadding: 8
        bottomPadding: 8
        leftPadding: 10
        rightPadding: 10
        text: control.currentText
        color: "#ececec"
        font.family: Constants.regularFontFamily
        verticalAlignment: Text.AlignVCenter
        font.pointSize: 11
        font.weight: Font.DemiBold

        readOnly: true

        MouseArea {
            anchors.fill: parent
            onClicked: {
                control.forceActiveFocus(Qt.MouseFocusReason)
                control.popup.visible = !control.popup.visible
            }
        }
    }

    background: Rectangle {
        // implicitWidth: 140
        implicitHeight: 32
        color: ColorPalette.neutral300
        radius: 6
        opacity: 0.05
    }

    delegate: ItemDelegate {
        id: delegate

        required property var model
        required property int index

        background: Rectangle {
            color: control.highlightedIndex === index ? ColorPalette.neutral800 : ColorPalette.neutral900
        }

        width: control.width - 4
        contentItem: RowLayout {
            Text {
                Layout.fillWidth: true
                Layout.fillHeight: true
                text: delegate.model[control.textRole]
                font.family: Constants.regularFontFamily
                font.pointSize: 11
                color: control.currentIndex === index ? ColorPalette.neutral100 : ColorPalette.neutral300
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
            }

            Text {
                text: "\ue86c"
                font.family: Constants.symbolFontFamily
                font.pixelSize: 18
                color: "#17b317"
                visible: control.currentIndex === index
                Layout.alignment: Qt.AlignCenter
            }
        }
        highlighted: control.highlightedIndex === index
    }

    popup: Popup {
        y: control.height - 1
        width: control.width
        implicitHeight: contentItem.implicitHeight + padding * 2
        padding: 4


        contentItem: ListView {
            id: theList
            property real scrollMultiplier: 4.0
            // property int maxHeight: delegate ? 8 * (delegate.height + 1) : 0
            property int maxHeight: 398

            boundsBehavior: Flickable.StopAtBounds
            clip: true
            implicitHeight: contentHeight > maxHeight ? maxHeight : contentHeight
            model: control.popup.visible ? control.delegateModel : null

            highlightFollowsCurrentItem: false

            highlight: Item {
            }


            // MouseArea {
            //     anchors.fill: parent
            //     acceptedButtons: Qt.NoButton
            //     // preventStealing: true
            //     onWheel: function (wheel) {
            //
            //         const numPixels = wheel.pixelDelta.y
            //         const numDegrees = wheel.angleDelta.y
            //
            //         if (numPixels !== 0) {
            //             theList.contentY -= numPixels
            //             return
            //         }
            //
            //         var scrollDelta = wheel.angleDelta.y / 8 * theList.scrollMultiplier;
            //
            //         theList.contentY -= scrollDelta
            //         if (theList.contentY < 0) {
            //             theList.contentY = 0
            //         } else if (theList.contentY > theList.contentHeight - theList.height) {
            //             theList.contentY = theList.contentHeight - theList.height
            //         }
            //     }
            // }

            ScrollIndicator.vertical: ScrollIndicator {
            }
        }

        background: Rectangle {
            color: ColorPalette.neutral900
            radius: 2
            border.color: ColorPalette.neutral700
        }
    }

    palette.buttonText: "white"
}