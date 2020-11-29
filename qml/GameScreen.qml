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
            height: parent.height
            width: parent.width
        }

        GameGround {
            x: parent.width - Sidebar.width

            height: parent.height
            width: parent.width
        }
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
