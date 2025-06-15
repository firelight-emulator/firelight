import QtQuick
import QtQuick.Controls
import Firelight 1.0

Item {
    id: root

    required property int platformId
    required property string contentHash

    onPlatformIdChanged: {
        console.log("Platform ID changed to:", root.platformId);
    }

    onContentHashChanged: {
        console.log("Content hash changed to:", root.contentHash);
    }

    property bool rewindEnabled: EmulationSettingsManager.getEffectiveValue(root.contentHash, root.platformId, "rewind-enabled", "true") === "true";
    property string pictureMode: EmulationSettingsManager.getEffectiveValue(root.contentHash, root.platformId, "picture-mode", "aspect-ratio-fill");

    function refreshValue(name) {
        if (name === "rewind-enabled") {
            root.rewindEnabled = EmulationSettingsManager.getEffectiveValue(root.contentHash, root.platformId, "rewind-enabled", "true") === "true";
        } else if (name === "picture-mode") {
            root.pictureMode = EmulationSettingsManager.getEffectiveValue(root.contentHash, root.platformId, "picture-mode", "aspect-ratio-fill");
        }
    }

    function setGameValue(name, value) {
        EmulationSettingsManager.setGameValue(root.contentHash, root.platformId, name, value);
    }

    function getGameValue(name) {
        return EmulationSettingsManager.getGameValue(root.contentHash, root.platformId, name);
    }

    Connections {
        target: EmulationSettingsManager

        function onGlobalValueChanged(name, value) {
            root.refreshValue(name);
        }

        function onPlatformValueChanged(platformId, name, value) {
            if (platformId === root.platformId) {
                root.refreshValue(name);
            }
        }

        function onGameValueChanged(contentHash, platformId, name, value) {
            if (contentHash === root.contentHash && platformId === root.platformId) {
                root.refreshValue(name);
            }
        }

        function onGlobalValueReset(name) {
            root.refreshValue(name);
        }

        function onPlatformValueReset(platformId, name) {
            if (platformId === root.platformId) {
                root.refreshValue(name);
            }
        }

        function onGameValueReset(contentHash, platformId, name) {
            if (contentHash === root.contentHash && platformId === root.platformId) {
                root.refreshValue(name);
            }
        }
    }


}
