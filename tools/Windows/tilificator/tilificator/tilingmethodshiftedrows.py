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

from tilificator.common import indexOfMinimum
from tilificator.tilingmethod import *
from tilificator.spritetile import *


class TilingMethodShiftedRows(TilingMethod):
    def __init__(self, si, tileTable, settings, optimizationSettings):
        super(TilingMethodShiftedRows, self).__init__(si, tileTable, settings, optimizationSettings)

    def tilify(self):
        t = Tilification()
        self.si.tilification = t
        t.tiles = []
        # For each line, compute first and last pixel
        lineBegin = []
        lineEnd = []
        for y in range(-(self.settings.tileHeight - 1), self.si.height):
            minX = self.si.width - 1
            maxX = 0
            if y >= 0:
                for x in range(self.si.width):
                    if self.si.data[y * self.si.width + x] != 0:
                        minX = min(x, minX)
                        maxX = max(x, maxX)
            lineBegin.append(minX)
            lineEnd.append(maxX)

        costs = [None] * self.settings.tileHeight
        # Start lowest sprite from feet: sY = (self.settings.tileHeight-(self.si.height%self.settings.tileHeight))%self.settings.tileHeight
        for shiftY in range(0, self.settings.tileHeight):  # range(sY, sY+1):
            numTilesPrev = self.tileTable.numTiles
            tilification, globalOptimization = self.getShiftedRowsTilification(lineBegin, lineEnd, shiftY)
            costs[shiftY] = costOfTilification(tilification=tilification,
                                               spriteImage=self.si,
                                               optimizationSettings=self.optimizationSettings,
                                               numberOfTilesAdded=self.tileTable.numTiles - numTilesPrev,
                                               globalOptimization=globalOptimization)
            self.tileTable.removeTiles(self.tileTable.numTiles - numTilesPrev)
            yield float(shiftY) / float(self.settings.tileHeight)

        if indexOfMinimum(costs) is not None:
            t.tiles, _ = self.getShiftedRowsTilification(lineBegin, lineEnd, indexOfMinimum(costs))

    def getShiftedRowsTilification(self, lineBegin, lineEnd, shiftY):
        maxTilesW = int(ceil(float(self.si.width) / float(self.settings.tileWidth)))
        maxTilesH = int(ceil(float(self.si.height) / float(self.settings.tileHeight)))

        if maxTilesH * self.settings.tileHeight - shiftY + self.settings.tileHeight - 1 < len(lineBegin):
            maxTilesH += 1

        t = []

        globalOptimizationFinal = 0

        numCombinations = [0] * maxTilesH
        for ty in range(maxTilesH):
            y = ty * self.settings.tileHeight - shiftY
            sy = ty * self.settings.tileHeight - shiftY + self.settings.tileHeight - 1
            croppedLineBegin = lineBegin[sy:sy + self.settings.tileHeight]
            tileBegin = min(croppedLineBegin)
            croppedLineEnd = lineEnd[sy:sy + self.settings.tileHeight]
            tileEnd = max(croppedLineEnd)
            HTiles = int(ceil(float(tileEnd + 1 - tileBegin) / float(self.settings.tileWidth)))
            shiftFreedomH = HTiles * self.settings.tileWidth - (tileEnd + 1 - tileBegin)

            # Check every shift possibility before definitely adding the tiles
            numTilesPrev = self.tileTable.numTiles
            xcoordCombinations = generateTileXCoords(tileEnd + 1 - tileBegin, self.settings.tileWidth, tileBegin)
            tilesAdded = [0] * len(xcoordCombinations)

            numCombinations[ty] = len(xcoordCombinations)

            # Reverse to prioritize less overlap between newly added tiles and previous ones - gives much worse results for Megaman sprites
            # xcoordCombinations.reverse()

            for i, xcoords in enumerate(xcoordCombinations):
                globalOptimization = 0
                for x in xcoords:
                    # Check if tile contains non-transparent pixels
                    if not self.tileTable.blankTile(self.si, x, y):
                        tileData = self.tileTable.cutTile(self.si, x, y)
                        tileID, flipH, flipV = self.tileTable.findTile(tileData)
                        if tileID < 0:
                            self.tileTable.addTile(tileData)
                            if self.optimizationSettings.useGlobalOptimization:
                                globalOptimization += self.si.globalMatches[x, y]

                # Estimate each future global match as one thousands of an actually matched tile
                tilesAdded[i] = self.tileTable.numTiles - numTilesPrev - globalOptimization * 0.001

                #tileTable.numTiles = numTilesPrev
                self.tileTable.removeTiles(self.tileTable.numTiles - numTilesPrev)

            # Obtain optimal shift and do definite tiling
            s = tilesAdded.index(min(tilesAdded))
            xOffset = tileBegin - s

            for x in xcoordCombinations[s]:
                # Check if tile contains non-transparent pixels
                if not self.tileTable.blankTile(self.si, x, y):
                    tileData = self.tileTable.cutTile(self.si, x, y)
                    tileID, flipH, flipV = self.tileTable.findTile(tileData)
                    if tileID < 0:
                        tileID = self.tileTable.numTiles
                        self.tileTable.addTile(tileData)
                        if self.optimizationSettings.useGlobalOptimization:
                            globalOptimizationFinal += self.si.globalMatches[x, y]

                    t.append(SpriteTile(x=x,
                                        y=y,
                                        w=self.settings.tileWidth,
                                        h=self.settings.tileHeight,
                                        tileID=tileID,
                                        flipH=flipH,
                                        flipV=flipV))

        #print ("  " + str(numCombinations) + " : %d => %d (%dx)" % (sum(numCombinations), product(numCombinations), product(numCombinations)/sum(numCombinations)))

        return t, globalOptimizationFinal


#
# Returns all possible combinations of x coordinates for tiles in a row where redundant extra space affects the possibilities. i.e., given this input
#
# n =
# x0, x1 ... xn : The x coordinates of the n tiles required to cover a "width x TileHeight" row
#
# Return list with all possible combinations of (x0, x1, ... xn) tuples satisfying the requirements:
# (1) -(L+1) < x0 < x1 < ... < xn < width
# (2) x0+TileWidth+1 > x1, x1+TileWidth+1 > x2 ... xn_1+TileWidth+1 > xn
#
def generateTileX(width, tileWidth, xOffs=0):
    n = int((width - 1) / tileWidth)

    if width <= tileWidth:  # n == 1:
        return list(range(-tileWidth + width + xOffs, 1 + xOffs))
    else:
        combinations = []
        for x0 in range(-tileWidth + (width - n * tileWidth), 1):
            nextRange = list(itertools.product([x0 + xOffs], generateTileX(width - (x0 + tileWidth), tileWidth, x0 + tileWidth + xOffs)))
            if n >= 2:
                nextRange = [list(itertools.chain([sublist[0]], sublist[1])) for sublist in nextRange]
            combinations.extend(nextRange)
        return combinations


def generateTileXCoords(width, tileWidth, xOffs=0):
    xcoords = generateTileX(width, tileWidth, xOffs)
    if width <= tileWidth:
        return [[xcoord] for xcoord in xcoords]
    else:
        return xcoords
