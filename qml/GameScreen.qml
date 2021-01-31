import QtQuick 2.15

// bitte in den IntroScreen gucken!

Screen {
    property int levelCount: 11
    signal bossBattle

    color: "white"

    Row {
//        Rectangle {
//            height: parent.height
//            width: parent.width / 2
//            MouseArea {
//                anchors.fill: parent
//                onClicked: console.info(["h", "w"], [parent.height, parent.width], [parent.height, parent.width * 0.5])
//            }
//        }

        anchors.fill: parent

        Sidebar {
            height: parent.height
            width: 200
        }

        GameGround {
            width: parent.width - x
            height: parent.height
        }
    }

    GameOverOverlay {
        anchors.fill: parent
    }

    Joypad {
        anchors {
            right: parent.right
            bottom: parent.bottom
            margins: 50
        }

        focus: true
        radius: 100

        onMoveUp: backend.player.moveUp()
        onMoveDown: backend.player.moveDown()
        onMoveLeft: backend.player.moveLeft()
        onMoveRight: backend.player.moveRight()
    }

    Keys.onSpacePressed: {
        if (!backend.player.isAlive)
            backend.respawn();
    }

    Keys.onEscapePressed: {
        console.info("isAlive:", backend.player.isAlive, ";",
                     "lives:", backend.player.lives, ";",
                     "energy:", backend.player.energy,
                     "levels:", levelCount,
                     "Tests:",
                     (parent.height - ((2 * 3) * 25) / 25),
                     (parent.height - ((2 * 3) * 35)) / 35,
                     )
    }

    Keys.onPressed: {
        if (event.key >= Qt.Key_0 && event.key <= Qt.Key_9) {
            var level = (event.key - Qt.Key_0 + 9) % 10;

            switch (event.modifiers & (Qt.ShiftModifier | Qt.ControlModifier | Qt.AltModifier)) {
            case Qt.NoModifier:
                break;
            case Qt.ControlModifier:
                level += 10;
                break;
            default:
                console.info("WAT");
                return;
            }

            console.info("level:", level);

            var button = levelButtons.itemAt(level);

            if (button)
                button.activated();
        }
    }
}
