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

from PIL import Image

from PySide2.QtGui import QPixmap, QImage
from PySide2.QtWidgets import QLabel
from PySide2.QtCore import Qt, QSize


class PalettedImageWidget(QLabel):
    def __init__(self, image, transparent=[]):
        super(PalettedImageWidget, self).__init__()
        self.zoom = 2
        self.image = image
        self.transparent = transparent
        self.padLeft = 0
        self.padRight = 0
        self.padTop = 0
        self.padBottom = 0
        self._cachedPaddingAndZoom = (self.padLeft, self.padRight, self.padTop, self.padBottom, self.zoom)
        if self.image is not None:
            self.imageChanged()

    def imageChanged(self):
        if self.image is not None:
            self._cachedPaddingAndZoom = (self.padLeft, self.padRight, self.padTop, self.padBottom, self.zoom)
            self.palette = self.image.getpalette()
            self.imageWidth, self.imageHeight = self.image.size
            self.data = self.image.getdata()
            # Convert image to QPixmap
            paddedWidth = self.padLeft + self.imageWidth + self.padRight
            paddedHeight = self.padTop + self.imageHeight + self.padBottom
            self.argbData = array.array('B', [0] * paddedWidth * paddedHeight * 4)
            for y in range(self.imageHeight):
                for x in range(self.imageWidth):
                    srcOffs = self.imageWidth * y + x
                    dstOffs = paddedWidth * (y + self.padTop) + self.padLeft + x
                    c = self.data[srcOffs]
                    if c not in self.transparent:
                        self.argbData[4 * dstOffs + 0] = self.palette[3 * c + 2]
                        self.argbData[4 * dstOffs + 1] = self.palette[3 * c + 1]
                        self.argbData[4 * dstOffs + 2] = self.palette[3 * c + 0]
                        self.argbData[4 * dstOffs + 3] = 255
                    else:
                        self.argbData[4 * dstOffs + 0] = 0
                        self.argbData[4 * dstOffs + 1] = 0
                        self.argbData[4 * dstOffs + 2] = 0
                        self.argbData[4 * dstOffs + 3] = 0
            qImage = QImage(bytes(self.argbData),
                            paddedWidth,
                            paddedHeight,
                            QImage.Format_ARGB32_Premultiplied)
            pixmap = QPixmap.fromImage(qImage)
            pixmap.setDevicePixelRatio(1 / self.zoom)
            self.setPixmap(pixmap)
            self.setFixedWidth(self.zoom * paddedWidth)
            self.setFixedHeight(self.zoom * paddedHeight)

    def paintEvent(self, event):
        paddingAndZoom = (self.padLeft, self.padRight, self.padTop, self.padBottom, self.zoom)
        if paddingAndZoom != self._cachedPaddingAndZoom:
            self.imageChanged()
        super(PalettedImageWidget, self).paintEvent(event)

    def width(self):
        if self.image:
            imageWidth, _ = self.image.size
            paddedWidth = self.padLeft + imageWidth + self.padRight
            return paddedWidth * self.zoom
        else:
            return 1000

    def height(self):
        if self.image:
            _, imageHeight = self.image.size
            paddedHeight = self.padTop + imageHeight + self.padBottom
            return paddedHeight * self.zoom
        else:
            return 1

    def sizeHint(self):
        return QSize(self.width(), self.height())
