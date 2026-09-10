// TODO: NEEDS REVIEW
import QtQuick
import QtQuick.Layouts
import Firelight 1.0

// TODO
// A column the cursor walks by geometry. It describes itself and moves nothing: what lies which way
// is the navigator's to answer
ColumnLayout {
    id: root

    // TODO
    // Answers for its children when a press asks what it can reach, so something level with any
    // part of this is level with the whole of it
    FLFocus.container: true
}
