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

from tilificator.sprite import SpriteImage


class SpriteTile(object):
    def __init__(self, x=0, y=0, w=8, h=8, tileID=0, flipH=False, flipV=False, colorDataHi=0):
        self.x = x
        self.y = y
        self.w = w
        self.h = h
        self.tileID = tileID
        self.flipH = flipH
        self.flipV = flipV
        self.colorDataHi = colorDataHi
        self.fixed = False


class Tilification(object):
    def __init__(self):
        self.numTilesAdded = 0
        self.maxSpritesScanline = 0
        self.maxSpritesScanlineBeyondLimit = 0
        self.avgSpritesScanline = 0
        self.avgSpritesScanlineBeyondLimit = 0
        self.tiles = []


class OptimizationSettings(object):
    def __init__(self):
        self.tilingMethod = 'ShiftedRowsNoOverlap'
        self.priorities = ['Number of tile patterns',
                           'Maximum sprites/scanline beyond limit',
                           'Average sprites/scanline beyond limit',
                           'Maximum sprites/scanline',
                           'Average sprites/scanline',
                           'Number of sprites',
                           'Global optimization estimate']
        self.spritesScanlineMaxLimit = 3
        self.spritesScanlineAvgLimit = 3
        self.useGlobalOptimization = False
        self.solidFactorExponent = 0.3

# - Allow a multitude of different optimization goals, and prioritizing them differently. Typical ones would be:
#   - Number of total tiles in Tile Table. In reality, this will need to be approximated by "number of tiles ADDED for this sprite" - which may not always coincide.
#   - MAX sprites/scanline calculated for entire sprite.
#   - MAX sprites/scanline _above a certain treshold_ calculated for entire sprite. i.e., determine a reasonable sprite width up to which there is no optimization penalty.
#   - AVERAGE number of sprites/scanline.
#   - AVERAGE number of sprites/scanline _above a certain treshold_.
#   - Number of total 8x8 hardware sprites in sprite.


def costOfTilification(tilification, spriteImage, optimizationSettings, numberOfTilesAdded=0, globalOptimization=0.0):
    if spriteImage.previousTilification != []:
        tilification = tilification[:]
        tilification.extend(spriteImage.previousTilification)

    height = max([st.y + st.h for st in tilification]) - min([st.y for st in tilification])
    spritesScanline = [0] * height

    for st in tilification:
        for i in range(st.h):
            if st.y + i >= 0 and st.y + i < height:
                spritesScanline[st.y + i] += 1

    numSprites = len(tilification)

    #print "sum(spritesScanline) = %d, height = %d" % (sum(spritesScanline), height)

    return costOfScanlines(spriteImage, optimizationSettings, spritesScanline, numSprites, numberOfTilesAdded, globalOptimization)


def costOfScanlines(spriteImage, optimizationSettings, spritesScanline, numSprites, numberOfTilesAdded, globalOptimization=0.0):
    spritesScanlineMax = max(spritesScanline)
    spritesScanlineAvg = float(sum(spritesScanline)) / float(spriteImage.height)
    spritesScanlineMaxBeyondLimit = max(0, spritesScanlineMax - optimizationSettings.spritesScanlineMaxLimit)
    spritesScanlineAvgBeyondLimit = max(0, spritesScanlineAvg - optimizationSettings.spritesScanlineAvgLimit)

    #numberOfTilesAdded = 0

    measures = {'Maximum sprites/scanline': spritesScanlineMax,
                'Average sprites/scanline': spritesScanlineAvg,
                'Maximum sprites/scanline beyond limit': spritesScanlineMaxBeyondLimit,
                'Average sprites/scanline beyond limit': spritesScanlineAvgBeyondLimit,
                'Number of tile patterns': numberOfTilesAdded,
                'Number of sprites': numSprites,
                'Global optimization estimate': (-globalOptimization if optimizationSettings.useGlobalOptimization else 0.0)}

    #cost += OptimizationSettings.cNumSprites*numSprites
    #cost += OptimizationSettings.cSpritesScanlineMax*spritesScanlineMax
    #cost += OptimizationSettings.cSpritesScanlineAvg*spritesScanlineAvg
    #cost += OptimizationSettings.cSpritesScanlineMaxBeyondLimit*spritesScanlineMaxBeyondLimit
    #cost += OptimizationSettings.cSpritesScanlineAvgBeyondLimit*spritesScanlineAvgBeyondLimit
    #cost += OptimizationSettings.cNumberOfTilesAdded*numberOfTilesAdded

    cost = 0
    for i, priority in enumerate(reversed(optimizationSettings.priorities)):
        cost += (100.0**i) * measures[priority]

    return cost


def coverageFromTilification(coverage, tilification, si, tileTable, binary=False):
    tileTable.coverageFromTilification(coverage, tilification, si, binary)
    return coverage


def spriteTileRedundant(st, si, coverage, tileTable):
    tx = st.x
    ty = st.y
    tID = st.tileID
    flipH = st.flipH
    flipV = st.flipV
    for y in range(st.h):
        for x in range(st.w):
            tilePixelInsideImage = (0 <= tx + x < si.width) and (0 <= ty + y < si.height)
            if not flipH and not flipV:
                cTile = tileTable.data[tID * st.w * st.h + st.w * y + x]
            elif flipH and not flipV:
                cTile = tileTable.data[tID * st.w * st.h + st.w * y + st.w - 1 - x]
            elif not flipH and flipV:
                cTile = tileTable.data[tID * st.w * st.h + st.w * (st.h - 1 - y) + x]
            elif flipH and flipV:
                cTile = tileTable.data[tID * st.w * st.h + st.w * (st.h - 1 - y) + st.w - 1 - x]
            if tilePixelInsideImage and cTile != 0 and coverage[(ty + y) * si.width + tx + x] == 0:
                return False
    return True


def spriteTileHasUnsharedPixels(st, si, spriteCoverage, tileTable):
    tx = st.x
    ty = st.y
    tID = st.tileID
    flipH = st.flipH
    flipV = st.flipV
    for y in range(st.h):
        for x in range(st.w):
            tilePixelInsideImage = (0 <= tx + x < si.width) and (0 <= ty + y < si.height)
            if not flipH and not flipV:
                cTile = tileTable.data[tID * st.w * st.h + st.w * y + x]
            elif flipH and not flipV:
                cTile = tileTable.data[tID * st.w * st.h + st.w * y + st.w - 1 - x]
            elif not flipH and flipV:
                cTile = tileTable.data[tID * st.w * st.h + st.w * (st.h - 1 - y) + x]
            elif flipH and flipV:
                cTile = tileTable.data[tID * st.w * st.h + st.w * (st.h - 1 - y) + st.w - 1 - x]
            if tilePixelInsideImage and cTile != 0 and si.data[(ty + y) * si.width + tx + x] == cTile and spriteCoverage[(ty + y) * si.width + tx + x] == 1:
                return True
    return False


def coverageFromImage(coverage, si):
    for y in range(si.height):
        for x in range(si.width):
            coverage[y * si.width + x] = 1 if si.data[y * si.width + x] != 0 else 0


def tilificationHasFullCoverage(si, spriteTiles, tileTable):
    imageCoverage = array.array('B', [0] * si.width * si.height)
    coverageFromImage(imageCoverage, si)
    tilesCoverage = array.array('B', [0] * si.width * si.height)
    coverageFromTilification(tilesCoverage, spriteTiles, si, tileTable, binary=True)
    return imageCoverage == tilesCoverage
