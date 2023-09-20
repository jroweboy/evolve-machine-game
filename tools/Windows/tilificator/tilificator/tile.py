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
import c_tile

from tilificator.spritetile import SpriteTile


class TileSettings(object):
    def __init__(self):
        self.tileWidth = 8
        self.tileHeight = 8
        self.colorSize = 256


class TileTable(object):
    def __init__(self, settings):
        self.settings = settings
        self.MAX_TILES = 8192
        self.numTiles = 0
        self.data = array.array('B', [0] * self.MAX_TILES * self.settings.tileWidth * self.settings.tileHeight)
        self.BLANK_HASH_VALUE = c_tile.hashTile(array.array('B', [0] * self.settings.tileWidth * self.settings.tileHeight),
                                                self.settings.tileWidth * self.settings.tileHeight)

    def hashTile(self, tileData):
        return c_tile.hashTile(tileData, self.settings.tileWidth * self.settings.tileHeight)

    def blankTile(self, si, xOffs, yOffs):
        return c_tile.blankTile(si.data, si.width, si.height, xOffs, yOffs, self.settings.tileWidth, self.settings.tileHeight)

    def cutTile(self, si, xOffs, yOffs, flipH=False, flipV=False):
        data = array.array('B', [0] * self.settings.tileWidth * self.settings.tileHeight)
        c_tile.cutTile(si.data, si.width, si.height, xOffs, yOffs, data, self.settings.tileWidth, self.settings.tileHeight, 1 if flipH else 0, 1 if flipV else 0)
        return data

    def clear(self):
        self.removeTiles(self.numTiles)

    def compareTiles(self, tileDataA, tileDataB):
        tileSize = self.settings.tileWidth * self.settings.tileHeight
        for i in range(tileSize):
            if tileDataA[i] != tileDataB[i]:
                return False
        return True

    def tilesEqual(self, i, tileData, flipH, flipV):
        for yy in range(self.settings.tileHeight):
            for xx in range(self.settings.tileWidth):
                if flipH:
                    x = self.settings.tileWidth - 1 - xx
                else:
                    x = xx
                if flipV:
                    y = self.settings.tileHeight - 1 - yy
                else:
                    y = yy
                if tileData[self.settings.tileWidth * y + x] != self.data[(i * self.settings.tileHeight + yy) * self.settings.tileWidth + xx]:
                    return False
        return True

    def removeTiles(self, numTilesToRemove=1):
        if numTilesToRemove > 0:
            c_tile.removeTiles(self.data, self.settings.tileWidth, self.settings.tileHeight, self.numTiles, numTilesToRemove)
        self.numTiles -= numTilesToRemove

    def findTile(self, tileData):
        return c_tile.findTile(tileData, self.data, self.numTiles, self.settings.tileWidth, self.settings.tileHeight, 1, 1)

    def getAllMatches(self, si, xStart, yStart, xEnd, yEnd):
        matchesDict = dict()
        paddedWidth = self.settings.tileWidth - 1 + si.width + self.settings.tileWidth - 1
        paddedHeight = self.settings.tileHeight - 1 + si.height + self.settings.tileHeight - 1
        matchesListList = c_tile.getAllMatches(xStart + self.settings.tileWidth - 1, yStart + self.settings.tileHeight - 1, xEnd + self.settings.tileWidth - 1, yEnd + self.settings.tileHeight - 1,
                                               si.dataPadded, si.hashImage, paddedWidth, paddedHeight,
                                               self.data, self.numTiles, self.settings.tileWidth, self.settings.tileHeight, 1, 1)
        for yAndList in matchesListList:
            y, matchesList = yAndList
            y -= self.settings.tileHeight - 1
            matchesDict[y] = [SpriteTile(x=st[0] - (self.settings.tileWidth - 1), y=st[1] - (self.settings.tileHeight - 1), tileID=st[2], flipH=st[3], flipV=st[4]) for st in matchesList]

        return matchesDict

    def matchTiles(self, si):
        imageCoverage = array.array('B', [0] * si.width * si.height)
        matched = c_tile.matchTiles(si.data, imageCoverage, si.width, si.height,
                                    self.data, self.numTiles, self.settings.tileWidth, self.settings.tileHeight,
                                    1, 1)
        return [[SpriteTile(x=st[0], y=st[1], tileID=st[2], flipH=st[3], flipV=st[4]) for st in matched], imageCoverage]

    def addTile(self, data):
        if self.numTiles < self.MAX_TILES:
            c_tile.addTile(data, self.data, self.numTiles, self.settings.tileWidth, self.settings.tileHeight)
            self.numTiles += 1
        else:
            print("Max tiles in tile table exceeded!")

    def save(self, filename):
        file = open(filename, 'wb')
        file.write(self.data[0:self.numTiles * self.settings.tileWidth * self.settings.tileHeight])
        file.close()

    def coverageFromTilification(self, coverage, tilification, si, binary):
        c_tile.coverageFromTilification(coverage, tilification, si.data, si.width, si.height, self.data, self.settings.tileWidth, self.settings.tileHeight, 1 if binary else 0)
