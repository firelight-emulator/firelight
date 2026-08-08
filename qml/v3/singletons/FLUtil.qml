pragma Singleton

import QtQuick

// TODO
// App-wide, stateless QML helper functions. Add small pure utilities here instead
// of copy-pasting them into components
QtObject {
    // A remote/qrc/file url passes through unchanged; a bare local path (e.g.
    // user-imported art) becomes a file:// url with backslashes normalized, so
    // Image/VectorImage can load it. Empty in, empty out
    function toUrl(pathOrUrl: string): string {
        if (!pathOrUrl) {
            return "";
        }
        if (pathOrUrl.indexOf("://") >= 0) {
            return pathOrUrl;
        }
        return "file:///" + pathOrUrl.replace(/\\/g, "/");
    }

    function formatPlayTime(seconds) {
        var h = Math.floor(seconds / 3600);
        var m = Math.round((seconds % 3600) / 60);
        if (h > 0) {
            return h + "h " + m + "m";
        }
        if (m > 0) {
            return m + "m";
        }
        return "<1m";
    }

    function formatLastPlayed(millis) {
        if (!millis || millis <= 0) {
            return "";
        }
        return "Last played " + Qt.formatDateTime(new Date(millis), "MMM d, yyyy");
    }
}
