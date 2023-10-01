#
# Copyright (C) 2021 Michel Iwaniec
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

import sys
import array

from tilificator.sprite import SpriteImage

from tilificator.spritetile import SpriteTile, Tilification
from tilificator.array2d import array2d

NESST_METASPRITE_BANK_SIZE = (256 * 256) + 2
NESST_ORIGIN_X = 64
NESST_ORIGIN_Y = 64


def _writeSpriteImage(si, spriteTiles, minX, minY, tileTable):
    for s in spriteTiles:
        s.x -= minX
        s.y -= minY
        for i in range(s.h):
            for j in range(s.w):
                x = s.x + j
                y = s.y + i
                tx = (s.w - 1 - j) if s.flipH else j
                ty = (s.h - 1 - i) if s.flipV else i
                c = tileTable.data[s.tileID * (s.w * s.h) + s.w * ty + tx]
                if 0 <= x < si.width and 0 <= y < si.height and c != 0:
                    si.data[x, y] = c | s.colorDataHi
    return


def _readMetaSprite(buf, originX, originY, tileTable):
    spriteTiles = []
    for i in range(64):
        y = buf[0]
        if y != 0xFF:
            tileIndex = buf[1]
            V = (buf[2] >> 7) & 1
            H = (buf[2] >> 6) & 1
            p = buf[2] & 0x3
            x = buf[3]
            st = SpriteTile(x - originX, y - originY, 8, 8, tileIndex, H, V, p << 2)
            spriteTiles.append(st)
            buf = buf[4:]
    if spriteTiles:
        minX = min([s.x for s in spriteTiles])
        minY = min([s.y for s in spriteTiles])
        maxX = max([s.x + s.w for s in spriteTiles])
        maxY = max([s.y + s.h for s in spriteTiles])
        w = maxX - minX
        h = maxY - minY
        si = SpriteImage()
        si.width = w
        si.height = h
        si.data = array2d('B', w, h, [0] * w * h)
        si.palette = array.array('B', [0] * 768)
        si.tilification = Tilification()
        si.tilification.tiles = spriteTiles
        _writeSpriteImage(si, spriteTiles, minX, minY, tileTable)
        return si
    else:
        return None


def _encodeSpriteImage(si, originX, originY):
    buf = array.array('B', [0xFF] * 256)
    for j, st in enumerate(si.tilification.tiles[0:64]):
        H = int(st.flipH)
        V = int(st.flipV)
        p = st.colorDataHi >> 2
        buf[4 * j + 0] = st.y + originY + 1
        buf[4 * j + 1] = st.tileID
        buf[4 * j + 2] = (V << 7) | (H << 6) | p
        buf[4 * j + 3] = st.x + originX
    return buf


def readMetaSpriteBank(filename, tileTable):
    buf = array.array('B', [])
    with open(filename, 'rb') as f:
        buf.fromfile(f, NESST_METASPRITE_BANK_SIZE)
    originX = buf[0]
    originY = buf[1]
    bufLength = len(buf)
    buf = buf[2:]
    spriteImages = []
    for i in range(256):
        si = _readMetaSprite(buf, originX, originY, tileTable)
        if si is not None:
            spriteImages.append(si)
        buf = buf[256:]
    return spriteImages, bufLength


def writeMetaSpriteBank(filename, spriteImages):
    buf = array.array('B', [0xFF] * NESST_METASPRITE_BANK_SIZE)
    buf[0] = NESST_ORIGIN_X
    buf[1] = NESST_ORIGIN_Y
    for i, si in enumerate(spriteImages[0:256]):
        buf[2 + i * 256:2 + (i + 1) * 256] = _encodeSpriteImage(si, NESST_ORIGIN_X, NESST_ORIGIN_Y)
    with open(filename, 'wb') as f:
        buf.tofile(f)
