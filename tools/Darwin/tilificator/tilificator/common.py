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
import operator
from functools import reduce


def rectangleInsideRectangle(a, b):
    return (b.x <= a.x <= b.x + b.w) and (b.y <= a.y <= b.y + b.h)


def firstNotNone(listOfValues):
    for value in listOfValues:
        if value is not None:
            return value
    return None


def mergeSortedLists(listA, listB):
    indexA = 0
    indexB = 0
    mergedList = []
    while indexA < len(listA) and indexB < len(listB):
        if listA[indexA] <= listB[indexB]:
            mergedList.append(listA[indexA])
            indexA += 1
        else:
            mergedList.append(listB[indexB])
            indexB += 1
    if indexA < len(listA):
        mergedList.extend(listA[indexA:])
    elif indexB < len(listB):
        mergedList.extend(listB[indexB:])
    return mergedList


def combinationsIncreasing(matchList):
    if matchList == []:  # if spriteTiles == []:
        return []
    positionsList = []
    for matchPositionX in matchList[0]:
        if len(matchList) > 1:  # if len(spriteTiles) > 1:
            positionsListRest = combinationsIncreasing(matchList[1:])
            positionsListRestGreater = [xList for xList in positionsListRest if ((firstNotNone(xList) is None or matchPositionX is None) or firstNotNone(xList) >= matchPositionX)]
            if positionsListRestGreater != []:
                positionsList.extend(expandedProduct([matchPositionX], positionsListRestGreater))
        else:
            positionsList.append([matchPositionX])
    return positionsList


def indexOfMinimum(sequence):
    minIndex = None
    for i, x in enumerate(sequence):
        if x is not None and (minIndex is None or x < sequence[minIndex]):
            minIndex = i
    return minIndex


def product(x):
    return reduce(operator.mul, x)


def splitWhenTrue(seqn, f):
    """
    Iterates through split portions of the original sorted sequence provided,
    where a split is performed whenever f(seqn[n],seqn[n+1]) == True
    """
    cluster = []
    for x in seqn:
        if cluster and f(cluster[-1], x):
            yield cluster
            cluster = []
        cluster.append(x)

    if cluster:
        yield cluster


def makeRisingEdge(buf):
    rising = [0] * len(buf)
    for i in range(len(buf)):
        previous = 0 if i == 0 else buf[i - 1]
        if buf[i] != 0 and previous == 0:
            rising[i] = 1
    return rising


def makeNext1(buf):
    rising = makeRisingEdge(buf)
    next1 = [len(buf)] * len(buf)
    nextIndex = len(buf)
    for i in range(len(buf) - 1, -1, -1):
        if rising[i] != 0 or buf[i] != 0:
            nextIndex = i
        next1[i] = nextIndex
    # Now fix the last ones...
    if len(buf) in next1:
        firstNonFixed = next1.index(len(buf))
        for i in range(firstNonFixed, len(buf)):
            next1[i] = next1[i - 1]
    return next1


def expandedProduct(a, b):
    if b == []:
        return [a]
    p = list(itertools.product(a, b))
    return [list(itertools.chain([sublist[0]], sublist[1])) if(isinstance(sublist[1], (list, tuple))) else [sublist[0], sublist[1]] for sublist in p]
