import QtQuick
import QtQuick.Controls
import QtMultimedia
import Firelight 1.0

Rectangle {
    id: activeFocusHighlight

    required property Item target
    property real borderWidth: 2
    property color borderColor: "lightblue"
    property real bounceAmplitude: 16
    property MediaPlayer bumpSfx: undefined

    anchors.fill: parent
    anchors.margins: -4
    border.color: "lightblue"
    border.width: 2
    color: "transparent"
    radius: 2
    visible: !InputMethodManager.usingMouse && parent !== null

    // SequentialAnimation {
    //     id: highlightBounceAnimationRight
    //
    //     running: false
    //
    //     ParallelAnimation {
    //         ScriptAction {
    //             script: {
    //                 sfx_player.play("uibump");
    //             }
    //         }
    //         PropertyAnimation {
    //             duration: 40
    //             easing.type: Easing.Linear
    //             from: -4
    //             property: "anchors.leftMargin"
    //             target: activeFocusHighlight
    //             to: -4 + activeFocusHighlight.bounceAmplitude
    //         }
    //         PropertyAnimation {
    //             duration: 40
    //             easing.type: Easing.Linear
    //             from: -4
    //             property: "anchors.rightMargin"
    //             target: activeFocusHighlight
    //             to: -4 - activeFocusHighlight.bounceAmplitude
    //         }
    //     }
    //     ParallelAnimation {
    //         PropertyAnimation {
    //             duration: 40
    //             easing.amplitude: 1.5
    //             easing.type: Easing.OutElastic
    //             from: -4 + activeFocusHighlight.bounceAmplitude
    //             property: "anchors.leftMargin"
    //             target: activeFocusHighlight
    //             to: -4
    //         }
    //         PropertyAnimation {
    //             duration: 40
    //             easing.amplitude: 1.5
    //             easing.type: Easing.OutElastic
    //             from: -4 - activeFocusHighlight.bounceAmplitude
    //             property: "anchors.rightMargin"
    //             target: activeFocusHighlight
    //             to: -4
    //         }
    //     }
    // }
    // SequentialAnimation {
    //     id: highlightBounceAnimationLeft
    //
    //     running: false
    //
    //     ParallelAnimation {
    //         ScriptAction {
    //             script: {
    //                 sfx_player.play("uibump");
    //             }
    //         }
    //         PropertyAnimation {
    //             duration: 40
    //             easing.type: Easing.Linear
    //             from: -4
    //             property: "anchors.leftMargin"
    //             target: activeFocusHighlight
    //             to: -4 - activeFocusHighlight.bounceAmplitude
    //         }
    //         PropertyAnimation {
    //             duration: 40
    //             easing.type: Easing.Linear
    //             from: -4
    //             property: "anchors.rightMargin"
    //             target: activeFocusHighlight
    //             to: -4 + activeFocusHighlight.bounceAmplitude
    //         }
    //     }
    //     ParallelAnimation {
    //         PropertyAnimation {
    //             duration: 40
    //             easing.amplitude: 1.5
    //             easing.type: Easing.OutElastic
    //             from: -4 - activeFocusHighlight.bounceAmplitude
    //             property: "anchors.leftMargin"
    //             target: activeFocusHighlight
    //             to: -4
    //         }
    //         PropertyAnimation {
    //             duration: 40
    //             easing.amplitude: 1.5
    //             easing.type: Easing.OutElastic
    //             from: -4 + activeFocusHighlight.bounceAmplitude
    //             property: "anchors.rightMargin"
    //             target: activeFocusHighlight
    //             to: -4
    //         }
    //     }
    // }
    //
    // // NumberAnimation {
    // //     target: activeFocusHighlight
    // //     property: "opacity"
    // //     duration: 100
    // //     easing.type: Easing.InOutQuad
    // //     loops: Animation.Infinite
    // // }
    //
    // SequentialAnimation {
    //     loops: Animation.Infinite // Loop the animation infinitely
    //     running: true
    //
    //     NumberAnimation {
    //         duration: 2000
    //         easing.type: Easing.InOutQuad
    //         from: 0.65
    //         property: "opacity"
    //         target: activeFocusHighlight
    //         to: 1.0
    //     }
    //     NumberAnimation {
    //         duration: 2000
    //         easing.type: Easing.InOutQuad
    //         from: 1.0
    //         property: "opacity"
    //         target: activeFocusHighlight
    //         to: 0.65
    //     }
    // }
    // Connections {
    //     function onActiveFocusItemChanged() {
    //         let item = window.activeFocusItem;
    //         if (item && item.hasOwnProperty('showGlobalCursor') && item.showGlobalCursor) {
    //             let cursorItem = item;
    //
    //             if (cursorItem.hasOwnProperty("globalCursorProxy")) {
    //                 cursorItem = cursorItem.globalCursorProxy;
    //             }
    //             activeFocusHighlight.parent = cursorItem;
    //             if (cursorItem.hasOwnProperty('background')) {
    //                 activeFocusHighlight.radius = cursorItem.background.radius + 4;
    //             } else if (cursorItem.hasOwnProperty('radius')) {
    //                 activeFocusHighlight.radius = cursorItem.radius + 4;
    //             }
    //         } else {
    //             activeFocusHighlight.parent = null;
    //         }
    //     }
    //
    //     target: window
    // }
}
