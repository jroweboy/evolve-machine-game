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

from collections import deque, defaultdict

import operator
from functools import reduce
import itertools
import bisect
from math import ceil

from tilificator.array2d import array2d
from tilificator.common import makeNext1, expandedProduct, indexOfMinimum, combinationsIncreasing, mergeSortedLists
from tilificator.spritetile import *

from tilificator.tilingmethod import *


class TilingMethodDragQueen(TilingMethod):

    def __init__(self, si, tileTable, settings, optimizationSettings):
        super(TilingMethodDragQueen, self).__init__(si, tileTable, settings, optimizationSettings)
        # TilingMethodDragQueen *requires* the hashImages - so generate them if they don't already exists
        if not hasattr(si, 'hashImage'):
            si.hashImage = self.makeHashImage(self.tileTable, si, flipH=False, flipV=False)
        if not hasattr(si, 'hashImageH'):
            si.hashImageH = self.makeHashImage(self.tileTable, si, flipH=True, flipV=False)
        if not hasattr(si, 'hashImageV'):
            si.hashImageV = self.makeHashImage(self.tileTable, si, flipH=False, flipV=True)
        if not hasattr(si, 'hashImageHV'):
            si.hashImageHV = self.makeHashImage(self.tileTable, si, flipH=True, flipV=True)
        self.makeSelfSimilarityImage(si)
        self.makeFirstAndLastNonBlankColumnTable(si)

    def makeSelfSimilarityImage(self, si):
        # Create dictionary of coordinates where hashes match
        sameHashDict = dict()
        for y in si.hashImage.yRange:
            for x in si.hashImage.xRange:
                hashFlipList = [(si.hashImage, False, False),
                                (si.hashImageH, True, False),
                                (si.hashImageV, False, True),
                                (si.hashImageHV, True, True)]
                for hashImage, flipH, flipV in hashFlipList:
                    h = hashImage[x, y]
                    if h in sameHashDict:
                        # Prevent same (x,y) coord from being added twice in case the tile is flip-symmetric
                        if not [coords for coords in sameHashDict[h] if(coords[0] == x and coords[1] == y)]:
                            sameHashDict[h].append((x, y, flipH, flipV))
                    else:
                        sameHashDict[h] = [(x, y, flipH, flipV)]

        # Keep only keys with more than one entry
        si.sameHashDict = dict()
        for h, matchesList in sameHashDict.items():
            if len(matchesList) > 1:
                si.sameHashDict[h] = matchesList

        # Create self similarity image for quickly accessing coordinates with identical tiledata
        si.selfSimilarityImage = defaultdict(list)
        for y in si.hashImage.yRange:
            for x in si.hashImage.xRange:
                h = si.hashImage[x, y]
                if h in si.sameHashDict:
                    if not self.tileTable.blankTile(si, x, y):
                        data = self.tileTable.cutTile(si, x, y)
                        for coords in si.sameHashDict[h]:
                            otherData = self.tileTable.cutTile(si, xOffs=coords[0], yOffs=coords[1], flipH=coords[2], flipV=coords[3])
                            if self.tileTable.compareTiles(data, otherData):
                                # Prevent same (x,y) coord from being added twice in case the tile is flip-symmetric
                                if not si.selfSimilarityImage[x, y] or not [coordsSSI for coordsSSI in si.selfSimilarityImage[x, y] if(coords[0] == coordsSSI[0] and coords[1] == coordsSSI[1])]:
                                    si.selfSimilarityImage[x, y].append(coords)
                # If just one match was appended, this is still equal to no match...
                if len(si.selfSimilarityImage[x, y]) < 2:
                    si.selfSimilarityImage[x, y] = []

        # Make the coordinates in selfSimilarityImage easy to find for each line
        si.selfSimilarityPerLine = defaultdict(list)
        for y in si.hashImage.yRange:
            for x in si.hashImage.xRange:
                if si.selfSimilarityImage[x, y]:
                    listOfMatches = [match for match in si.selfSimilarityImage[x, y] if match[0] != x and match[1] == y]
                    if listOfMatches != []:
                        si.selfSimilarityPerLine[y].append([x, y, listOfMatches])

    def getOptimizedTilificationFromColumns(self, columns, xOffs=0, yOffs=0):
        tilification = []
        for x, ycoords in enumerate(columns):
            for y in ycoords:
                tilification.append(SpriteTile(x * self.settings.tileWidth + xOffs, y + yOffs, self.settings.tileWidth, self.settings.tileHeight, 0, False, False))
        self.optimizeRows(self.si, self.tileTable, tilification)
        self.optimizeRowsTilesNew(self.si, self.tileTable, tilification)
        tilification = [st for st in tilification if not self.tileTable.blankTile(self.si, st.x, st.y)]
        numTilesAdded, gmi = self.addTiles(self.tileTable, self.si, tilification)
        self.tileTable.removeTiles(numTilesAdded)
        return tilification, numTilesAdded, gmi

    def reduceColumn(self, si, columns, xOffs=0, columnIndex=0):
        columnRowLineBufs = [([0] * self.settings.tileHeight) + self.makeRowLineBuf(si, xOffs + self.settings.tileWidth * i) for i in range(len(columns))]
        minY = -self.settings.tileHeight
        maxY = si.height - 1
        height = maxY + 1 - minY
        spritesScanline = [0] * height
        for y in range(height):
            spritesScanline[y] += sum([rowLineBuf[y] for rowLineBuf in columnRowLineBufs])

        # Create a dictionary of possible matches for this column, containing a list of "potential matches" for y coordinates
        # a "potential match" exists if there is a tiletable match or a self-match less than tileWidth horizontal pixels away
        possibleMatches = {}
        matchesDict = self.tileTable.getAllMatches(si, -self.settings.tileWidth + 1, minY, si.width - 1, maxY)
        for y in range(height):
            x = columnIndex * self.settings.tileWidth + xOffs
            # Tiletable matches
            if y in matchesDict:
                matches = [(spriteTile.x, spriteTile.y) for spriteTile in matchesDict[y] if -8 < x - spriteTile.x < 8]
                if y in possibleMatches:
                    possibleMatches[y].extend(matches)
                else:
                    possibleMatches[y] = matches
            # Self matches - not working properly ATM
            # for xt,yt,listOfMatches in si.selfSimilarityPerLine[y]:
            #    if -8 < x-xt < 8:
            #        if y in possibleMatches:
            #            possibleMatches[y].append((xt,yt))
            #        else:
            #            possibleMatches[y] = [(xt,yt)]

        minimumCost = None
        for ycoords in columns[columnIndex]:
            tileHeight = self.settings.tileHeight
            numberOfPotentialMatches = sum([len(possibleMatches[y + tileHeight]) for y in ycoords if (y + tileHeight) in possibleMatches])
            spritesScanlineColumn = [-1] * height
            for spriteY in ycoords:
                for y in range(self.settings.tileHeight):
                    if spriteY + y < len(spritesScanlineColumn):
                        spritesScanlineColumn[spriteY + y] += 1
            cost = costOfScanlines(spriteImage=si,
                                   optimizationSettings=self.optimizationSettings,
                                   spritesScanline=spritesScanline + spritesScanlineColumn,
                                   numSprites=len(ycoords),
                                   numberOfTilesAdded=len(ycoords) - numberOfPotentialMatches)
            if minimumCost is None or cost < minimumCost:
                minimumCost = cost
                minimumCostCoords = ycoords
        return [minimumCostCoords], minimumCost

    def reduceCombinations(self, yCoordCombinations, xOffs, limit):
        reducedCombination = []
        reducedCombinationCost = []
        for i, combination in enumerate(yCoordCombinations):
            if len(combination) > 1:
                combination, cost = self.reduceColumn(self.si,
                                                      yCoordCombinations,
                                                      xOffs=xOffs,
                                                      columnIndex=i)
            else:
                combination = yCoordCombinations[i]
                cost = float('inf')
            reducedCombination.append(combination)
            reducedCombinationCost.append(cost)
        while reduce(operator.mul, [len(combination) for combination in yCoordCombinations]) > limit:  # mul([len(combination) for combination in yCoordCombinations]) > limit:
            minimumCostIndex = reducedCombinationCost.index(min(reducedCombinationCost))
            yCoordCombinations[minimumCostIndex] = reducedCombination[minimumCostIndex]
            reducedCombinationCost[minimumCostIndex] = float('inf')
        return yCoordCombinations

    def tilify(self):
        # Create padded version of spriteImage data for c_tile.getAllMatches function
        padX = self.settings.tileWidth - 1
        padY = self.settings.tileHeight - 1
        self.si.dataPadded = array2d('B', range(-padX, self.si.width + padX), range(-padY, self.si.height + padY), 0)
        for y in self.si.dataPadded.yRange:
            for x in self.si.dataPadded.xRange:
                if 0 <= x < self.si.width and 0 <= y < self.si.height:
                    self.si.dataPadded[x, y] = self.si.data[y * self.si.width + x]

        t = Tilification()
        self.si.tilification = t
        t.tiles = []

        maxTilesW = int(ceil(float(self.si.width) / float(self.settings.tileWidth)))
        shiftFreedom = (maxTilesW * self.settings.tileWidth - self.si.width)

        numCombinations = 0
        prods = []
        for s in range(shiftFreedom + 1):
            ycoordCombinationsMinimal = [0] * maxTilesW
            for r in range(maxTilesW):
                x = r * self.settings.tileWidth - s
                ycoordCombinationsMinimal[r] = self.getSpriteColumns(self.si, x)
            ycoordCombinationsMinimal = self.reduceCombinations(ycoordCombinationsMinimal, -s, 1000)
            numCombinations += reduce(operator.mul, [len(combination) for combination in ycoordCombinationsMinimal])  # shiftCombinationLen
            prod = itertools.product(*[ycoords for ycoords in ycoordCombinationsMinimal])
            prods.append(prod)

        percentDonePrevious = 0
        indexPrevious = 0
        minimumCost = None
        index = 0
        for s in range(shiftFreedom + 1):
            for x, columns in enumerate(prods[s]):
                tilification, numTilesAdded, gmi = self.getOptimizedTilificationFromColumns(columns,
                                                                                            xOffs=-s,
                                                                                            yOffs=-self.settings.tileHeight)
                cost = costOfTilification(tilification=tilification,
                                          spriteImage=self.si,
                                          optimizationSettings=self.optimizationSettings,
                                          numberOfTilesAdded=numTilesAdded,
                                          globalOptimization=gmi)
                if minimumCost is None or cost < minimumCost:
                    minimumCost = cost
                    minimumCostIndex = index
                    minimumCostTilification = tilification
                # Perform yield only on whole % factors
                percentDone = (100 * index) / numCombinations
                if percentDone > percentDonePrevious or (index - indexPrevious) > 10:
                    yield float(percentDone) / 100.0
                    percentDonePrevious = percentDone
                    indexPrevious = index
                index += 1

        #print "  best index = " + str(minimumCostIndex)

        # Select best config and tilify
        #s = optimalCostConfigs[minimumCostIndex][1]
        #config = optimalCostConfigs[minimumCostIndex][2]
        t.tiles = minimumCostTilification  # optimalCostConfigs[minimumCostIndex][3]

        #self.optimizeRows(self.si, self.tileTable, t.tiles)
        #self.si.cutRows, self.si.imageCuts = self.optimizeRowsTilesNew(self.si, self.tileTable, t.tiles)
        self.addTiles(self.tileTable, self.si, t.tiles)

    #
    # Creates links between adjacent sprite tiles touching each other
    #

    def linkSpriteTiles(self, allSpriteTiles):
        for st in allSpriteTiles:
            st.leftOfCandidates = [stL for stL in allSpriteTiles if abs(st.y - stL.y) < self.settings.tileHeight and stL.x <= st.x]
            st.rightOfCandidates = [stR for stR in allSpriteTiles if abs(st.y - stR.y) < self.settings.tileHeight and st.x <= stR.x]
        self.relinkSpriteTiles(allSpriteTiles)

    def relinkSpriteTiles(self, allSpriteTiles):
        for st in allSpriteTiles:
            self.relinkSpriteTile(st)
            st.xmovement = 0

    def relinkSpriteTile(self, st):
        st.leftOf = [stL for stL in st.leftOfCandidates if stL.x < st.x <= stL.x + self.settings.tileWidth]
        st.rightOf = [stR for stR in st.rightOfCandidates if st.x < stR.x <= st.x + self.settings.tileWidth]

    @staticmethod
    def resetMovement(allSpriteTiles):
        for st in allSpriteTiles:
            st.xmovement = 0

    def makeFirstNonBlankPixelInColumTable(self, si):
        padX = self.settings.tileWidth - 1
        padY = self.settings.tileHeight - 1
        paddedWidth = si.width + 2 * padX
        paddedHeight = si.height + 2 * padY
        firstNonBlankPixelTable = array2d('B', paddedWidth, paddedHeight, 0)
        for i in range(paddedWidth * paddedHeight):
            firstNonBlankPixelTable[i] = paddedHeight
        for x in range(0, paddedWidth):
            firstNonBlankPixel = paddedHeight
            for y in reversed(list(range(0, paddedHeight))):
                sx = x - padX
                sy = y - padY
                if 0 <= sx < si.width and 0 <= sy < si.height and si.data[sy * si.width + sx]:
                    firstNonBlankPixel = y
                firstNonBlankPixelTable[y * paddedWidth + x] = firstNonBlankPixel - y
        return firstNonBlankPixelTable

    def columnBlank(self, si, xStart, yStart, height):
        return si.firstNonBlankPixelInColumnTable[xStart, yStart] >= height

    def makeFirstAndLastNonBlankColumnTable(self, si):
        padX = self.settings.tileWidth - 1
        padY = self.settings.tileHeight - 1
        paddedWidth = si.width + 2 * padX
        paddedHeight = si.height + 2 * padY
        si.firstNonBlankPixelInColumnTable = self.makeFirstNonBlankPixelInColumTable(si)
        si.firstNonBlankColumnTable = []
        si.lastNonBlankColumnTable = []
        for h in range(0, self.settings.tileHeight + 2):
            si.firstNonBlankColumnTable.append(array2d('B', paddedWidth, paddedHeight, 0))
            si.lastNonBlankColumnTable.append(array2d('B', paddedWidth, paddedHeight, 0))
            for y in range(0, paddedHeight):
                nonBlankColumns = deque()
                for x in reversed(list(range(0, paddedWidth))):
                    if not self.columnBlank(si, x, y, h):
                        nonBlankColumns.appendleft(x)
                    # Remove nonBlankColumn if we cannot reach it anymore
                    if nonBlankColumns and (nonBlankColumns[-1] - x >= self.settings.tileWidth):
                        nonBlankColumns.pop()
                    si.firstNonBlankColumnTable[h][x, y] = min(nonBlankColumns[0] - x, self.settings.tileWidth) if nonBlankColumns else self.settings.tileWidth
                    si.lastNonBlankColumnTable[h][x, y] = min(nonBlankColumns[-1] - x, self.settings.tileWidth - 1) if nonBlankColumns else 0

                # for x in range(0, si.width):
                    #st = SpriteTile(x=x, y=y)
                    #si.firstNonBlankColumnTable[h][y*si.width+x] = self.findFirstNonBlankColumnOld(si, st, 0, h-1)
                    #si.lastNonBlankColumnTable[h][y*si.width+x] = self.findLastNonBlankColumnOld(si, st, 0, h-1)

    def findFirstNonBlankColumnOld(self, spriteImage, spriteTile, yStart=0, yEnd=None):
        if yEnd is None:
            yEnd = self.settings.tileHeight - 1
        for x in range(spriteTile.x, spriteTile.x + self.settings.tileWidth):
            for y in range(spriteTile.y + yStart, spriteTile.y + yEnd + 1):
                if (0 <= x < spriteImage.width) and (0 <= y < spriteImage.height) and (spriteImage.data[y * spriteImage.width + x] != 0):
                    return x - spriteTile.x
        return self.settings.tileWidth

    def findFirstNonBlankColumn(self, spriteImage, spriteTile, yStart=0, yEnd=None):
        if yEnd is None:
            yEnd = self.settings.tileHeight - 1
        columnHeight = yEnd + 1 - yStart
        if columnHeight < 0:
            columnHeight = 0
        w = spriteImage.firstNonBlankColumnTable[columnHeight].width
        h = spriteImage.firstNonBlankColumnTable[columnHeight].height
        x = min(max(spriteTile.x + self.settings.tileWidth - 1, 0), w - 1)
        y = min(max(spriteTile.y + yStart + self.settings.tileHeight - 1, 0), h - 1)
        return spriteImage.firstNonBlankColumnTable[columnHeight][x, y]

    def findLastNonBlankColumnOld(self, spriteImage, spriteTile, yStart=0, yEnd=None, debug=False):
        if yEnd is None:
            yEnd = self.settings.tileHeight - 1
        for x in reversed(list(range(spriteTile.x, spriteTile.x + self.settings.tileWidth))):
            for y in range(spriteTile.y + yStart, spriteTile.y + yEnd + 1):
                if (0 <= x < spriteImage.width) and (0 <= y < spriteImage.height) and (spriteImage.data[y * spriteImage.width + x] != 0):
                    return x - spriteTile.x
        return 0

    def findLastNonBlankColumn(self, spriteImage, spriteTile, yStart=0, yEnd=None, debug=False):
        if yEnd is None:
            yEnd = self.settings.tileHeight - 1
        columnHeight = yEnd + 1 - yStart
        if columnHeight < 0:
            columnHeight = 0
        w = spriteImage.firstNonBlankColumnTable[columnHeight].width
        h = spriteImage.firstNonBlankColumnTable[columnHeight].height
        x = min(max(spriteTile.x + self.settings.tileWidth - 1, 0), w - 1)
        y = min(max(spriteTile.y + yStart + self.settings.tileHeight - 1, 0), h - 1)
        return spriteImage.lastNonBlankColumnTable[columnHeight][x, y]

    def makeRowLineBuf(self, si, sx):
        buf = [0] * si.height
        for y in range(si.height):
            for xx in range(self.settings.tileWidth):
                x = sx + xx
                if (0 <= x < si.width) and (0 <= y < si.height) and (si.data[y * si.width + x] != 0):
                    buf[y] = 1
                    break
        return buf

    def genYCoords(self, buf, bufNext1, offs=0):
        if offs >= len(buf):
            return []
        coordsList = []
        for y in range(offs, bufNext1[offs] + 1):
            tail = self.genYCoords(buf, bufNext1, y + self.settings.tileHeight)
            nextRange = expandedProduct([y], tail)
            coordsList.extend(nextRange)
        return coordsList

    def getSpriteColumns(self, si, x):
        buf = ([0] * self.settings.tileHeight) + self.makeRowLineBuf(si, x)
        bufNext1 = makeNext1(buf)
        ycoordCombinations = self.genYCoords(buf, bufNext1, bufNext1[0] - self.settings.tileHeight + 1)
        ycoordCombinationsLen = [len(coordsList) for coordsList in ycoordCombinations]
        minLen = min(ycoordCombinationsLen)
        ycoordCombinationsMinimal = [coordsList for coordsList in ycoordCombinations if len(coordsList) == minLen]
        return ycoordCombinationsMinimal

    def getLeftBorderGap(self, st):
        return self.getBorderGap(st, st.leftOf)

    def getRightBorderGap(self, st):
        return self.getBorderGap(st, st.rightOf)

    def getBorderGap(self, spriteTile, leftOrRightOf):
        startY = spriteTile.y + 0
        endY = spriteTile.y + self.settings.tileHeight
        for st in leftOrRightOf:
            if st.y < spriteTile.y and st.y + self.settings.tileHeight > startY:
                startY = st.y + self.settings.tileHeight
            if st.y >= spriteTile.y and st.y < endY:
                endY = st.y
        startY -= spriteTile.y
        endY -= spriteTile.y
        return startY, endY

    def getCommonLinesRange(self, spriteTileA, spriteTileB):
        if spriteTileA.y < spriteTileB.y:
            return spriteTileB.y, spriteTileA.y + self.settings.tileHeight
        else:
            return spriteTileA.y, spriteTileB.y + self.settings.tileHeight

    def draggableToRight(self, spriteTile, si):
        if spriteTile.fixed:
            return False
        if spriteTile.leftOf == []:
            return self.findFirstNonBlankColumn(si, spriteTile, 0, self.settings.tileHeight - 1) > 0
        else:
            startY, endY = self.getLeftBorderGap(spriteTile)
            firstNonBlankColumn = self.findFirstNonBlankColumn(si, spriteTile, startY, endY - 1)
            if firstNonBlankColumn == 0:
                return False
            for stL in spriteTile.leftOf:
                startY, endY = self.getCommonLinesRange(spriteTile, stL)
                startY -= spriteTile.y
                endY -= spriteTile.y
                if startY != endY:
                    firstNonBlankColumnInCommonArea = self.findFirstNonBlankColumn(si, spriteTile, startY, endY - 1)
                else:
                    firstNonBlankColumnInCommonArea = None
                if not (stL.x + self.settings.tileWidth - spriteTile.x > 0 or self.draggableToRight(stL, si) or firstNonBlankColumnInCommonArea != 0):
                # if (not (stL.x+self.settings.tileWidth-spriteTile.x > 0)) and (firstNonBlankColumnInCommonArea == 0) and (not self.draggableToRight(stL, si)):
                    return False
            return True

    def dragToRight(self, spriteTile, si):
        spriteTile.xmovement = 1
        for stL in spriteTile.leftOf:
            if stL.xmovement == 0 and self.draggableToRight(stL, si):
                self.dragToRight(stL, si)

    def draggableToLeft(self, spriteTile, si):
        if spriteTile.fixed:
            return False
        elif spriteTile.rightOf == []:
            return self.findLastNonBlankColumn(si, spriteTile, 0, self.settings.tileHeight - 1) < self.settings.tileWidth - 1
        else:
            startY, endY = self.getRightBorderGap(spriteTile)
            lastNonBlankColumn = self.findLastNonBlankColumn(si, spriteTile, startY, endY - 1)
            if lastNonBlankColumn == self.settings.tileWidth - 1:
                return False
            for stR in spriteTile.rightOf:
                startY, endY = self.getCommonLinesRange(spriteTile, stR)
                startY -= spriteTile.y
                endY -= spriteTile.y
                if startY != endY:
                    lastNonBlankColumnInCommonArea = self.findLastNonBlankColumn(si, spriteTile, startY, endY - 1)
                else:
                    lastNonBlankColumnInCommonArea = None
                if not (spriteTile.x + self.settings.tileWidth - stR.x > 0 or self.draggableToLeft(stR, si) or lastNonBlankColumnInCommonArea != self.settings.tileWidth - 1):
                # if (not (spriteTile.x+self.settings.tileWidth-stR.x > 0)) and (lastNonBlankColumnInCommonArea == self.settings.tileWidth-1) and (not self.draggableToLeft(stR, si)):
                    return False
            return True

    def dragToLeft(self, spriteTile, si):
        spriteTile.xmovement = -1
        for stR in spriteTile.rightOf:
            if stR.xmovement == 0 and self.draggableToLeft(stR, si):
                self.dragToLeft(stR, si)

    def dragFullyToLeft(self, st, si, allSpriteTiles):
        while self.draggableToLeft(st, si):
            if st.x < -self.settings.tileWidth:
                st.fixed = True
                #print "  WARNING: sprite at y=%d dragged outside of left edge" % st.y
                break
            self.dragToLeft(st, si)
            for spriteTile in allSpriteTiles:
                spriteTile.x += spriteTile.xmovement
            self.relinkSpriteTiles(allSpriteTiles)

    def dragFullyToRight(self, st, si, allSpriteTiles):
        while self.draggableToRight(st, si):
            if st.x > si.width + self.settings.tileWidth:
                st.fixed = True
                #print "  WARNING: sprite at y=%d dragged outside of right edge" % st.y
                break
            self.dragToRight(st, si)
            for spriteTile in allSpriteTiles:
                spriteTile.x += spriteTile.xmovement
            self.relinkSpriteTiles(allSpriteTiles)

    def minimumSpriteTilesNeededForRow(self, spriteTiles, si, yStart, yEnd):
        width = self.findLastNonBlankColumn(si, spriteTiles[-1], yStart - spriteTiles[-1].y, yEnd - spriteTiles[-1].y) + 1 + spriteTiles[-1].x - (self.findFirstNonBlankColumn(si, spriteTiles[0], yStart - spriteTiles[0].y, yEnd - spriteTiles[0].y) + spriteTiles[0].x)
        minimumSpriteTiles = int(ceil(float(width) / float(self.settings.tileWidth)))
        return minimumSpriteTiles

    def reductionImpossible(self, si, spriteTiles):
        cutRows, imageCuts = self.getCutRows(spriteTiles)
        #spriteTilesNonOptional = []
        for spriteTile in spriteTiles:
            spriteTile.nonOptional = False
        numNonOptional = 0
        for i, row in enumerate(cutRows):
            if row and self.minimumSpriteTilesNeededForRow(row, si, imageCuts[i], imageCuts[i + 1]) == len(row):
                for spriteTile in row:
                    # if spriteTile not in spriteTilesNonOptional:
                    #    spriteTilesNonOptional.append(spriteTile)
                    if not spriteTile.nonOptional:
                        spriteTile.nonOptional = True
                        numNonOptional += 1
        return numNonOptional == len(spriteTiles)
        # if numNonOptional == len(spriteTiles):
        #    print "    Reduction possible: NO"
        #    return True
        # else:
        #    print "    Reduction possible: MAYBE"
        #    return False

    def tryToReduce(self, si, tileTable, allSpriteTiles):
        lenLast = len(allSpriteTiles)

        # Try to reduce spriteTiles by dragging leftmost tiles fully to the left and check for blank space
        # self.relinkSpriteTiles(allSpriteTiles)
        leftMost = [st for st in allSpriteTiles if st.leftOf == [] and st.rightOf != []]
        rightMost = [st for st in allSpriteTiles if st.rightOf == [] and st.leftOf != []]

        for st in leftMost:
            self.dragFullyToLeft(st, si, allSpriteTiles)
        blankSpriteTiles = [st for st in allSpriteTiles if self.tileTable.blankTile(si, st.x, st.y)]
        if blankSpriteTiles:
            for st in blankSpriteTiles:
                allSpriteTiles.remove(st)
                self.linkSpriteTiles(allSpriteTiles)
                return
        else:
            self.relinkSpriteTiles(allSpriteTiles)

        # Try to reduce spriteTiles by dragging rightmost tiles fully to the right and check for blank space
        self.linkSpriteTiles(allSpriteTiles)
        leftMost = [st for st in allSpriteTiles if st.leftOf == [] and st.rightOf != []]
        rightMost = [st for st in allSpriteTiles if st.rightOf == [] and st.leftOf != []]
        for st in rightMost:
            self.dragFullyToRight(st, si, allSpriteTiles)
        blankSpriteTiles = [st for st in allSpriteTiles if self.tileTable.blankTile(si, st.x, st.y)]
        if blankSpriteTiles:
            for st in blankSpriteTiles:
                allSpriteTiles.remove(st)
                self.linkSpriteTiles(allSpriteTiles)
                return
        else:
            self.relinkSpriteTiles(allSpriteTiles)

        # Try to reduce spriteTiles by dragging leftMost spriteTiles to the right,
        # rightmost to the left and finally trying to find redundant spriteTiles
        for st in leftMost:
            self.dragFullyToRight(st, si, allSpriteTiles)

        for st in rightMost:
            self.dragFullyToLeft(st, si, allSpriteTiles)

        # add tiles
        numTilesPrev = self.tileTable.numTiles
        self.addTiles(self.tileTable, si, allSpriteTiles)

        # Obtain coverage
        spriteCoverage = array.array('B', [0] * self.si.width * self.si.height)
        coverageFromTilification(spriteCoverage, allSpriteTiles, self.si, self.tileTable)
        si.spriteCoverage = spriteCoverage
        # Unconditionally select all spritetiles which contain at least one pixel not shared by any other tile
        spriteTilesNonOptional = [st for st in allSpriteTiles if spriteTileHasUnsharedPixels(st, self.si, spriteCoverage, self.tileTable)]
        spriteCoverageNonOptional = array.array('B', [0] * self.si.width * self.si.height)
        coverageFromTilification(spriteCoverageNonOptional, spriteTilesNonOptional, self.si, self.tileTable, binary=True)

        # Remove them from original list
        spriteTilesOptional = [st for st in allSpriteTiles if st not in spriteTilesNonOptional]

        # Remove all tiles that only cover redundant pixels already covered by the non-optional tiles
        redundantSpriteTiles = [st for st in spriteTilesOptional if spriteTileRedundant(st, self.si, spriteCoverageNonOptional, self.tileTable)]

        for st in redundantSpriteTiles:
            allSpriteTiles.remove(st)
            spriteTilesOptional.remove(st)

        # Now, the only optional tiles left cannot be removed in total. Try to
        # remove only one at a time and check cost of each alternative
        if spriteTilesOptional:
            costs = []
            tilifications = []
            for spriteTile in spriteTilesOptional:
                spriteTiles = [st for st in allSpriteTiles if st != spriteTile]
                costs.append(costOfTilification(tilification=spriteTiles, spriteImage=self.si, optimizationSettings=self.optimizationSettings, numberOfTilesAdded=0))
                tilifications.append(spriteTiles)
            allSpriteTiles[:] = tilifications[indexOfMinimum(costs)]

        self.tileTable.removeTiles(self.tileTable.numTiles - numTilesPrev)

        if len(allSpriteTiles) != lenLast:
            self.linkSpriteTiles(allSpriteTiles)
        else:
            self.relinkSpriteTiles(allSpriteTiles)

    def optimizeRows(self, si, tileTable, allSpriteTiles):
        self.linkSpriteTiles(allSpriteTiles)
        lenLast = 0
        while lenLast != len(allSpriteTiles) and (not self.reductionImpossible(si, allSpriteTiles)):
            lenLast = len(allSpriteTiles)
            self.tryToReduce(si, tileTable, allSpriteTiles)

    def tryToMoveToX(self, matchPositionX, st, allSpriteTiles, si):
        while st.x != matchPositionX:
            if st.x < matchPositionX and self.draggableToRight(st, si):
                self.dragToRight(st, si)
                for spriteTile in allSpriteTiles:
                    spriteTile.x += spriteTile.xmovement
                self.relinkSpriteTiles(allSpriteTiles)
            elif st.x > matchPositionX and self.draggableToLeft(st, si):
                self.dragToLeft(st, si)
                for spriteTile in allSpriteTiles:
                    spriteTile.x += spriteTile.xmovement
                self.relinkSpriteTiles(allSpriteTiles)
            else:
                break
        return st.x == matchPositionX

    def matchSpriteTilesIterative(self, spriteTiles, positionsList, allSpriteTiles, si, listOfSimilar):
        """
        For a given set of sprite tiles and a corresponding list of candiate positions for these tiles,
        returns a list of final positions and the tiletable tile count increase (calculated how???)
        """
        if spriteTiles == []:
            return [], []
        numTilesAddedList = []
        finalPositionsList = []
        self.relinkSpriteTiles(allSpriteTiles)
        numTileTableMatches = 0
        for positions in positionsList:
            matchedDict = dict()
            prevFixed = [False] * len(spriteTiles)
            finalPositions = []
            for i, st in enumerate(spriteTiles):
                prevFixed[i] = st.fixed
                if positions[i] is not None and not st.fixed:
                    if self.tryToMoveToX(positions[i], st, allSpriteTiles, si):
                        st.fixed = True
                        numTileTableMatches += 1

            # Now try the list of similar tiles
            numTilesAdded = 0
            for similar in listOfSimilar:
                matchedDict = dict()
                numSelfMatches = 0
                wasFixedIndices = []
                for match in similar:
                    index, matchPositionX, flipH, flipV = match
                    if not spriteTiles[index].fixed:
                        st = spriteTiles[index]
                        if self.tryToMoveToX(matchPositionX, st, allSpriteTiles, si):
                            st.fixed = True
                            wasFixedIndices.append(index)
                            w = si.width + (self.settings.tileWidth - 1) * 2
                            offs = (st.y + self.settings.tileHeight - 1) * w + st.x + self.settings.tileWidth - 1
                            h = si.hashImage[offs]
                            hH = si.hashImageH[offs]
                            hV = si.hashImageV[offs]
                            hHV = si.hashImageHV[offs]
                            if (h not in matchedDict) and (hH not in matchedDict) and (hV not in matchedDict) and (hHV not in matchedDict):
                                matchedDict[h] = True
                            numSelfMatches += 1
                if numSelfMatches > 1:
                    numTilesAdded += 1
                else:
                    # Reset all fixed flags, as we did not get any real matches
                    for index in wasFixedIndices:
                        spriteTiles[index].fixed = False

            numUnfixedRemaining = 0
            for i, st in enumerate(spriteTiles):
                finalPositions.append(st.x)
                if not st.fixed:
                    numUnfixedRemaining += 1
                st.fixed = prevFixed[i]
            finalPositionsList.append(finalPositions)
            numTilesAddedList.append(numTilesAdded + numUnfixedRemaining)

        return finalPositionsList, numTilesAddedList

    def getAllMatchesSelf(self, spriteTiles):
        yCoordinates = []
        for st in spriteTiles:
            if st.y not in yCoordinates:
                yCoordinates.append(st.y)
        matchDict = dict()
        listOfSimilar = []
        for yCoord in yCoordinates:
            for selfSimilarities in self.si.selfSimilarityPerLine[yCoord]:
                x, y = selfSimilarities[0], selfSimilarities[1]
                w = self.si.width + (self.settings.tileWidth - 1) * 2
                offs = (y + self.settings.tileHeight - 1) * w + x + self.settings.tileWidth - 1
                h = self.si.hashImage[offs]
                hH = self.si.hashImageH[offs]
                hV = self.si.hashImageV[offs]
                hHV = self.si.hashImageHV[offs]
                if (h not in matchDict) and (hH not in matchDict) and (hV not in matchDict) and (hHV not in matchDict):
                    matchDict[h] = True
                    matchDict[hH] = True
                    matchDict[hV] = True
                    matchDict[hHV] = True
                    matchList = [(x, y, False, False)]
                    matchList.extend(selfSimilarities[2])
                    similar = []
                    for match in matchList:
                        x, y, flipH, flipV = match
                        spriteTilesInRangeIndices = [i for i, st in enumerate(spriteTiles) if st.minimumX <= x <= st.maximumX and st.y == yCoord]
                        if spriteTilesInRangeIndices:
                            index = spriteTilesInRangeIndices[0]
                            similar.append((index, x, flipH, flipV))
                    if similar != []:
                        listOfSimilar.append(similar)
        return listOfSimilar

    @staticmethod
    def validCombinedMatch(spriteTiles, combinedMatchPositionsX):
        for i, st in enumerate(spriteTiles):
            x = combinedMatchPositionX[i]
            while st.x != x:
                if x < st.x:
                    if not isDraggableLeft():
                        return False
                    else:
                        dragLeft()
                elif st.x < x:
                    if not isDraggableRight():
                        return False
                    else:
                        dragRight()
            st.fixed = True
        return True

    def squeezeSpriteTiles(self, si, allSpriteTiles):
        # Drag leftMost spriteTiles to the right, and rightmost to the left
        self.relinkSpriteTiles(allSpriteTiles)
        leftMost = [st for st in allSpriteTiles if st.leftOf == [] and st.rightOf != []]
        rightMost = [st for st in allSpriteTiles if st.rightOf == [] and st.leftOf != []]
        for st in leftMost:
            self.dragFullyToRight(st, si, allSpriteTiles)
        for st in rightMost:
            self.dragFullyToLeft(st, si, allSpriteTiles)

    def moveSpriteTilesUsingGMI(self, si, allSpriteTiles):
        """
        For a sprite tile st, returns True if changing its position from oldX to newX would 'squeeze' it more.
        Squeezing a sprite tile means moving a left-edge one to the right, and a right-edge one to the left.
        (and doing nothing for central sprites at this point)
        """
        def _moreSqueezed(st, newX, oldX):
            if not st.leftOf:
                return newX > oldX
            elif not st.rightOf:
                return newX < oldX
            else:
                return False
        """
        Moves the sprite tiles to the optimal place according to the stored global-match-image
        """
        def _maxMatchSpriteTile(si, st, allSpriteTiles):
            maxMatch = -1
            maxMatchX = 0
            y = st.y
            for x in range(st.minimumX, st.maximumX + 1):
                if si.globalMatches.xRange[0] <= x <= si.globalMatches.xRange[-1] and si.globalMatches.yRange[0] <= y <= si.globalMatches.yRange[-1]:
                    #solidFactor = self.si.tileNumSolid[x, y] / (self.settings.tileWidth * self.settings.tileHeight)
                    gmi = si.globalMatches[x, y]  # * solidFactor * solidFactor
                else:
                    gmi = 0
                isMoreSqueezed = gmi == maxMatch and _moreSqueezed(st, x, maxMatchX)
                if gmi > maxMatch or isMoreSqueezed:
                    maxMatch = gmi
                    maxMatchX = x
            return maxMatch, maxMatchX

        def _printSpriteTiles(spriteTiles):
            for i, st in enumerate(spriteTiles):
                print('{}: ({},{}) {}..{}'.format(i,
                                                  st.x + self.settings.tileWidth - 1,
                                                  st.y + self.settings.tileHeight - 1,
                                                  st.minimumX,
                                                  st.maximumX))

        def _maxMatchSpriteTiles(si, spriteTiles, allSpriteTiles):
            maxMatch = -1
            maxMatchX = 0
            maxMatchSpriteTile = spriteTiles[0]
            for st in spriteTiles:
                thisMaxMatch, thisMaxMatchX = _maxMatchSpriteTile(si, st, allSpriteTiles)
                if thisMaxMatch > maxMatch:
                    maxMatch = thisMaxMatch
                    maxMatchX = thisMaxMatchX
                    maxMatchSpriteTile = st
            return maxMatch, maxMatchX, maxMatchSpriteTile

        self.relinkSpriteTiles(allSpriteTiles)
        # Keep optimizing until there are no movable sprite tiles left
        movableSpriteTiles = [st for st in allSpriteTiles if not st.fixed]
        while movableSpriteTiles:
            maxMatch, x, stMatch = _maxMatchSpriteTiles(si, movableSpriteTiles, allSpriteTiles)
            if self.tryToMoveToX(x, stMatch, allSpriteTiles, si):
                stMatch.x = x
                stMatch.fixed = True
                #print('Successfully moved spritetile to {}'.format(x))
            else:
                movableSpriteTiles = [st for st in movableSpriteTiles if st != stMatch]
                #print('Could not move spritetile to {}'.format(x))
            movableSpriteTiles = [st for st in movableSpriteTiles if not st.fixed]
            # print('movableSpriteTiles:')
            # _printSpriteTiles(movableSpriteTiles)

    def getSpriteTilesRangeX(self, si, allSpriteTiles):
        for st in allSpriteTiles:
            self.dragFullyToLeft(st, si, allSpriteTiles)
            st.minimumX = st.x
        for st in allSpriteTiles:
            self.dragFullyToRight(st, si, allSpriteTiles)
            st.maximumX = st.x

    @staticmethod
    def mergeMatches(matchesA, matchesB):
        if len(matchesA) != len(matchesB):
            print("len(matchesA) != len(matchesB) in mergeMatches")
            raise Exception

        mergedMatches = []
        for i in range(len(matchesA)):
            listA = matchesA[i]
            listB = matchesB[i]
            if (None in listA) and (None in listB):
                mergedMatches.append([None])
            elif None in listA:
                mergedMatches.append(listB)
            elif None in listB:
                mergedMatches.append(listA)
            else:
                mergedMatches.append(mergeSortedLists([x for x in listA if x is not None], [x for x in listB if x is not None]))
        return mergedMatches

    def optimizeRowsTilesNew(self, si, tileTable, allSpriteTiles):
        self.getSpriteTilesRangeX(si, allSpriteTiles)
        #self.squeezeSpriteTiles(si, allSpriteTiles)

        numTilesPrev = self.tileTable.numTiles

        cutRows, imageCuts = self.getCutRows(allSpriteTiles)

        #si.tilification.tiles = allSpriteTiles

        for st in allSpriteTiles:
            st.fixed = False
            st.tileID = -1

        cutRows.sort(key=lambda spriteTiles: len(spriteTiles), reverse=True)

        for spriteTiles in cutRows:
            if spriteTiles:
                minY = min([st.y for st in spriteTiles])
                maxY = max([st.y for st in spriteTiles])
                matchesDict = self.tileTable.getAllMatches(self.si, -self.settings.tileWidth + 1, minY, si.width - 1, maxY)
                matchesListTileTable = []
                for st in spriteTiles:
                    if st.y in matchesDict:
                        matches = [spriteTile.x for spriteTile in matchesDict[st.y] if st.minimumX <= spriteTile.x <= st.maximumX]
                    else:
                        matches = [None]
                    if not matches:
                        matches = [None]
                    matchesListTileTable.append(matches)

                listOfSimilar = self.getAllMatchesSelf(spriteTiles)

                matchesList = matchesListTileTable
                positionsList = []
                numMatchesList = []
                positionsCandidatesList = combinationsIncreasing(matchesList)
                if positionsCandidatesList != []:
                    positionsList, numTilesAddedList = self.matchSpriteTilesIterative(spriteTiles, positionsCandidatesList, allSpriteTiles, si, listOfSimilar)
                    bestMatch = min(numTilesAddedList)
                    bestMatchIndex = numTilesAddedList.index(bestMatch)
                    for i, st in enumerate(spriteTiles):
                        if positionsList[bestMatchIndex] != []:
                            st.x = positionsList[bestMatchIndex][i]
                            tileData = self.tileTable.cutTile(si, st.x, st.y)
                            st.tileID, st.flipH, st.flipV = self.tileTable.findTile(tileData)
                            if st.tileID < 0:
                                st.tileID = self.tileTable.numTiles
                                self.tileTable.addTile(tileData)
                                st.fixed = True
                            else:
                                st.fixed = True

        # Set fixed flag false for all spriteTiles that didn't get any match, and then squeeze
        numAddedTiles = self.tileTable.numTiles - numTilesPrev
        tileReferences = defaultdict(int)
        for st in allSpriteTiles:
            tileReferences[st.tileID] += 1
        for st in allSpriteTiles:
            st.fixed = st.tileID < numTilesPrev or tileReferences[st.tileID] > 1
        if self.optimizationSettings.useGlobalOptimization:
            self.moveSpriteTilesUsingGMI(si, allSpriteTiles)
        else:
            self.squeezeSpriteTiles(si, allSpriteTiles)

        self.tileTable.removeTiles(self.tileTable.numTiles - numTilesPrev)

        # Last, reset fixed flag to enable drag debugging
        for st in allSpriteTiles:
            st.fixed = False

        return cutRows, imageCuts

    def getCutRows(self, allSpriteTiles):
        # Create variable-height rows by "cutting" the spriteImage at every SpriteTile start/ending
        imageCuts = []
        for st in allSpriteTiles:
            if st.y not in imageCuts:
                bisect.insort(imageCuts, st.y)
            if st.y + self.settings.tileHeight not in imageCuts:
                bisect.insort(imageCuts, st.y + self.settings.tileHeight)
        cutRows = []
        y = imageCuts[0]
        for i in range(1, len(imageCuts)):
            h = imageCuts[i] - y
            spriteTiles = []
            for st in allSpriteTiles:
                if st not in spriteTiles and ((st.y <= y < st.y + self.settings.tileHeight) or (st.y < y + h < st.y + self.settings.tileHeight)):
                    spriteTiles.append(st)
            cutRows.append(spriteTiles)
            y = imageCuts[i]
        return cutRows, imageCuts
