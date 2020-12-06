import QtQuick 2.15

// bitte in den IntroScreen gucken!

Screen {
    property int levelCount: 11
    signal battle

    color: "white"
    focus: true

    Row {
        anchors.fill: parent

        Sidebar {
            width: 200
            height: parent.height
        }

        Rectangle {
            height: parent.height
            width: 10
            color: "pink"
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

        radius: 100

        onMoveUp: backend.player.moveUp()
        onMoveDown: backend.player.moveDown()
        onMoveLeft: backend.player.moveLeft()
        onMoveRight: backend.player.moveRight()
    }

    Keys.onLeftPressed:     backend.player.moveLeft()
    Keys.onRightPressed:    backend.player.moveRight()
    Keys.onUpPressed:       backend.player.moveUp()
    Keys.onDownPressed:     backend.player.moveDown()

    Keys.onSpacePressed: if (!backend.player.isAlive) backend.player.respawn()
    Keys.onEscapePressed: console.info("isAlive:", backend.player.isAlive, ";",
                                       "lives:", backend.player.lives, ";",
                                       "energy:", backend.player.energy,
                                       "levels:", levelCount,
                                       "Tests:",
                                            (parent.height - ((2 * 3) * 25) / 25),
                                            (parent.height - ((2 * 3) * 35)) / 35,
                                        )

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
