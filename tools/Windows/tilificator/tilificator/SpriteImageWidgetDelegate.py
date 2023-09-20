#! /usr/bin/env python
#
# Copyright (C) 2018 Michel Iwaniec
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
from PySide2.QtWidgets import QLabel, QGroupBox, QHBoxLayout, QStyledItemDelegate
from PySide2.QtCore import Qt, QSize
from PySide2.QtGui import QPixmap, QImage

from tilificator.sprite import SpriteImage
from tilificator.rectangles_widget import RectanglesWidget
from tilificator.SpriteImageWidget import SpriteImageWidget


class SpriteImageWidgetDelegate(QStyledItemDelegate):
    """
    Delegate to render a sprite image in a QListWidget/QListView
    
    Only handles persistent editor mode at this point in time.
    """

    def __init__(self, parent=None):
        super(SpriteImageWidgetDelegate, self).__init__(parent)
        self.siwDict = dict()

    def createEditor(self, parent, option, index):
        siw = SpriteImageWidget(None)
        siw.setParent(parent)
        return siw

    def setEditorData(self, siw, index):
        spriteImage = index.data()
        siw.setSpriteImage(spriteImage)
        siw.spriteTilesUpdated()
        self.siwDict[spriteImage] = siw

    def getEditor(self, spriteImage):
        """
        Returns the "editor" (SpriteImageWidget) associated with a particular SpriteImage
        """
        return self.siwDict[spriteImage]

    def sizeHint(self, option, index):
        spriteImage = index.data()
        if spriteImage in self.siwDict:
            siw = self.siwDict[spriteImage]
            return QSize(siw.width(), siw.height())
        else:
            return QSize(16, 16)
