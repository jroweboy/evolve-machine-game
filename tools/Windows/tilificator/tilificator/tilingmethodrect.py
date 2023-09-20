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

import itertools
from math import ceil

from tilificator.tilingmethod import *
from tilificator.spritetile import *


class TilingMethodRect(TilingMethod):
    def __init__(self, si, tileTable, settings, optimizationSettings):
        super(TilingMethodRect, self).__init__(si, tileTable, settings, optimizationSettings)

    def tilify(self):
        t = Tilification()
        self.si.tilification = t
        t.tiles = []
        bestShiftX = 0
        bestShiftY = 0
        bestCost = float('inf')
        for shiftY in range(0, self.settings.tileHeight):
            for shiftX in range(0, self.settings.tileWidth):
                numTilesPrev = self.tileTable.numTiles
                tilification, gmi = self.getRectTilification(shiftX, shiftY)
                cost = costOfTilification(tilification=tilification,
                                          spriteImage=self.si,
                                          optimizationSettings=self.optimizationSettings,
                                          numberOfTilesAdded=self.tileTable.numTiles - numTilesPrev,
                                          globalOptimization=gmi)
                if cost < bestCost:
                    bestCost = cost
                    bestShiftX = shiftX
                    bestShiftY = shiftY
                self.tileTable.removeTiles(self.tileTable.numTiles - numTilesPrev)
                yield float(shiftY) / float(self.settings.tileHeight)
        t.tiles, _ = self.getRectTilification(bestShiftX, bestShiftY)

    def getRectTilification(self, shiftX, shiftY):
        maxTilesW = int(ceil(float(self.si.width) / float(self.settings.tileWidth))) + 1
        maxTilesH = int(ceil(float(self.si.height) / float(self.settings.tileHeight))) + 1
        t = []
        gmi = 0
        for ty in range(maxTilesH):
            for tx in range(maxTilesW):
                x = self.settings.tileWidth * tx - shiftX
                y = self.settings.tileHeight * ty - shiftY
                if not self.tileTable.blankTile(self.si, x, y):
                    tileData=self.tileTable.cutTile(self.si, x, y)
                    tileID, flipH, flipV=self.tileTable.findTile(tileData)
                    if tileID < 0:
                        tileID=self.tileTable.numTiles
                        self.tileTable.addTile(tileData)
                        if self.optimizationSettings.useGlobalOptimization:
                            gmi += self.si.globalMatches[x, y]
                    t.append(SpriteTile(x=x,
                                        y=y,
                                        w=self.settings.tileWidth,
                                        h=self.settings.tileHeight,
                                        tileID=tileID,
                                        flipH=flipH,
                                        flipV=flipV))
        return t, gmi
