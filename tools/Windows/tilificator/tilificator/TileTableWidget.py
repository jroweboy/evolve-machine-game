#! /usr/bin/env python
#
# Copyright (C) 2012-2018 Michel Iwaniec
#
# This file is part of Tilificator.
#
# Tilificator is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Tilificator is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Tilificator.  If not, see <http://www.gnu.org/licenses/>.
#

import array
from math import ceil

from PIL import Image

from PySide2.QtCore import Signal, Slot
from PySide2.QtGui import QGuiApplication, QPainter, QColor, QPen
from PySide2.QtWidgets import QFileDialog
from PySide2.QtCore import Qt

from tilificator.sprite import *
from tilificator.tile import *
from tilificator.palettedimagewidget import PalettedImageWidget


class TileTableWidget(PalettedImageWidget):
    TileTableUpdated = Signal(PalettedImageWidget, list)
    SelectionUpdated = Signal(PalettedImageWidget, list)

    def __init__(self, tileTable):
        PalettedImageWidget.__init__(self, image=None, transparent=[0])
        self.palette = array.array('B', [(int(i / 3) * 16) % 256 for i in range(768)])
        self.tileTable = tileTable
        self.TILE_COLUMNS = 16
        self.selection = []
        self.tileColumn = 0
        self.tileRow = 0
        self.zoom = 4

        self.resize(600, 600)
        self.setMouseTracking(True)
        self.setFocusPolicy(Qt.ClickFocus)

        self.resize(self.TILE_COLUMNS * self.tileTable.settings.tileWidth, self.tileTable.settings.tileHeight)

    def redraw(self):
        self.repaint()

    def paintEvent(self, event):
        if self.tileTable.numTiles > 0:
            super(TileTableWidget, self).paintEvent(event)
            if self.selection:
                painter = QPainter()
                pen = QPen()
                pen.setWidth(2)
                pen.setCosmetic(True)
                painter.begin(self)
                pen.setColor(QColor.fromRgbF(0.8, 0.0, 0.0))
                painter.setPen(pen)
                zoom = self.zoom
                for tileSelected in self.selection:
                    row = int(tileSelected / self.TILE_COLUMNS)
                    column = int(tileSelected % self.TILE_COLUMNS)
                    settings = self.tileTable.settings
                    x, y = column * settings.tileWidth, row * settings.tileHeight
                    painter.drawRect(x * zoom, y * zoom, settings.tileWidth * zoom, settings.tileHeight * zoom)
                painter.end()

    def mouseMoveEvent(self, event):
        x, y, width, height = event.x(), event.y(), self.width(), self.height()
        settings = self.tileTable.settings
        self.tileColumn = min(self.TILE_COLUMNS - 1, int(x // (settings.tileWidth * 4)))
        self.tileRow = int(y // (settings.tileHeight * 4))
        #print('mouse motion event: x = ' + str(x) + ', y = ' + str(y) + ', w = ' +str(width) + ', h = ' + str(height))
        return True

    def mousePressEvent(self, event):
        if event.button() == Qt.LeftButton:
            tileSelected = self.tileRow * self.TILE_COLUMNS + self.tileColumn
            if tileSelected < self.tileTable.numTiles:
                ctrlDown = (QGuiApplication.keyboardModifiers() & Qt.ControlModifier)
                shiftDown = (QGuiApplication.keyboardModifiers() & Qt.ShiftModifier)
                if ctrlDown or shiftDown:
                    if tileSelected in self.selection:
                        self.selection.remove(tileSelected)
                    else:
                        self.selection.append(tileSelected)
                    self.selection.sort()
                else:
                    self.selection = [tileSelected]
                self.SelectionUpdated.emit(self, self.selection)
                self.redraw()
        return True

    def keyPressEvent(self, event):
        if event.key() == Qt.Key_S:
            filename, fileType = QFileDialog.getSaveFileName(parent=self,
                                                             caption='Open project',
                                                             filter='Tilificator tile data (*.til)')
            if filename != '':
                self.tileTable.save(filename)

        if event.key() == Qt.Key_Delete:
            self.deleteSelectedTiles()
            self.tileTableChanged()
            self.redraw()

    def selectTiles(self, selection):
        self.selection = selection
        self.redraw()

    def deleteSelectedTiles(self):
        selectedTiles = self.selection

        # Build tile index remapping tables
        forwardRemapping = [0] * self.tileTable.numTiles
        inverseRemapping = [0] * (self.tileTable.numTiles - len(selectedTiles))
        n = 0
        for i in range(self.tileTable.numTiles):
            forwardRemapping[i] = n
            if n < len(inverseRemapping):
                inverseRemapping[n] = i
            if i not in selectedTiles:
                n += 1

        # Move tile data
        newData = array.array('B', [0] * self.tileTable.settings.tileWidth * self.tileTable.settings.tileHeight * len(inverseRemapping))
        settings = self.tileTable.settings
        tileSize = settings.tileWidth * settings.tileHeight
        for i in range(self.tileTable.numTiles - len(selectedTiles)):
            for k in range(tileSize):
                newData[tileSize * i + k] = self.tileTable.data[tileSize * inverseRemapping[i] + k]

        self.tileTable.removeTiles(self.tileTable.numTiles)
        for i, tile in enumerate(inverseRemapping):
            self.tileTable.addTile(newData[tileSize * i:tileSize * (i + 1)])
        self.TileTableUpdated.emit(self, forwardRemapping)
        self.selectTiles([])
        self.SelectionUpdated.emit(self, self.selection)

    def tileTableChanged(self):
        self.numTileRows = int(ceil(float(self.tileTable.numTiles) / self.TILE_COLUMNS))
        settings = self.tileTable.settings
        data = array.array('B', [0] * settings.tileWidth * self.TILE_COLUMNS * self.numTileRows * settings.tileHeight)
        for i in range(self.tileTable.numTiles):
            for yy in range(settings.tileHeight):
                for xx in range(settings.tileWidth):
                    tileColumn = i % self.TILE_COLUMNS
                    tileRow = i // self.TILE_COLUMNS
                    x = tileColumn * settings.tileWidth + xx
                    y = tileRow * settings.tileHeight + yy

                    offs = (settings.tileWidth * self.TILE_COLUMNS) * y + x

                    c = self.tileTable.data[settings.tileWidth * (settings.tileHeight * i + yy) + xx]
                    data[offs] = c

        if self.tileTable.numTiles > 0:
            self.image = Image.frombuffer('P', (settings.tileWidth * self.TILE_COLUMNS, self.numTileRows * settings.tileHeight), data, 'raw', 'P', 0, 1)
            self.image.putpalette(self.palette)
        else:
            self.image = None
        self.imageChanged()

        self.resize(self.TILE_COLUMNS * settings.tileWidth * self.zoom, self.numTileRows * settings.tileHeight * self.zoom)
        self.redraw()
        self.TileTableUpdated.emit(self, [])
