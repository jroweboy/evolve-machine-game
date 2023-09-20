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
import array
from functools import reduce

from PIL import Image

from tilificator.array2d import array2d


class SpriteImage(object):
    def __init__(self, filename=None):
        Image.init()
        self.offsX = 0
        self.offsY = 0
        self.flipAdjustX = 0
        if filename is not None:
            im = Image.open(filename)
            self.width, self.height = im.size
            self.data = array2d('B', self.width, self.height, im.getdata())
            self.palette = array.array('B', im.getpalette())
            self.filename = filename
        else:
            self.width = 0
            self.height = 0
            self.data = None
            self.palette = None
            self.filename = None

    def save(self, filename):
        im = Image.frombuffer('P', (self.width, self.height), self.data, 'raw', 'P', 0, 1)
        im.putpalette(self.palette)
        im.save(filename)

    def crop(self):
        minX = self.width - 1
        minY = self.height - 1
        maxX = 0
        maxY = 0
        for y in range(self.height):
            for x in range(self.width):
                if self.data[y * self.width + x] != 0:
                    minX = min(x, minX)
                    minY = min(y, minY)
                    maxX = max(x, maxX)
                    maxY = max(y, maxY)

        newWidth = maxX - minX + 1
        newHeight = maxY - minY + 1
        newData = array2d('B', newWidth, newHeight, 0)
        for y in range(newHeight):
            for x in range(newWidth):
                newData[y * newWidth + x] = self.data[(y + minY) * self.width + minX + x]
        self.width = newWidth
        self.height = newHeight
        self.data = newData
        self.offsX = minX
        self.offsY = minY
        return self.offsX, self.offsY

    def pad(self, leftPadding, topPadding, rightPadding, bottomPadding):
        newWidth = leftPadding + self.width + rightPadding
        newHeight = topPadding + self.height + bottomPadding
        newData = array2d('B', newWidth, newHeight, 0)
        for y in range(self.height):
            for x in range(self.width):
                newData[(y + topPadding) * newWidth + leftPadding + x] = self.data[y * self.width + x]

        self.width = newWidth
        self.height = newHeight
        self.data = newData

    def mask(self, bitMask):
        for y in range(self.height):
            for x in range(self.width):
                self.data[y * self.width + x] &= bitMask

    def blank(self):
        return sum(self.data) == 0

    def printData(self, data=None):
        if data is None:
            data = self.data
        for y in range(self.height):
            for x in range(self.width):
                sys.stdout.write("%d," % data[y * self.width + x])
            sys.stdout.write("\n")


class SpriteAnimation:
    def __init__(self, filename=None):
        self.frameRefs = []
        self.frameDelays = []
        self.totalFrames = 0
        if filename is None:
            self.loadImages(filename)

    def loadImages(self, prefix):
        self.frameRefs = []
        self.frameDelays = []

        sprList = open("../sprite_animations/" + prefix + "/" + prefix + ".txt", "r").read().split()

        self.offsX = int(sprList[0])
        self.offsY = int(sprList[1])
        self.flipAdjustX = int(sprList[2])

        delaysStr = sprList[3:]
        if len(delaysStr) > 1:
            self.totalFrames = reduce(lambda x, y: int(x) + int(y), delaysStr)
        else:
            self.totalFrames = int(delaysStr[0])

        for i, delay in enumerate(delaysStr):
            fn = "../sprite_animations/" + prefix + "/" + prefix + str(i) + ".png"
            ref = SpriteImage(fn)
            ref.offsX = self.offsX
            ref.offsY = self.offsY
            ref.flipAdjustX = self.flipAdjustX
            self.frameRefs.append(ref)
            self.frameDelays.append(int(delay))

    def getSpriteImage(self, frame):
        time = 0
        frame = frame % self.totalFrames
        for i in range(len(self.frameRefs)):
            time += self.frameDelays[i]
            if time > frame:
                return self.frameRefs[i]

        return self.frameRefs[-1]
