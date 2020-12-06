# This Python file uses the following encoding: utf-8

from datetime import datetime
from enum import IntEnum, auto

import json
import os
import random
import sys

from PySide2.QtCore import (
    QAbstractListModel, QByteArray, QEnum, QObject, QPoint, QTimer,
    Property, Signal, Slot,
)

from PySide2.QtGui import (
    Qt, QGuiApplication,
)

from PySide2.QtQml import (
    QQmlApplicationEngine,
    qmlRegisterType,
)

class Actor(QObject):
    def __init__(self, backend, **kwargs):
        super().__init__()

        self._backend = backend
        self._name = str(kwargs.get("name", "no name"))
        self._maximumEnergy = int(kwargs["maximumEnergy"])
        self._maximumLifes = int(kwargs.get("maximumLifes", 1))
        self._lives = self._maximumLifes
        self._origin = QPoint(int(kwargs["x"]), int(kwargs["y"]))
        self.respawn()

    @Slot()
    def moveLeft(self):
        destination = self._position + QPoint(-1, 0)
        if self._backend.canMoveTo(self, destination):
            self._position = destination
            self.positionChanged.emit()

    @Slot()
    def moveUp(self):
        destination = self._position + QPoint(0, -1)
        if self._backend.canMoveTo(self, destination):
            self._position = destination
            self.positionChanged.emit()

    @Slot()
    def moveDown(self):
        destination = self._position + QPoint(0, +1)
        if self._backend.canMoveTo(self, destination):
            self._position = destination
            self.positionChanged.emit()

    @Slot()
    def moveRight(self):
        destination = self._position + QPoint(+1, 0)
        if self._backend.canMoveTo(self, destination):
            self._position = destination
            self.positionChanged.emit()

    @Slot()
    def respawn(self):
        self._energy = self._maximumEnergy
        self._position = self._origin

        self.energyChanged.emit()
        self.positionChanged.emit()
        self.livesChanged.emit()

    @Slot(QObject, result=int)
    def attack(self, opponent):
        return 0

    @Slot(int)
    def stealEnergy(self, amount):
        self._energy = max(0, self._energy - amount)
        self.energyChanged.emit()

        if self._energy == 0:
            self.die()

    @Slot(int)
    def giveEnergy(self, amount):
        self._energy = min(self._maximumEnergy, self._energy + amount)
        self.energyChanged.emit()

    @Slot()
    def die(self):
        if self._lives > 0:
            self._lives = self._lives - 1
            self.livesChanged.emit()

    @Signal
    def positionChanged(self): pass

    @Signal
    def livesChanged(self): pass

    @Signal
    def energyChanged(self): pass

    @Signal
    def maximumEnergyChanged(self): pass

    type = Property(str, lambda self: type(self).__name__, constant=True)

    x = Property(int, lambda self: self._position.x(), notify=positionChanged)
    y = Property(int, lambda self: self._position.y(), notify=positionChanged)
    position = Property(QPoint, lambda self: self._position, notify=positionChanged)

    name = Property(str, lambda self: self._name, constant=True)
    lives = Property(int, lambda self: self._lives, notify=livesChanged)
    isAlive = Property(bool, lambda self: self._lives > 0 and self._energy > 0, notify=energyChanged)

    energy = Property(int, lambda self: self._energy, notify=energyChanged)
    maximumEnergy = Property(int, lambda self: self._maximumEnergy, notify=maximumEnergyChanged)

class Player(Actor):
    def __init__(self, backend, **kwargs):
        super().__init__(backend, **kwargs)

    @Slot(Actor, result=int)
    def attack(self, opponent):
        if type(opponent) is Enemy:
            opponent.stealEnergy(1)
            return random.randint(0, 1)

        return 0

class Enemy(Actor):
    def __init__(self, backend, **kwargs):
        super().__init__(backend, **kwargs)

    @Slot()
    def act(self):
        action = random.randint(0, 4)

        if action == 0 and backend.player.x < self.x:
            self.moveLeft()

        if action == 1 and backend.player.y < self.y:
            self.moveUp()

        if action == 2 and backend.player.x > self.x:
            self.moveRight()

        if action == 3 and backend.player.y > self.y:
            self.moveDown()

    @Slot(Actor, result=int)
    def attack(self, opponent):
        if type(opponent) is Player:
            opponent.stealEnergy(1)
            return random.randint(0, 1)

        return 0

class Tile:
    class Type:
        def __init__(self, name, walkable):
            self.name = name
            self.walkable = walkable

    types = {
        "G": Type(name="Grass",      walkable=True),
        "W": Type(name="DeepWater",  walkable=False),
        "w": Type(name="Water",      walkable=True),
        "H": Type(name="Hill",       walkable=True),
        "M": Type(name="Mountain",   walkable=False),
        "S": Type(name="Sand",       walkable=True),
        "I": Type(name="Ice",        walkable=True),
        "L": Type(name="Lava",       walkable=True),
        "@": Type(name="Tree",       walkable=False),
        "#": Type(name="Fence",      walkable=False),
        "-": Type(name="Fence",      walkable=False),
        "|": Type(name="Fence",      walkable=False),
        "/": Type(name="Fence",      walkable=False),
        "\\": Type(name="Fence",     walkable=False),
    }

    def __init__(self, spec):
        tspec = spec[0]
        ispec = (spec + ' ')[1]

        if tspec == 'T':
            tspec = 'G'
            ispec = '@'
        elif tspec == 'F':
            tspec = 'G'
            ispec = '#'

        self.type = Tile.types[tspec]
        self.item = Tile.types.get(ispec)

    def isWalkable(self):
        if self.item and not self.item.walkable:
            return False

        return self.type.walkable

class MapModel(QAbstractListModel):
    @QEnum
    class Roles(IntEnum):
        Type = Qt.UserRole + 1
        Column = auto()
        Row = auto()
        Walkable = auto()
        Item = auto()

    def __init__(self):
        super().__init__()

    def roleNames(self):
        return {
            self.Roles.Type: b"type",
            self.Roles.Item: b"item",
            self.Roles.Column: b"column",
            self.Roles.Row: b"row",
            self.Roles.Walkable: b"walkable",
        }

    @Slot(str, int, result=bool)
    def load(self, filename, format=1):
        print("Loading map from %s" % filename)

        srcdir = os.path.dirname(__file__)
        filename = os.path.join(srcdir, "data", filename)

        with open(filename, "r") as file:
            tiles = file.read().strip().split('\n')

            if format == 2:
                tiles.pop()

            tiles = [row.strip() for row in tiles]

            if format == 2:
                tiles = [[Tile(r[i:i+2]) for i in range(0, len(r), 2)] for r in tiles]

            elif format == 1:
                tiles = [[Tile(c) for c in r] for r in tiles]

            self.beginResetModel()
            self.__tiles = tiles
            self.__columns = len(self.__tiles[0])
            self.__rows = len(self.__tiles)
            self.endResetModel()

            self.columnsChanged.emit()
            self.rowsChanged.emit()

        return True

    def rowCount(self, parent):
        if parent.isValid():
            return 0

        return self.__columns * self.__rows

    def data(self, index, role):
        if self.hasIndex(index.row(), index.column(), index.parent()):
            row = index.row() // self.__columns
            column = index.row() % self.__columns
            tile = self.__tiles[row][column]

            if role == self.Roles.Column:
                return column
            elif role == self.Roles.Row:
                return row
            elif role == self.Roles.Type:
                return tile.type.name
            elif role == self.Roles.Item:
                return tile.item and tile.item.name or None
            elif role == self.Roles.Walkable:
                return tile.isWalkable()

        return None

    def indexByPoint(self, p):
        return self.index(p.y() * self.__columns + p.x())

    def dataByPoint(self, p, role):
        return self.data(self.indexByPoint(p), role)

    @Signal
    def columnsChanged(self): pass

    @Signal
    def rowsChanged(self): pass

    columns = Property(int, lambda self: self.__columns, notify=columnsChanged)
    rows = Property(int, lambda self: self.__rows, notify=rowsChanged)

class Backend(QObject):
    def __init__(self):
        super().__init__()

        self.__timer = QTimer()
        self.__timer.setInterval(100)
        self.__timer.timeout.connect(self.__onTimeout)

        self.__player = None
        self.__enemies = []

        self.__map = MapModel()
        self.__map.columnsChanged.connect(self.columnsChanged)
        self.__map.rowsChanged.connect(self.rowsChanged)

        self.load("level1.json")

    def __onTimeout(self):
        for enemy in self.__enemies:
            enemy.act()

    @Slot(str, result=bool)
    def load(self, filename):
        print("Loading level from %s" % filename)

        if self.__player:
            self.__player.disconnect(self)

        self.__timer.stop()

        srcdir = os.path.dirname(__file__)
        filename = os.path.join(srcdir, "data", filename)

        if not os.path.isfile(filename):
            return False

        if filename.endswith(".json"):
            level = json.load(open(filename, "r"))

            minfo = level["map"]
            self.__map.load(minfo["filename"], format=minfo["format"])

            self.__enemies = []
            for einfo in level["enemies"]:
                self.__enemies.append(Enemy(self, **einfo))

            pinfo = level["player"]
            self.__player = Player(self, **pinfo)

            self.__player.positionChanged.connect(self.__onPlayerPositionChanged)
            self.__player.livesChanged.connect(self.__onPlayerLivesChanged)

            self.actorsChanged.emit()
            self.enemiesChanged.emit()
            self.playerChanged.emit()

            return True

        elif filename.endswith(".txt"):
            return (self.load("level1.json")
                    and self.__map.load(filename))

        else:
            print("Unsupported filename: %s", filename)
            return False

    def __onPlayerPositionChanged(self):
        self.__timer.start()

    def __onPlayerLivesChanged(self):
        self.__timer.stop()

    def canMoveTo(self, actor, to):
        if not actor.isAlive:
            return False

        if to.x() < 0 or to.x() >= self.columns:
            return False
        if to.y() < 0 or to.y() >= self.rows:
            return False
        if not self.__map.dataByPoint(to, MapModel.Roles.Walkable):
            return False

        for opponent in self.actors:
            if opponent == actor:
                continue

            if not opponent.isAlive:
                continue

            if to == opponent.position:
                if type(opponent) != type(actor):
                    actor.giveEnergy(actor.attack(opponent))

                return False

        return True

    @Signal
    def columnsChanged(self): pass

    @Signal
    def rowsChanged(self): pass

    @Signal
    def actorsChanged(self): pass

    @Signal
    def enemiesChanged(self): pass

    @Signal
    def playerChanged(self): pass

    columns = Property(int, lambda self: self.__map.columns, notify=columnsChanged)
    rows = Property(int, lambda self: self.__map.rows, notify=rowsChanged)

    actors = Property(list, lambda self: [self.__player] + self.__enemies, notify=actorsChanged)
    enemies = Property(list, lambda self: self.__enemies, notify=enemiesChanged)
    player = Property(Player, lambda self: self.__player, notify=playerChanged)
    map = Property(QObject, lambda self: self.__map, constant=True)

if __name__ == "__main__":
    app = QGuiApplication(sys.argv)
    backend = Backend()

    qmlRegisterType(MapModel, "GameOne", 1, 0, "MapModel")

    engine = QQmlApplicationEngine()
    engine.rootContext().setContextProperty("backend", backend)

    srcdir = os.path.dirname(__file__)
    engine.load(os.path.join(srcdir, "qml", "main.qml"))

    if not engine.rootObjects():
        sys.exit(-1)

    sys.exit(app.exec_())
