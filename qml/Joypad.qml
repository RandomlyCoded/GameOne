import QtQuick 2.15
import QtGraphicalEffects 1.12

Item {
    id: joypad

    property alias radius: outerCircle.radius
    property alias innerRadius: innerCircle.radius

    signal moveUp()
    signal moveLeft()
    signal moveRight()
    signal moveDown()

    readonly property real currentAngle: {
        var dx = mouseArea.mouseX - mouseArea.start.x;
        var dy = mouseArea.mouseY - mouseArea.start.y;
        return Math.atan2(dy, dx);
    }

    readonly property real currentSpeed: {
        var dx = delta.x / radius;
        var dy = delta.y / radius;
        return Math.sqrt(dx * dx + dy * dy);
    }

    readonly property int currentSector: {
        if (delta.x || delta.y)
            return mouseArea.sectorForAngle(currentAngle);

        return -1;
    }

    readonly property point delta: {
        if (!mouseArea.pressed)
            return Qt.point(0, 0);

        var dx = mouseArea.mouseX - mouseArea.start.x;
        var dy = mouseArea.mouseY - mouseArea.start.y;

        var phi = Math.atan2(dy, dx);

        var cx = joypad.radius * Math.cos(phi);
        var cy = joypad.radius * Math.sin(phi);

        if (cx < 0)
            dx = Math.max(dx, cx);
        else if (cx > 0)
            dx = Math.min(dx, cx);

        if (cy < 0)
            dy = Math.max(dy, cy);
        else if (cy > 0)
            dy = Math.min(dy, cy);

        return Qt.point(dx, dy);
    }

    width: outerCircle.width
    height: outerCircle.height

    Item {
        id: directionIndicator

        property int highlightedSector: -1

        onHighlightedSectorChanged: resetHighlight.start()

        Timer {
            id: resetHighlight

            repeat: false
            interval: 150

            onTriggered: directionIndicator.highlightedSector = -1
        }

        anchors.fill: parent

        Repeater {
            id: indictors

            model: 4

            Rectangle {
                readonly property bool highlighted: joypad.currentSector === modelData
                                                    || directionIndicator.highlightedSector === modelData

                antialiasing: true
                border { width: 1; color: "black" }
                color: ["orange", "lime", "pink", "cyan"][modelData]

                anchors {
                    centerIn: parent
                    horizontalCenterOffset: [0, +1, 0, -1][modelData] * Math.sqrt(2 * Math.pow(joypad.radius, 2))/2
                    verticalCenterOffset: [-1, 0, +1, 0][modelData] * Math.sqrt(2 * Math.pow(joypad.radius, 2))/2
                }

                opacity: highlighted ? 1 : 0.1
                Behavior on opacity { NumberAnimation { duration: 100 } }

                width: joypad.radius
                height: joypad.radius
                rotation: 45
            }
        }

        visible: false
    }

    OpacityMask {
        anchors.fill: parent
        source: directionIndicator
        maskSource: Circle {
            radius: joypad.radius
            visible: false
        }
    }

    Circle {
        id: outerCircle

        anchors.centerIn: parent

        border { color: "black"; width: 3 }
        color: "transparent"

        radius: 50
    }

    Circle {
        id: innerCircle

        anchors {
            centerIn: parent
            horizontalCenterOffset: joypad.delta.x
            verticalCenterOffset: joypad.delta.y
        }

        color: "black"
        radius: 20
    }

    Timer {
        // Bewegungsgeschwindigkeit anhand der Auslenkung festlegen
        interval: 350 - 300 * Math.pow(joypad.currentSpeed, 1.5)

        repeat: true
        running: mouseArea.pressed

        onTriggered: mouseArea.runSectorAction(currentSector)
    }

    MouseArea {
        id: mouseArea

        property point start

        function sectorForAngle(angle) {
            return Math.floor((angle / (Math.PI/2) + 5.5) % 4);
        }

        function runSectorAction(sector) {
            var actionId = ["moveUp", "moveRight", "moveDown", "moveLeft"][sector];
            var action = joypad[actionId];

            if (typeof(action) === "function") {
                directionIndicator.highlightedSector = sector;
                action();
            }
        }

        anchors.fill: parent

        onPressed: {
            start = Qt.point(mouse.x, mouse.y);
        }

        onClicked: {
            var dx = mouseArea.mouseX - width/2;
            var dy = mouseArea.mouseY - height/2;
            var angle = Math.atan2(dy, dx);
            var sector = sectorForAngle(angle);

            runSectorAction(sector);
        }
    }

    Keys.onUpPressed: mouseArea.runSectorAction(0)
    Keys.onRightPressed: mouseArea.runSectorAction(1)
    Keys.onDownPressed: mouseArea.runSectorAction(2)
    Keys.onLeftPressed: mouseArea.runSectorAction(3)
}

