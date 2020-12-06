import QtQuick 2.15
import QtQuick.Shapes 1.15

Rectangle {
    id: joypad

    signal moveUp()
    signal moveLeft()
    signal moveRight()
    signal moveDown()

    border { color: "black"; width: 3 }
    color: "red"

    radius: 50
    width: 2 * radius
    height: 2 * radius

    MouseArea {
        id: mouseArea

        property point start

        anchors.fill: parent

        onPressed: start = Qt.point(mouse.x, mouse.y)

        Timer {
            interval: 150
            repeat: true
            running: mouseArea.pressed

            onTriggered: {
                console.info("MOVE?");

                var dx = mouseArea.mouseX - mouseArea.start.x;
                var dy = mouseArea.mouseY - mouseArea.start.y;

                if (dx < -joypad.radius * 0.2)
                    joypad.moveLeft();
                else if (dx > joypad.radius * 0.2)
                    joypad.moveRight();
                else if (dy < -joypad.radius * 0.2)
                    joypad.moveUp();
                else if (dy > joypad.radius * 0.2)
                    joypad.moveDown();
            }
        }
    }
}
