#
# Copyright (C) 2018 Michel Iwaniec
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

from array import array
from copy import copy, deepcopy


class array2d(array):
    """
    Simple wrapper for the array object, allowing indexing with 2d coordinates, padding, and border mode
    """
    def __new__(cls, typecode, xRange, yRange, initializer=None):
        if isinstance(initializer, (int, float)):
            initializer = [initializer] * len(cls._toRange(xRange)) * len(cls._toRange(yRange))
        return array.__new__(cls, typecode, initializer)

    def __init__(self, typecode, xRange, yRange, initializer=None):
        super(array2d, self).__init__()
        self.xRange = self._toRange(xRange)
        self.yRange = self._toRange(yRange)

    def __copy__(self):
        copy_saved = self.__copy__
        self.__copy__ = None
        newObj = copy(self, memo)
        self.__copy__ = copy_saved
        return newObj

    def __deepcopy__(self, memo):
        deepcopy_saved = self.__deepcopy__
        self.__deepcopy__ = None
        newObj = deepcopy(self, memo)
        self.__deepcopy__ = deepcopy_saved
        return newObj

    @staticmethod
    def _toRange(r):
        if isinstance(r, int):
            return range(r)
        elif isinstance(r, range):
            return r
        else:
            raise ValueError('invalid range')

    def __getitem__(self, index):
        if isinstance(index, tuple):
            x, y = index
            if self.xRange[0] <= x <= self.xRange[-1] and self.yRange[0] <= y <= self.yRange[-1]:
                xt = x - self.xRange[0]
                yt = y - self.yRange[0]
                return super(array2d, self).__getitem__(self.width * yt + xt)
            else:
                raise IndexError('coordinates ({},{}) out-of-range'.format(x, y))
        else:
            return super(array2d, self).__getitem__(index)

    def __setitem__(self, index, value):
        if isinstance(index, tuple):
            x, y = index
            if self.xRange[0] <= x <= self.xRange[-1] and self.yRange[0] <= y <= self.yRange[-1]:
                xt = x - self.xRange[0]
                yt = y - self.yRange[0]
                super(array2d, self).__setitem__(self.width * yt + xt, value)
            else:
                raise IndexError('coordinates ({},{}) out-of-range'.format(x, y))
        else:
            super(array2d, self).__setitem__(index, value)

    def __add__(self, other):
        if self.typecode != other.typecode:
            raise TypeError('operand types {} and {} do not match'.format(self.typecode, other.typecode))
        if self.xRange != other.xRange or self.yRange != other.yRange:
            raise IndexError('operand dimensions do not match')
        result = array2d(self.typecode, self.xRange, self.yRange, 0)
        for i in range(self.width * self.height):
            result[i] = min(self[i] + other[i], 255)
        return result

    @property
    def width(self):
        return len(self.xRange)

    @property
    def height(self):
        return len(self.yRange)
