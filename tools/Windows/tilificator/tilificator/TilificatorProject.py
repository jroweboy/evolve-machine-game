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

import plistlib
import sys
import array
import json
import codecs
from collections import OrderedDict

from PIL import Image

from tilificator.sprite import SpriteImage
from tilificator.spritetile import SpriteTile, Tilification, OptimizationSettings
from tilificator.tile import TileSettings, TileTable


class TilificatorProject(object):
    def __init__(self, tileTable=None, optimizationSettings=None, spriteImages=[], palette=None):
        self.tileTable = tileTable
        self.optimizationSettings = optimizationSettings
        self.spriteImages = spriteImages
        self.palette = palette

    @staticmethod
    def split(sequence, splitSize):
        numSplits = (len(sequence) + splitSize - 1) // splitSize
        for i in range(numSplits):
            yield sequence[i * splitSize:(i + 1) * splitSize]

    def write(self, f):
        tpr = OrderedDict()

        # Encode file information section
        tpr["fileinfo"] = OrderedDict([('type', 'Tilificator Project'), ('version', 1.0)])

        # Encode optimization settings
        tpr["optimizationsettings"] = OrderedDict([('tilingmethod', self.optimizationSettings.tilingMethod),
                                                   ('priorities', self.optimizationSettings.priorities),
                                                   ('spritesperscanlinemax', self.optimizationSettings.spritesScanlineMaxLimit),
                                                   ('spritesperscanlineavg', self.optimizationSettings.spritesScanlineAvgLimit),
                                                   ('globaloptimization', self.optimizationSettings.useGlobalOptimization),
                                                   ('solidfactorexponent', self.optimizationSettings.solidFactorExponent)])

        # Write sprite images
        spriteImagesList = []
        for si in self.spriteImages:
            siDict = OrderedDict()
            siDict["width"] = si.width
            siDict["height"] = si.height
            siDict["bytedata"] = [codecs.decode(codecs.encode(line.tostring(), "hex"), "UTF-8") for
                                  line in self.split(si.data, si.width)]
            siDict["spritetiles"] = []
            for st in si.tilification.tiles:
                siDict["spritetiles"].append(OrderedDict([('x', st.x),
                                                          ('y', st.y),
                                                          ('t', st.tileID),
                                                          ('c', st.colorDataHi),
                                                          ('H', (1 if st.flipH else 0)),
                                                          ('V', (1 if st.flipV else 0))]))
            spriteImagesList.append(siDict)
        tpr["spriteimages"] = spriteImagesList

        # Encode tiletable
        settings = self.tileTable.settings
        tileTableSize = self.tileTable.numTiles * settings.tileWidth * settings.tileHeight
        settingsDict = OrderedDict([('tilewidth', settings.tileWidth),
                                    ('tileheight', settings.tileHeight),
                                    ('colorsize', settings.colorSize)])
        bytedata = [codecs.decode(codecs.encode(line.tostring(), "hex"), "UTF-8")
                    for line in self.split(self.tileTable.data[0:tileTableSize], settings.tileWidth)]
        tpr["tiletable"] = OrderedDict([('settings', settingsDict),
                                        ('numtiles', self.tileTable.numTiles),
                                        ('bytedata', bytedata)])

        # Encode palette
        palette = array.array('B', self.palette)
        tpr["palette"] = [codecs.decode(codecs.encode(line.tostring(), "hex"), "UTF-8") for line in self.split(palette, 3)]

        # Write to file
        json.dump(tpr, f, separators=(',', ':'), indent=1)

    def read(self, f):
        # Read from file
        tpr = json.load(f)

        # Decode optimization settings
        osDict = tpr["optimizationsettings"]
        self.optimizationSettings = OptimizationSettings()
        self.optimizationSettings.tilingMethod = osDict['tilingmethod']
        priorities = osDict['priorities']
        # If priorities are missing (due to reading a legacy file), add them last
        for p in self.optimizationSettings.priorities:
            if p not in priorities:
                priorities.append(p)
        self.optimizationSettings.priorities = priorities
        self.optimizationSettings.spritesScanlineMaxLimit = osDict['spritesperscanlinemax']
        self.optimizationSettings.spritesScanlineAvgLimit = osDict['spritesperscanlineavg']
        if 'globaloptimization' in osDict:
            self.optimizationSettings.useGlobalOptimization = osDict['globaloptimization']
            self.optimizationSettings.solidFactorExponent = osDict['solidfactorexponent']

        # Decode tiletable
        tiletableDict = tpr["tiletable"]
        s = tiletableDict['settings']
        tileSettings = TileSettings()
        tileSettings.tileWidth = s['tilewidth']
        tileSettings.tileHeight = s['tileheight']
        tileSettings.colorSize = s['colorsize']
        self.tileTable = TileTable(tileSettings)
        numTilesInFile = tiletableDict['numtiles']
        bytedata = array.array('B', codecs.decode(''.join(tiletableDict['bytedata']), "hex"))
        for i in range(numTilesInFile):
            settings = self.tileTable.settings
            tileDataSize = settings.tileWidth * settings.tileHeight
            tileData = array.array('B', bytedata[i * tileDataSize:(i + 1) * tileDataSize])
            self.tileTable.addTile(tileData)

        # Decode palette
        self.palette = array.array('B', codecs.decode(''.join(tpr['palette']), "hex"))

        # Decode sprite images
        spriteImagesList = tpr["spriteimages"]
        self.spriteImages = []
        for siDict in spriteImagesList:
            si = SpriteImage()
            si.width = siDict["width"]
            si.height = siDict["height"]
            si.data = array.array('B', codecs.decode(''.join(siDict['bytedata']), "hex"))
            si.palette = self.palette
            si.tilification = Tilification()
            si.tilification.tiles = []
            for st in siDict["spritetiles"]:
                si.tilification.tiles.append(SpriteTile(x=st['x'],
                                                        y=st['y'],
                                                        w=tileSettings.tileWidth,
                                                        h=tileSettings.tileHeight,
                                                        tileID=st['t'],
                                                        flipH=st['H'] > 0,
                                                        flipV=st['V'] > 0,
                                                        colorDataHi=st['c']))
            self.spriteImages.append(si)

    def readXML(self, f):
        workspace = plistlib.readPlist(f)
        s = workspace["settings"]
        tileSettings = TileSettings()
        tileSettings.tileWidth = s["tilewidth"]
        tileSettings.tileHeight = s["tileheight"]
        spriteImagesList = workspace["spriteimages"]
        self.spriteImages = []
        for siDict in spriteImagesList:
            si = SpriteImage()
            si.width = siDict["width"]
            si.height = siDict["height"]
            si.data = array.array('B', siDict["pixeldata"].data)
            si.palette = workspace["palette"]
            si.tilification = Tilification()
            si.tilification.tiles = []
            for tile in siDict["tilification"]:
                x, y, tileID, flipH, flipV = tile
                si.tilification.tiles.append(SpriteTile(x=x, y=y, tileID=tileID, flipH=flipH, flipV=flipV))
            self.spriteImages.append(si)
        self.tileTable = TileTable(tileSettings)
        numTilesInFile = workspace["tiletable"][0]
        for i in range(numTilesInFile):
            settings = self.tileTable.settings
            tileDataSize = settings.tileWidth * settings.tileHeight
            tileData = array.array('B', workspace["tiletable"][1].data[i * tileDataSize:(i + 1) * tileDataSize])
            self.tileTable.addTile(tileData)
        self.palette = workspace["palette"]
