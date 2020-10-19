# This Python file uses the following encoding: utf-8
import random
import os
import sys

from PySide2.QtCore import QObject, QPoint, Property, Signal, Slot
from PySide2.QtGui import QGuiApplication
from PySide2.QtQml import QQmlApplicationEngine

class Actor(QObject):
    def __init__(self, backend, x0=0, y0=0, **kwargs):
        super().__init__()

        self._backend = backend
        self._maximumEnergy = int(kwargs["maximumEnergy"])
        self._maximumLifes = int(kwargs.get("maximumLifes", 1))
        self._lives = self._maximumLifes
        self._x0 = int(x0)
        self._y0 = int(y0)
        self.respawn()

    @Slot()
    def moveLeft(self):
        if self.isAlive and self._x > 0:
            self._x = self._x - 1
            self.positionChanged.emit()

    @Slot()
    def moveRight(self):
        if self.isAlive and self._x < self._backend.columns - 1:
            self._x = self._x + 1
            self.positionChanged.emit()

    @Slot()
    def moveUp(self):
        if self.isAlive and self._y > 0:
            self._y = self._y - 1
            self.positionChanged.emit()

    @Slot()
    def moveDown(self):
        if self.isAlive and self._y < self._backend.rows - 1:
            self._y = self._y + 1
            self.positionChanged.emit()

    @Slot()
    def respawn(self):
        self._energy = self._maximumEnergy
        self._x = self._x0
        self._y = self._y0

        self.energyChanged.emit()
        self.livesChanged.emit()
        self.positionChanged.emit()

    @Slot()
    def attack(self, opponent):
        return 0

    @Slot()
    def stealEnergy(self, amount):
        self._energy = max(0, self._energy - amount)
        self.energyChanged.emit()

        if self._energy == 0:
            self.die()

    @Slot()
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

    x = Property(int, lambda self: self._x, notify=positionChanged)
    y = Property(int, lambda self: self._y, notify=positionChanged)
    position = Property(QPoint, lambda self: QPoint(self.x, self.y), notify=positionChanged)

    lives = Property(int, lambda self: self._lives, notify=livesChanged)
    isAlive = Property(bool, lambda self: self._lives > 0 and self._energy > 0, notify=energyChanged)

    energy = Property(int, lambda self: self._energy, notify=energyChanged)
    maximumEnergy = Property(int, lambda self: self._maximumEnergy, notify=maximumEnergyChanged)

class Player(Actor):
    def __init__(self, backend, **kwargs):
        super().__init__(backend, **kwargs)

    @Slot()
    def attack(self, opponent):
        if type(opponent) is Enemy:
            opponent.stealEnergy(1)
            return random.randint(0, 1)

        return 0

class Enemy(Actor):
    def __init__(self, backend, x0, y0, **kwargs):
        super().__init__(backend, x0, y0, **kwargs)

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

    @Slot()
    def attack(self, opponent):
        if type(opponent) is Player:
            opponent.stealEnergy(1)
            return random.randint(0, 1)

        return 0

class Backend(QObject):
    def __init__(self):
        QObject.__init__(self)

        self.__columns = 10
        self.__rows = 10

        self.__player = Player(self, maximumLifes=3, maximumEnergy=5)
        self.__enemies = [
            Enemy(self, 5, 5, maximumEnergy=10),
            Enemy(self, 5, 9, maximumEnergy=3),
            Enemy(self, 9, 5, maximumEnergy=3),
        ]

        for actor in self.actors:
            print("actor: %r" % actor)
            actor.positionChanged.connect(lambda a=actor: self.checkCombatRules(a))

    def checkCombatRules(self, actor):
        for opponent in self.actors:
            if opponent == actor or type(opponent) == type(actor):
                continue

            if type(actor) is Player:
                print("combat: %r" % [actor, opponent.position == actor.position, opponent.position, actor.position])

            if opponent.position == actor.position:

                actor.giveEnergy(actor.attack(opponent))

    columns = Property(int, lambda self: self.__columns, constant=True)
    rows = Property(int, lambda self: self.__rows, constant=True)

    actors = Property(list, lambda self: [self.__player] + self.__enemies, constant=True)
    player = Property(Player, lambda self: self.__player, constant=True)
    enemies = Property(list, lambda self: self.__enemies, constant=True)

if __name__ == "__main__":
    app = QGuiApplication(sys.argv)
    backend = Backend()

    engine = QQmlApplicationEngine()
    engine.rootContext().setContextProperty("backend", backend)

    srcdir = os.path.dirname(__file__)
    engine.load(os.path.join(srcdir, "qml", "main.qml"))

    if not engine.rootObjects():
        sys.exit(-1)

    sys.exit(app.exec_())
