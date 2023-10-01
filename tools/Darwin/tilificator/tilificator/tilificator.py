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

import copy
import sys
import itertools
import operator
import functools

from tilificator.array2d import array2d
from tilificator.sprite import SpriteImage
from tilificator.spritetile import SpriteTile, OptimizationSettings, Tilification

from tilificator.tilingmethod import TilingMethod
from tilificator.tilingmethodrect import TilingMethodRect
from tilificator.tilingmethodshiftedrows import TilingMethodShiftedRows
from tilificator.tilingmethodshiftedrowsnooverlap import TilingMethodShiftedRowsNoOverlap
from tilificator.tilingmethoddragqueen import TilingMethodDragQueen
from tilificator.tilingmethodfullmatch import TilingMethodFullMatch


def getTilingMethod(optimizationSettings):
    methods = {'Rect': TilingMethodRect,
               'ShiftedRows': TilingMethodShiftedRows,
               'ShiftedRowsNoOverlap': TilingMethodShiftedRowsNoOverlap,
               'DragQueen': TilingMethodDragQueen,
               'FullMatch': TilingMethodFullMatch}

    return methods[optimizationSettings.tilingMethod]


def splitLayeredImage(spriteImage, tileSettings):
    """
    Splits an image based on number of colors supported by tiletable
    """
    maximumColor = max(spriteImage.data)
    colorSize = tileSettings.colorSize
    numSplitImages = (maximumColor + colorSize - 1) // colorSize
    splitImages = []
    for i in range(0, numSplitImages):
        si = copy.deepcopy(spriteImage)
        for y in range(si.height):
            for x in range(si.width):
                c = si.data[y * si.width + x]
                si.data[y * si.width + x] = c % colorSize if i * colorSize < c < (i + 1) * colorSize else 0
        if not si.blank():
            si.crop()
            colorDataHi = i * colorSize
            si.colorDataHi = colorDataHi
            splitImages.append(si)
    # Sort depending on image size (largest first)
    splitImages = sorted(splitImages, key=lambda si: si.width * si.height, reverse=True)
    return splitImages


def mergeTilifications(spriteImages):
    """
    Merges the tilifications from a list of images previously created by splitLayeredImage into a single tilification.
    """
    tilification = Tilification()
    for si in spriteImages:
        for st in si.tilification.tiles:
            tilification.tiles.append(SpriteTile(x=st.x + si.offsX,
                                                 y=st.y + si.offsY,
                                                 w=st.w,
                                                 h=st.h,
                                                 tileID=st.tileID,
                                                 flipH=st.flipH,
                                                 flipV=st.flipV,
                                                 colorDataHi=si.colorDataHi))
    return tilification


def tilifySpriteImageWithSplit(spriteImage, tileTable, optimizationSettings=OptimizationSettings()):
    spriteImage.tilification = Tilification()
    splitImages = splitLayeredImage(spriteImage, tileTable.settings)
    if splitImages:
        splitImages[0].previousTilification = []
    for i, si in enumerate(splitImages):
        xOffs, yOffs = si.offsX, si.offsY
        tilingMethod = getTilingMethod(optimizationSettings)
        tilingMethod = tilingMethod(si, tileTable, tileTable.settings, optimizationSettings)
        for progress in tilingMethod.tilify():
            if i == 0:
                yield progress
            else:
                yield 1.0
        for st in si.tilification.tiles:
            st.colorDataHi = si.colorDataHi
            st.x += xOffs
            st.y += yOffs
        if i < len(splitImages) - 1:
            splitImages[i + 1].previousTilification = si.tilification.tiles
            splitImages[i + 1].previousTilification.extend(si.previousTilification)
        else:
            spriteImage.tilification = si.tilification
            spriteImage.tilification.tiles.extend(si.previousTilification)


def tilifySpriteImage(spriteImage, tileTable, optimizationSettings=OptimizationSettings()):
    spriteImage.tilification = Tilification()
    tilingMethod = getTilingMethod(optimizationSettings)
    tilingMethod = tilingMethod(spriteImage, tileTable, tileTable.settings, optimizationSettings)
    for progress in tilingMethod.tilify():
        yield progress


def generateSpriteImageLookups(tileTable, spriteImage):
    """
    Generates a number of lookup-tables used for quickly looking up data about the image, as well as for generating 
    other lookup tables in some of the tiling methods.
    """
    spriteImage.hashImage = TilingMethod.makeHashImage(tileTable, spriteImage, flipH=False, flipV=False)
    spriteImage.hashImageH = TilingMethod.makeHashImage(tileTable, spriteImage, flipH=True, flipV=False)
    spriteImage.hashImageV = TilingMethod.makeHashImage(tileTable, spriteImage, flipH=False, flipV=True)
    spriteImage.hashImageHV = TilingMethod.makeHashImage(tileTable, spriteImage, flipH=True, flipV=True)
    spriteImage.tileCache = TilingMethod.makeTileCache(tileTable, spriteImage, flipH=False, flipV=False)
    spriteImage.tileCacheH = TilingMethod.makeTileCache(tileTable, spriteImage, flipH=True, flipV=False)
    spriteImage.tileCacheV = TilingMethod.makeTileCache(tileTable, spriteImage, flipH=False, flipV=True)
    spriteImage.tileCacheHV = TilingMethod.makeTileCache(tileTable, spriteImage, flipH=True, flipV=True)
    spriteImage.tileNumSolid = TilingMethod.makeTileNumSolid(spriteImage)
    spriteImage.hashDict = TilingMethod.makeHashDict(tileTable, spriteImage)


def generateMatchImageDict(tileTable, spriteImages):
    """
    For a list of sprite images, creates a dictionary with keys (indexA, indexB), and the value being the
    match image of matches found in spriteImages[indexB], for each (x,y) in spriteImages[indexA]
    """
    matchImageDict = dict()
    for siIndexA, siIndexB in itertools.product(range(len(spriteImages)), range(len(spriteImages))):
        matchImageDict[siIndexA, siIndexB] = TilingMethod.makeMatchImage(tileTable, spriteImages[siIndexA], spriteImages[siIndexB])
        yield matchImageDict


def generateGlobalMatchImage(tileTable, spriteImages, matchImageDict, indexDst, indicesSrc, solidFactorExponent):
    """
    Takes a list of spriteImages, a previously generated matchImageDict, indexDst, and a sequence of indices indicesSrc
    
    Returns a new floating-point image with all the source-match-images from the matchImageDict added, and transformed
    according to the following expression:
    (NumberOfGlobalMatches / TotalImages) * (NumSolidPixelsInTile / (tileWidth*tileHeight))^solidFactorExponent
    """
    tileNumSolid = spriteImages[indexDst].tileNumSolid
    gmi = array2d('d', tileNumSolid.xRange, tileNumSolid.yRange, 0.0)
    if indicesSrc:
        gmSum = functools.reduce(operator.add, [matchImageDict[indexDst, i] for i in indicesSrc])
        for y in gmi.yRange:
            for x in gmi.xRange:
                tileWidth = tileTable.settings.tileWidth
                tileHeight = tileTable.settings.tileHeight
                solidFactor = tileNumSolid[x, y] / (tileWidth * tileHeight)
                numImages = len(indicesSrc)
                gmi[x, y] = (gmSum[x, y] / numImages) * pow(solidFactor, solidFactorExponent)
    return gmi
