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

import sys
import copy

from PIL import Image

from PySide6.QtCore import Signal, Slot
from PySide6.QtGui import QGuiApplication, QPainter, QColor, QPen
from PySide6.QtCore import Qt

from tilificator.spritetile import SpriteTile
from tilificator.common import indexOfMinimum
from tilificator.palettedimagewidget import PalettedImageWidget
from tilificator.tilingmethoddragqueen import TilingMethodDragQueen


class Rectangle:
    def __init__(self, x, y, w, h):
        self.x = x
        self.y = y
        self.w = w
        self.h = h


class RectanglesWidget(PalettedImageWidget):
    SelectionUpdated = Signal(PalettedImageWidget, list)

    def __init__(self, image=None, color=(0.0, 0.5, 0.0), selectedColor=(0.8, 0.0, 0.0), transparent=[]):
        super(RectanglesWidget, self).__init__(image, transparent)

        self.zoom = 2.0
        self.dragging = False
        self.dragDeltaX = 0
        self.dragDeltaY = 0
        self.lastMouseX = 0
        self.lastMouseY = 0

        self.color = color
        self.selectedColor = selectedColor
        self.rectangles = []
        self.selectedRectangles = []
        self.setMouseTracking(True)
        self.setFocusPolicy(Qt.ClickFocus)
        self.tilingMethod = None

    def paintEvent(self, event):
        super(RectanglesWidget, self).paintEvent(event)
        pen = QPen()
        pen.setWidth(2)
        pen.setCosmetic(True)
        # Draw rectangles
        painter = QPainter()
        painter.begin(self)
        pen.setColor(QColor.fromRgbF(*self.color))
        painter.setPen(pen)
        zoom = self.zoom
        for r in self.rectangles:
            painter.drawRect((self.padLeft + r.x) * zoom, (self.padTop + r.y) * zoom, r.w * zoom, r.h * zoom)
        pen.setColor(QColor.fromRgbF(*self.selectedColor))
        painter.setPen(pen)
        for r in self.selectedRectangles:
            painter.drawRect((self.padLeft + r.x) * zoom, (self.padTop + r.y) * zoom, r.w * zoom, r.h * zoom)

        # Draw cutRows
        try:
            self.imageCuts
            pen.setColor(QColor.fromRgbF(0.0, 0.0, 0.0))
            painter.setPen()
            for y in self.imageCuts:
                painter.drawLine(0 * zoom, (self.padTop + y) * zoom, (self.padLeft + self.padRight + self.imageWidth) * zoom, (self.padTop + y) * zoom)
            #print 'cutRows:'
            # for spriteTiles in self.cutRows:
            #    spriteTilesNum = [self.rectangles.index(st) for st in spriteTiles]
            #    print '  ' + str(spriteTilesNum)
            #print 'imageCuts = ' + str(self.imageCuts)
        except:
            pass
        painter.end()

    def redraw(self):
        self.repaint()

    def keyPressEvent(self, event):
        if event.key() == Qt.Key_Delete:
            rectangles = copy.copy(self.rectangles)
            for r in rectangles:
                if r in self.selectedRectangles:
                    self.rectangles.remove(r)
            self.selectedRectangles = []
            self.redraw()
            return True
        if event.key() == Qt.Key_F:
            for r in self.selectedRectangles:
                r.fixed = not r.fixed
                return True

    def mouseMoveEvent(self, event):
        x, y, width, height = event.x(), event.y(), self.imageWidth, self.imageHeight
        #self.tileColumn = min(self.TILE_COLUMNS-1, int(x//(Settings.tileWidth*4)))
        #self.tileRow = int(y//(Settings.tileHeight*4))
        if self.dragging:
            self.dragDeltaX += (x - self.lastMouseX) / self.zoom
            self.dragDeltaY += (y - self.lastMouseY) / self.zoom

            if self.tilingMethod is None:
                self.tilingMethod = TilingMethodDragQueen(self.si, None, Settings, OptimizationSettings)

            self.tilingMethod.linkSpriteTiles(self.rectangles)

            for st in self.rectangles:
                st.xmovement = 0
                st.shiftFreedom = None

                if st.leftOf == []:
                    st.shiftFreedom = self.tilingMethod.findFirstNonBlankColumn(self.si, st, 0, 7)
                elif st.leftOf != []:  # and st.leftOf[0].y != st.y:
                    startY = 0
                    endY = Settings.tileHeight - 1
                    for stL in st.leftOf:
                        if st.y < stL.y + Settings.tileHeight < st.y + Settings.tileHeight:
                            startY = stL.y - st.y + Settings.tileHeight
                        if st.y < stL.y < st.y + Settings.tileHeight:
                            endY = stL.y - st.y - 1
                    st.shiftFreedom = min([(stL.x + Settings.tileWidth - st.x) for stL in st.leftOf])  # findFirstNonBlankColumn(self.si, st, startY, endY)

            for r in self.selectedRectangles:
                if isinstance(r, Rectangle):
                    r.x += int(self.dragDeltaX)
                    r.y += int(self.dragDeltaY)
                    self.dragDeltaX -= int(self.dragDeltaX)
                    self.dragDeltaY -= int(self.dragDeltaY)
                else:
                    #maxDrag = getMaxDragToRight(r, self.si)
                    # if maxDrag > 0:
                    if int(self.dragDeltaX) > 0:
                        maxDrag = min(self.tilingMethod.draggableToRight(r, self.si), int(self.dragDeltaX))
                        if maxDrag > 0:
                            #r.x += min(int(self.dragDeltaX), maxDrag)
                            #print ("maxDrag = %d" % maxDrag)
                            self.tilingMethod.dragToRight(r, self.si, maxDrag)  # int(self.dragDeltaX))
                        self.dragDeltaX -= int(self.dragDeltaX)
                        self.dragDeltaY -= int(self.dragDeltaY)
                    elif int(self.dragDeltaX) < 0:
                        maxDrag = min(self.tilingMethod.draggableToLeft(r, self.si), int(-self.dragDeltaX))
                        if maxDrag > 0:
                            #r.x += min(int(self.dragDeltaX), maxDrag)
                            #print ("maxDrag = %d" % maxDrag)
                            self.tilingMethod.dragToLeft(r, self.si, maxDrag)  # int(self.dragDeltaX))
                        self.dragDeltaX -= int(self.dragDeltaX)
                        self.dragDeltaY -= int(self.dragDeltaY)
            for r in self.rectangles:
                r.x += r.xmovement
            self.redraw()
            #self.dragDeltaX -= int(self.dragDeltaX)
            #self.dragDeltaY -= int(self.dragDeltaY)
        self.lastMouseX = event.x
        self.lastMouseY = event.y
        x = int(x // self.zoom)
        y = int(y // self.zoom)

        return True

    @staticmethod
    def insideRectangle(x, y, r):
        return r.x <= x <= (r.x + r.w) and r.y <= y <= (r.y + r.h)

    def mousePressEvent(self, event):
        if event.button() == Qt.LeftButton:
            x = int(event.x() // self.zoom) - self.padLeft
            y = int(event.y() // self.zoom) - self.padTop
            ctrlDown = (QGuiApplication.keyboardModifiers() & Qt.ControlModifier)
            shiftDown = (QGuiApplication.keyboardModifiers() & Qt.ShiftModifier)
            selection = [r for r in self.rectangles if self.insideRectangle(x, y, r)]
            if selection != []:
                selectionAreas = [r.w * r.h for r in selection]
                selection = [selection[indexOfMinimum(selectionAreas)]]
            if (ctrlDown or shiftDown) and selection:
                #print "append"
                if selection[0] not in self.selectedRectangles:
                    self.selectedRectangles.append(selection[0])
            else:
                #print "no append"
                self.selectedRectangles = selection
            self.SelectionUpdated.emit(self, self.selectedRectangles)
            self.redraw()

            #self.dragging = True

        if event.button() == Qt.MiddleButton:
            print("middle button pressed")

        if event.button() == Qt.RightButton:
            for r in self.selectedRectangles:
                self.tilingMethod.dragFullyToRight(r, self.si, self.rectangles)
                r.fixed = True
            self.redraw()
            # for r in self.selectedRectangles:
            #    r.fixed = not r.fixed

        return True

    def mouseReleaseEvent(self, event):
        self.dragging = False
        return True
