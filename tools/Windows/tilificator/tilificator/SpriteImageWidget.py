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

from PIL import Image

from PySide2.QtCore import Signal, Slot
from PySide2.QtWidgets import QLabel, QGroupBox, QHBoxLayout
from PySide2.QtCore import Qt, QSize

from tilificator.sprite import SpriteImage
from tilificator.rectangles_widget import RectanglesWidget


class SpriteImageWidget(QGroupBox):
    SelectionUpdated = Signal(QGroupBox, list)

    def __init__(self, spriteImage):
        super(SpriteImageWidget, self).__init__()
        self.selection = []
        self.spriteImage = spriteImage
        if self.spriteImage:
            self.spriteImage.spriteCoverage = None

        self.hbox = QHBoxLayout()

        self.rectanglesWidget = RectanglesWidget(image=None, transparent=[0])
        self.rectanglesWidget.setParent(self)
        self.rectanglesWidget.si = spriteImage

        self.label = QLabel()
        self.hbox.addWidget(self.rectanglesWidget)
        self.hbox.addWidget(self.label)
        self.setLayout(self.hbox)

        if spriteImage:
            self.setSpriteImage(spriteImage)

        self.setMouseTracking(True)

        self.rectanglesWidget.SelectionUpdated.connect(self.selectionUpdated)

    def setSpriteImage(self, spriteImage):
        self.spriteImage = spriteImage
        self.spriteImage.spriteCoverage = None
        self.rectanglesWidget.si = spriteImage
        self.rectanglesWidget.image = Image.frombuffer('P', (spriteImage.width, spriteImage.height), spriteImage.data, 'raw', 'P', 0, 1)
        self.rectanglesWidget.image.putpalette(spriteImage.palette)
        self.rectanglesWidget.imageChanged()
        self.rectanglesWidget.resize(spriteImage.width, spriteImage.height)

    def selectTiles(self, tiles):
        self.rectanglesWidget.selectedRectangles = [r for r in self.rectanglesWidget.rectangles if r.tileID in tiles]
        self.selection = [r.tileID for r in self.rectanglesWidget.rectangles if r.tileID in tiles]
        self.SelectionUpdated.emit(self, self.selection)
        self.rectanglesWidget.redraw()

    def selectionUpdated(self, widget, selection):
        self.selection = [r.tileID for r in selection]
        self.SelectionUpdated.emit(self, self.selection)

    def spriteTilesUpdated(self):
        try:
            self.rectanglesWidget.rectangles = self.spriteImage.tilification.tiles
        except:
            self.rectanglesWidget.rectangles = []
        self.rectanglesWidget.selectedRectangles = []
        try:
            self.rectanglesWidget.imageCuts = self.spriteImage.imageCuts
            self.rectanglesWidget.cutRows = self.spriteImage.cutRows
        except:
            pass

    @staticmethod
    def index(stList, st):
        if st in stList:
            return stList.index(st)
        else:
            return -1

    def updateLabel(self):
        try:
            text = ''
            for st in self.rectanglesWidget.rectangles:
                spanBegin = ''
                spanEnd = ''
                if st in self.rectanglesWidget.selectedRectangles:
                    spanBegin = '<span foreground="red">'
                    spanEnd = '</span>'

                startY, endY = getLeftBorderGap(st)
                maxDragL = 1 if draggableToLeft(st, self.rectanglesWidget.si) else 0
                maxDragR = 1 if draggableToRight(st, self.rectanglesWidget.si) else 0  # getMaxDragToRight(st, self.rectanglesWidget.si)
                leftOf = [self.index(self.rectanglesWidget.rectangles, stL) for stL in st.leftOf]
                rightOf = [self.index(self.rectanglesWidget.rectangles, stR) for stR in st.rightOf]
                text += spanBegin + ('(x,y) = (%d,%d); gap = (%d,%d); maxDrag = (%d,%d)' % (st.x, st.y, startY, endY, maxDragL, maxDragR)) + spanEnd + '\n'
                text += '    ' + spanBegin + str(leftOf) + '   ' + str(rightOf) + spanEnd + '\n'
            self.label.set_markup(text)
        except:
            pass

    def width(self):
        return self.rectanglesWidget.width()

    def height(self):
        return self.rectanglesWidget.height()

    def sizeHint(self):
        return QSize(self.width(), self.height())
