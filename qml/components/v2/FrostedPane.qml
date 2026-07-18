import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Window
import QtQml.Models
import QtQuick.Layouts 1.0
import QtNetwork
import QtMultimedia
import QtQuick.Effects
import Firelight 1.0

Pane {
    id: root

    background: Item {
        Rectangle {
            id: theMask3
            visible: false
            anchors.fill: parent
            color: "#000000"
            radius: 8
            layer.enabled: true
            layer.smooth: true
        }

        MultiEffect {
            source: ShaderEffectSource {
                width: root.width
                height: root.height
                sourceItem: window.background
                sourceRect: Qt.rect(root.x, root.y, root.width, root.height)
            }

            maskEnabled: true
            maskSource: theMask3
            maskThresholdMin: 0.5
            maskSpreadAtMin: 1.0
            autoPaddingEnabled: false
            anchors.fill: parent
            blurEnabled: true
            blurMultiplier: 1.0
            blurMax: 48
            blur: 1.0
        }

        Rectangle {
            anchors.fill: parent
            color: "#000000"
            opacity: 0.1
            radius: 8
        }

        Rectangle {
            anchors.fill: parent
            color: "transparent"
            opacity: 0.14

            border.width: 1
            border.color: "white"

            radius: 8
        }
    }
}
