import QtQuick 2.15

Rectangle {
    opacity: enabled ? 1 : 0
    visible: opacity > 0

    Behavior on opacity {
        NumberAnimation {
            duration: 250
        }
    }

    x: enabled ? 0 : -width

    Behavior on x {
        NumberAnimation {
            duration: 550
            easing { type: Easing.OutBounce; overshoot: 1.2 }
        }
    }
}
