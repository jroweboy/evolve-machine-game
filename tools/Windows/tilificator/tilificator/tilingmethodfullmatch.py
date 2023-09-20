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

from tilificator.tilingmethod import *
from tilificator.spritetile import *
from tilificator.common import splitWhenTrue, indexOfMinimum


class TilingMethodFullMatch(TilingMethod):
    def __init__(self, si, tileTable, settings, optimizationSettings):
        super(TilingMethodFullMatch, self).__init__(si, tileTable, settings, optimizationSettings)

    @staticmethod
    def invertByteArray(src):
        return array.array('B', [(~x) & 0xFF for x in src])

    @staticmethod
    def bitwiseAndByteArrays(a, b):
        return array.array('B', [a[i] & b[i] for i in range(len(a))])

    def tilify(self):
        t = Tilification()
        self.si.tilification = t
        t.tiles = []

        (spriteTiles, spriteCoverage) = self.tileTable.matchTiles(self.si)

        # Obtain coverage
        spriteCoverage = array.array('B', [0] * self.si.width * self.si.height)
        coverageFromTilification(spriteCoverage, spriteTiles, self.si, self.tileTable)

        # If we do not have full coverage, just return
        if not tilificationHasFullCoverage(self.si, spriteTiles, self.tileTable):
            return

        # Perform first step of optimization by unconditionally selecting all spritetiles which contain at least one pixel not shared by any other tile
        spriteTilesNonOptional = [st for st in spriteTiles if spriteTileHasUnsharedPixels(st, self.si, spriteCoverage, self.tileTable)]
        spriteCoverageNonOptional = array.array('B', [0] * self.si.width * self.si.height)
        spriteCoverageNonOptionalInverted = array.array('B', [0] * self.si.width * self.si.height)
        coverageFromTilification(spriteCoverageNonOptional, spriteTilesNonOptional, self.si, self.tileTable, binary=True)

        # Remove them from original list
        spriteTiles = [st for st in spriteTiles if st not in spriteTilesNonOptional]

        # Remove all tiles that only cover redundant pixels already covered by the non-optional tiles
        spriteTilesReduced = [st for st in spriteTiles if not spriteTileRedundant(st, self.si, spriteCoverageNonOptional, self.tileTable)]

        # Split into clusters that are vertically independent
        spriteTilesReduced.sort(key=lambda spriteTile: spriteTile.y)
        clusters = [cluster for cluster in splitWhenTrue(spriteTilesReduced, f=lambda stA, stB: stA.y + self.settings.tileHeight <= stB.y)]

        # Work on one cluster at a time
        for clusterIndex, cluster in enumerate(clusters):
            # Generate coverage bitmap for all sprites in cluster
            spriteCoverageOptional = array.array('B', [0] * self.si.width * self.si.height)
            coverageFromTilification(spriteCoverageOptional, cluster, self.si, self.tileTable, binary=True)

            #
            # O N
            # 0 0: 0
            # 0 1: 0
            # 1 0: 1
            # 1 1: 0 => O & ~N
            spriteCoverageNonOptionalInverted = self.invertByteArray(spriteCoverageNonOptional)
            spriteCoverageOptionalExclusive = self.bitwiseAndByteArrays(spriteCoverageOptional, spriteCoverageNonOptionalInverted)
            for length in range(0, len(cluster)):
                # if length > 8:
                #    print "Skipping combinations longer than 8..."
                #    continue

                allCombinations = list(itertools.combinations(cluster, length))
                costs = [None] * len(allCombinations)  # costs = [10000000000000000]*len(allCombinations)

                foundFullyCovered = False
                for i, combination in enumerate(allCombinations):
                    # if fullyCovered(newTilifications + list(combination), si, tileTable):
                    # E C
                    # 0 0: 0
                    # 0 1: 0
                    # 1 0: 0
                    # 1 1: 1 => C
                    spriteCoverageCombination = array.array('B', [0] * self.si.width * self.si.height)
                    coverageFromTilification(spriteCoverageCombination, combination, self.si, self.tileTable, binary=True)
                    if self.bitwiseAndByteArrays(spriteCoverageCombination, spriteCoverageOptionalExclusive) == spriteCoverageOptionalExclusive:
                        foundFullyCovered = True
                        costs[i] = costOfTilification(tilification=(spriteTilesNonOptional + list(combination)), spriteImage=self.si, optimizationSettings=self.optimizationSettings)

                if foundFullyCovered:
                    spriteTilesNonOptional += list(allCombinations[indexOfMinimum(costs)])
                    break

            yield float(clusterIndex) / float(len(clusters))

        t.tiles = spriteTilesNonOptional
