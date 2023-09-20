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

from collections import defaultdict
import array
from tilificator.array2d import array2d

from PIL import Image

from tilificator.spritetile import SpriteTile
from tilificator.tile import TileTable


class TilingMethod(object):
    def __init__(self, si, tileTable, settings, optimizationSettings):
        self.tileTable = tileTable
        self.settings = settings
        self.optimizationSettings = optimizationSettings
        self.si = si

    @staticmethod
    def makeHashImage(tileTable, si, flipH, flipV):
        """
        Creates a hash-image of a sprite image which, for each pixel at (x,y), contains the 16-bit hash value of the
        tile with upper-left corner at (x,y), or -1 for blank tiles.
        flipH / flipV allow creating a hash-image with the hash at (x,y) being from a horizontally / vertically 
        flipped version of the tile.
        Also creates a dictionary lookup for all the hashes 
        """
        padX = tileTable.settings.tileWidth - 1
        padY = tileTable.settings.tileHeight - 1
        hashImage = array2d('H', range(-padX, si.width + padX), range(-padY, si.height + padY), 0)
        for y in hashImage.yRange:
            for x in hashImage.xRange:
                if not tileTable.blankTile(si, x, y):
                    data = tileTable.cutTile(si, x, y, flipH, flipV)
                    h = tileTable.hashTile(data)
                    hashImage[x, y] = h
                else:
                    hashImage[x, y] = tileTable.BLANK_HASH_VALUE
        return hashImage

    @staticmethod
    def makeHashDict(tileTable, si):
        """
        For a given spriteImage with hashimages, creates a dict for looking up whether a given tile hash exists in the image.
        Dict contains binary encoding of flipping variants for each hash key, as such
        """
        d = defaultdict(list)
        for y in si.hashImage.yRange:
            for x in si.hashImage.xRange:
                h = si.hashImage[x, y]
                hH = si.hashImageH[x, y]
                hV = si.hashImageV[x, y]
                hHV = si.hashImageHV[x, y]
                if h != tileTable.BLANK_HASH_VALUE or not tileTable.blankTile(si, x, y):
                    d[h].append((x, y, False, False))
                if hH != tileTable.BLANK_HASH_VALUE or not tileTable.blankTile(si, x, y):
                    d[hH].append((x, y, True, False))
                if hV != tileTable.BLANK_HASH_VALUE or not tileTable.blankTile(si, x, y):
                    d[hV].append((x, y, False, True))
                if hHV != tileTable.BLANK_HASH_VALUE or not tileTable.blankTile(si, x, y):
                    d[hHV].append((x, y, True, True))
        return d

    @staticmethod
    def makeTileCache(tileTable, spriteImage, flipH, flipV):
        """
        For a given spriteImage, traverses each (x,y) position and cuts the tile with origin at (x,y),
        storing this in a cached structure, with one tile for each pixel in the sprite image.
        flipH / flipV allow creating a hash-image with the hash at (x,y) being from a horizontally / vertically 
        flipped version of the tile.
        """
        tileCache = defaultdict(list)
        for y in spriteImage.hashImage.yRange:
            for x in spriteImage.hashImage.xRange:
                tileCache[x, y] = tileTable.cutTile(spriteImage, x, y, flipH, flipV)
        return tileCache

    @staticmethod
    def makeTileNumSolid(spriteImage):
        yRange = spriteImage.hashImage.yRange
        xRange = spriteImage.hashImage.xRange
        tileNumSolid = array2d('H', xRange, yRange, 0)
        for y in yRange:
            for x in xRange:
                tileData = spriteImage.tileCache[x, y]
                tileNumSolid[x, y] = len([t for t in tileData if t != 0])
        return tileNumSolid

    @staticmethod
    def makeMatchImage(tileTable, siA, siB):
        """
        For two given sprite images siA and siB, traverses each (x,y) position in siA and records all possible
        tile matches with siB, relying on their hashImage / hashDict to accelerate this process.
        """
        globalMatches = array2d('B', siA.hashImage.xRange, siA.hashImage.yRange, 0)
        for y in globalMatches.yRange:
            for x in globalMatches.xRange:
                if siA.tileNumSolid[x, y] > 0:
                    h = siA.hashImage[x, y]
                    for xx, yy, flipH, flipV in siB.hashDict[h]:
                        i = (2 * int(flipV) + int(flipH))
                        tileDataA = siA.tileCache[x, y]
                        tileDataB = [siB.tileCache[xx, yy],
                                      siB.tileCacheH[xx, yy],
                                      siB.tileCacheV[xx, yy],
                                      siB.tileCacheHV[xx, yy]][i]
                        if tileTable.compareTiles(tileDataA, tileDataB):
                            if globalMatches[x, y] < 255:
                                globalMatches[x, y] += 1
        return globalMatches

    def tilify(self):
        yield 1.0

    def addTiles(self, tileTable, si, tilification):
        numTilesOld = self.tileTable.numTiles
        globalOptimization = 0
        for st in tilification:
            tileData = self.tileTable.cutTile(si, st.x, st.y)
            tileID, flipH, flipV = self.tileTable.findTile(tileData)
            if tileID < 0:
                tileID = self.tileTable.numTiles  # -1
                self.tileTable.addTile(tileData)
                if self.optimizationSettings.useGlobalOptimization:
                    globalOptimization += si.globalMatches[st.x, st.y]
            st.tileID = tileID
            st.flipH = flipH
            st.flipV = flipV
        return self.tileTable.numTiles - numTilesOld, globalOptimization
