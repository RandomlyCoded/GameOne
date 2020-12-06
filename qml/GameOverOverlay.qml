import QtQuick 2.15

Rectangle {
    id: gameOverOverlay

    property real animationProgress: 0

    color: "#aa220000"

    Text {
        anchors.centerIn: parent

        color: "red"
        font { pixelSize: 100; bold: true; italic: true }

        style: Text.Outline
        styleColor: gameOverOverlay.animationProgress < 0.5 ? "pink" : "lime"
        Behavior on styleColor { ColorAnimation { duration: 50 } }

        text: "Game Over"

        NumberAnimation {
            from: 0
            to: 1
            duration: 350
            loops: Animation.Infinite
            target: gameOverOverlay
            property: "animationProgress"
            running: true
        }
    }

    visible: opacity > 0
    opacity: backend && backend.player.lives === 0 ? 1 : 0
    Behavior on opacity { NumberAnimation { duration: 750 } }
}
