pragma Singleton

import QtQuick
import Firelight 1.0

// Typed read path for the general app settings declared in the settings
// catalog, mirroring AppearanceSettings. The settings page writes these through
// SettingsModel; the rest of the app binds to the names here
//
// The value properties are read-only: assigning to one would break its binding
// and silently stop tracking the store. The set* functions are for the few
// places that change a setting from outside the settings page
//
// Window geometry lives in WindowGeometry, not here — it isn't a setting
QtObject {
    id: root

    property SettingBinding fullscreenBinding: SettingBinding { key: "fullscreen" }
    property SettingBinding advancedBinding: SettingBinding { key: "show-advanced-settings" }
    property SettingBinding sortBinding: SettingBinding { key: "library-sort-method" }
    property SettingBinding newUserBinding: SettingBinding { key: "show-new-user-flow" }

    readonly property bool fullscreen: fullscreenBinding.value === "true"
    readonly property bool showAdvancedSettings: advancedBinding.value === "true"
    readonly property bool showNewUserFlow: newUserBinding.value === "true"
    readonly property string librarySortMethod: sortBinding.value

    function setFullscreen(value) {
        fullscreenBinding.value = value ? "true" : "false";
    }

    function setShowAdvancedSettings(value) {
        advancedBinding.value = value ? "true" : "false";
    }

    function setShowNewUserFlow(value) {
        newUserBinding.value = value ? "true" : "false";
    }

    function setLibrarySortMethod(value) {
        sortBinding.value = value;
    }
}
