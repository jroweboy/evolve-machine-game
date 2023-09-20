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

import sys
import os

import array

from PySide2.QtWidgets import QApplication, QDialog, QFileDialog, QDialogButtonBox, QPushButton, QRadioButton, QSpinBox, QGroupBox, QVBoxLayout, QFormLayout
from PySide2.QtCore import Qt

from tilificator.tile import TileSettings

class RawTileFormat:
    def __init__(self):
        pass


RawFormats = dict()

RawFormats['NES'] = RawTileFormat()
RawFormats['NES'].width = 8
RawFormats['NES'].height = 8
RawFormats['NES'].stride = 128
RawFormats['NES'].bitsOffs = [[], []]
RawFormats['NES'].bitsOffs[0] = [7, 6, 5, 4, 3, 2, 1, 0,
                                 15, 14, 13, 12, 11, 10, 9, 8,
                                 23, 22, 21, 20, 19, 18, 17, 16,
                                 31, 30, 29, 28, 27, 26, 25, 24,
                                 39, 38, 37, 36, 35, 34, 33, 32,
                                 47, 46, 45, 44, 43, 42, 41, 40,
                                 55, 54, 53, 52, 51, 50, 49, 48,
                                 63, 62, 61, 60, 59, 58, 57, 56]
RawFormats['NES'].bitsOffs[1] = [71, 70, 69, 68, 67, 66, 65, 64,
                                 79, 78, 77, 76, 75, 74, 73, 72,
                                 87, 86, 85, 84, 83, 82, 81, 80,
                                 95, 94, 93, 92, 91, 90, 89, 88,
                                 103, 102, 101, 100, 99, 98, 97, 96,
                                 111, 110, 109, 108, 107, 106, 105, 104,
                                 119, 118, 117, 116, 115, 114, 113, 112,
                                 127, 126, 125, 124, 123, 122, 121, 120]

RawFormats['NES_8x16'] = RawTileFormat()
RawFormats['NES_8x16'].width = 8
RawFormats['NES_8x16'].height = 16
RawFormats['NES_8x16'].stride = 256
RawFormats['NES_8x16'].bitsOffs = [[], []]
RawFormats['NES_8x16'].bitsOffs[0] = [7, 6, 5, 4, 3, 2, 1, 0,
                                      15, 14, 13, 12, 11, 10, 9, 8,
                                      23, 22, 21, 20, 19, 18, 17, 16,
                                      31, 30, 29, 28, 27, 26, 25, 24,
                                      39, 38, 37, 36, 35, 34, 33, 32,
                                      47, 46, 45, 44, 43, 42, 41, 40,
                                      55, 54, 53, 52, 51, 50, 49, 48,
                                      63, 62, 61, 60, 59, 58, 57, 56,
                                      128+7, 128+6, 128+5, 128+4, 128+3, 128+2, 128+1, 128+0,
                                      128+15, 128+14, 128+13, 128+12, 128+11, 128+10, 128+9, 128+8,
                                      128+23, 128+22, 128+21, 128+20, 128+19, 128+18, 128+17, 128+16,
                                      128+31, 128+30, 128+29, 128+28, 128+27, 128+26, 128+25, 128+24,
                                      128+39, 128+38, 128+37, 128+36, 128+35, 128+34, 128+33, 128+32,
                                      128+47, 128+46, 128+45, 128+44, 128+43, 128+42, 128+41, 128+40,
                                      128+55, 128+54, 128+53, 128+52, 128+51, 128+50, 128+49, 128+48,
                                      128+63, 128+62, 128+61, 128+60, 128+59, 128+58, 128+57, 128+56]
RawFormats['NES_8x16'].bitsOffs[1] = [71, 70, 69, 68, 67, 66, 65, 64,
                                      79, 78, 77, 76, 75, 74, 73, 72,
                                      87, 86, 85, 84, 83, 82, 81, 80,
                                      95, 94, 93, 92, 91, 90, 89, 88,
                                      103, 102, 101, 100, 99, 98, 97, 96,
                                      111, 110, 109, 108, 107, 106, 105, 104,
                                      119, 118, 117, 116, 115, 114, 113, 112,
                                      127, 126, 125, 124, 123, 122, 121, 120,
                                      128+71, 128+70, 128+69, 128+68, 128+67, 128+66, 128+65, 128+64,
                                      128+79, 128+78, 128+77, 128+76, 128+75, 128+74, 128+73, 128+72,
                                      128+87, 128+86, 128+85, 128+84, 128+83, 128+82, 128+81, 128+80,
                                      128+95, 128+94, 128+93, 128+92, 128+91, 128+90, 128+89, 128+88,
                                      128+103, 128+102, 128+101, 128+100, 128+99, 128+98, 128+97, 128+96,
                                      128+111, 128+110, 128+109, 128+108, 128+107, 128+106, 128+105, 128+104,
                                      128+119, 128+118, 128+117, 128+116, 128+115, 128+114, 128+113, 128+112,
                                      128+127, 128+126, 128+125, 128+124, 128+123, 128+122, 128+121, 128+120]

RawFormats['GB'] = RawTileFormat()
RawFormats['GB'].width = 8
RawFormats['GB'].height = 8
RawFormats['GB'].stride = 128
RawFormats['GB'].bitsOffs = [[], []]
RawFormats['GB'].bitsOffs[0] = [7, 6, 5, 4, 3, 2, 1, 0,
                                23, 22, 21, 20, 19, 18, 17, 16,
                                39, 38, 37, 36, 35, 34, 33, 32,
                                55, 54, 53, 52, 51, 50, 49, 48,
                                71, 70, 69, 68, 67, 66, 65, 64,
                                87, 86, 85, 84, 83, 82, 81, 80,
                                103, 102, 101, 100, 99, 98, 97, 96,
                                119, 118, 117, 116, 115, 114, 113, 112]
RawFormats['GB'].bitsOffs[1] = [15, 14, 13, 12, 11, 10, 9, 8,
                                31, 30, 29, 28, 27, 26, 25, 24,
                                47, 46, 45, 44, 43, 42, 41, 40,
                                63, 62, 61, 60, 59, 58, 57, 56,
                                79, 78, 77, 76, 75, 74, 73, 72,
                                95, 94, 93, 92, 91, 90, 89, 88,
                                111, 110, 109, 108, 107, 106, 105, 104,
                                127, 126, 125, 124, 123, 122, 121, 120]

RawFormats['SMS'] = RawTileFormat()
RawFormats['SMS'].width = 8
RawFormats['SMS'].height = 8
RawFormats['SMS'].stride = 256
RawFormats['SMS'].bitsOffs = [[], [], [], []]
RawFormats['SMS'].bitsOffs[0] = [7, 6, 5, 4, 3, 2, 1, 0,
                                 15, 14, 13, 12, 11, 10, 9, 8,
                                 23, 22, 21, 20, 19, 18, 17, 16,
                                 31, 30, 29, 28, 27, 26, 25, 24,
                                 39, 38, 37, 36, 35, 34, 33, 32,
                                 47, 46, 45, 44, 43, 42, 41, 40,
                                 55, 54, 53, 52, 51, 50, 49, 48,
                                 63, 62, 61, 60, 59, 58, 57, 56]
RawFormats['SMS'].bitsOffs[1] = [71, 70, 69, 68, 67, 66, 65, 64,
                                 79, 78, 77, 76, 75, 74, 73, 72,
                                 87, 86, 85, 84, 83, 82, 81, 80,
                                 95, 94, 93, 92, 91, 90, 89, 88,
                                 103, 102, 101, 100, 99, 98, 97, 96,
                                 111, 110, 109, 108, 107, 106, 105, 104,
                                 119, 118, 117, 116, 115, 114, 113, 112,
                                 127, 126, 125, 124, 123, 122, 121, 120]
RawFormats['SMS'].bitsOffs[2] = [135, 134, 133, 132, 131, 130, 129, 128,
                                 143, 142, 141, 140, 139, 138, 137, 136,
                                 151, 150, 149, 148, 147, 146, 145, 144,
                                 159, 158, 157, 156, 155, 154, 153, 152,
                                 167, 166, 165, 164, 163, 162, 161, 160,
                                 175, 174, 173, 172, 171, 170, 169, 168,
                                 183, 182, 181, 180, 179, 178, 177, 176,
                                 191, 190, 189, 188, 187, 186, 185, 184]
RawFormats['SMS'].bitsOffs[3] = [199, 198, 197, 196, 195, 194, 193, 192,
                                 207, 206, 205, 204, 203, 202, 201, 200,
                                 215, 214, 213, 212, 211, 210, 209, 208,
                                 223, 222, 221, 220, 219, 218, 217, 216,
                                 231, 230, 229, 228, 227, 226, 225, 224,
                                 239, 238, 237, 236, 235, 234, 233, 232,
                                 247, 246, 245, 244, 243, 242, 241, 240,
                                 255, 254, 253, 252, 251, 250, 249, 248]


class RawTilesDialog(QDialog):
    def __init__(self, settings):
        super(RawTilesDialog, self).__init__()
        self.settings = settings
        self.filename = None
        # Radio buttons for supported formats
        self.formatsGroupBox = QGroupBox('Tile format')
        self.formatsLayout = QVBoxLayout()
        self.formatsLayout.addWidget(QRadioButton('NES'))
        self.formatsLayout.addWidget(QRadioButton('GB'))
        self.formatsLayout.addWidget(QRadioButton('SMS'))
        self.formatsLayout.addStretch(1)
        self.formatsGroupBox.setLayout(self.formatsLayout)
        self.fileChooserButton = QPushButton('Browse')
        # Use form layout for formats/file button
        self.formLayout = QFormLayout()
        self.formLayout.addRow(self.formatsGroupBox)
        self.formLayout.addRow(self.fileChooserButton)
        self.setLayout(self.formLayout)
        self.fileSize = 0
        self.resize(100, 100)
        # Check first radio button
        self.formatsLayout.itemAt(0).widget().setChecked(True)
        self.setModal(True)

    def getRawFormat(self):
        for i in range(self.formatsLayout.count()):
            radioButton = self.formatsLayout.itemAt(i).widget()
            if radioButton.isChecked():
                if radioButton.text() == 'NES' and self.settings.tileHeight == 16:
                    return RawFormats['NES_8x16']
                else:
                    return RawFormats[radioButton.text()]


class ImportRawTilesDialog(RawTilesDialog):
    def __init__(self, settings):
        super(ImportRawTilesDialog, self).__init__(settings)
        self.setWindowTitle('Import RAW tiles')
        self.maxTiles = 1024
        # File offset
        self.fileOffsetSpinBox = QSpinBox()
        self.fileOffsetSpinBox.setRange(0, 1048576)
        # Num tiles
        self.numTilesSpinBox = QSpinBox()
        self.numTilesSpinBox.setRange(0, self.maxTiles)
        self.formLayout.addRow('File offset:', self.fileOffsetSpinBox)
        self.formLayout.addRow('Number of tiles:', self.numTilesSpinBox)
        # File selection
        self.fileChooserButton.setText('Import from file...')
        self.fileChooserButton.clicked.connect(self.loadFile)
        # Ok/Cancel buttons
        buttonBox = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        buttonBox.accepted.connect(self.accept)
        buttonBox.rejected.connect(self.reject)
        self.buttonOk = buttonBox.button(QDialogButtonBox.Ok)
        self.buttonOk.setEnabled(False)
        self.formLayout.addWidget(buttonBox)
        self.show()

    def loadFile(self, filechooserbutton):
        filename, fileType = QFileDialog.getOpenFileName(parent=self,
                                                         caption='Open RAW data file')
        if filename != '':
            self.fileSize = os.path.getsize(filename)
            self.updateLimits()
            self.numTilesSpinBox.setValue(self.maxTiles)
            self.filename = filename
            self.buttonOk.setEnabled(True)

    def on_format_click(self, button):
        self.updateLimits()

    def updateLimits(self):
        tileFormat = self.getRawFormat()
        tileSize = tileFormat.width * tileFormat.height * len(tileFormat.bitsOffs)
        self.maxTiles = 8 * self.fileSize / (tileFormat.width * tileFormat.height * len(tileFormat.bitsOffs))

    def get_tiles(self):
        filename = self.filename
        numTiles = min(self.numTilesSpinBox.value(), self.maxTiles)
        fileOffset = min(self.fileOffsetSpinBox.value(), os.path.getsize(filename))
        return readTiles(filename, numTiles, self.getRawFormat(), fileOffset)


class ExportRawTilesDialog(RawTilesDialog):
    def __init__(self, settings):
        super(ExportRawTilesDialog, self).__init__(settings)
        self.setWindowTitle('Export RAW tiles')
        # File selection
        self.fileChooserButton.setText('Export to file...')
        self.fileChooserButton.clicked.connect(self.saveFile)
        # Ok/Cancel buttons
        buttonBox = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        buttonBox.accepted.connect(self.accept)
        buttonBox.rejected.connect(self.reject)
        self.buttonOk = buttonBox.button(QDialogButtonBox.Ok)
        self.buttonOk.setEnabled(False)
        self.formLayout.addWidget(buttonBox)
        self.show()

    def saveFile(self, filechooserbutton):
        filename, fileType = QFileDialog.getSaveFileName(parent=self,
                                                         caption='Save RAW data file')
        if filename != '':
            self.filename = filename
            self.buttonOk.setEnabled(True)

    def write_tiles(self, tileData, numTiles):
        writeTiles(self.filename, tileData, numTiles, self.getRawFormat())


def readTiles(filename, numTiles, tileFormat, fileOffset=0):
    byteData = array.array('B', [])
    byteData.fromfile(open(filename, 'rb'), os.path.getsize(filename))
    tileDataSrc = array.array('B', [0] * numTiles * tileFormat.stride)

    # Expand to 1 entry per bit
    for i in range(len(tileDataSrc)):
        tileDataSrc[i] = (byteData[fileOffset + int(i / 8)] >> int(i % 8)) & 1

    tileDataDst = array.array('B', [0] * tileFormat.width * tileFormat.height * numTiles)
    for i in range(numTiles):
        for y in range(tileFormat.height):
            for x in range(tileFormat.width):
                c = 0
                for n in range(len(tileFormat.bitsOffs)):
                    c |= tileDataSrc[i * tileFormat.stride + tileFormat.bitsOffs[n][y * tileFormat.width + x]] << n
                tileDataDst[i * tileFormat.width * tileFormat.height + y * tileFormat.width + x] = c
    return tileDataDst, numTiles, tileFormat.width, tileFormat.height


def writeTiles(filename, tileData, numTiles, tileFormat):
    tileWidth, tileHeight = tileFormat.width, tileFormat.height
    rawData = array.array('B', [0] * ((numTiles * tileFormat.stride) // 8))
    with open(filename, "wb") as f:
        for i in range(numTiles):
            for y in range(tileHeight):
                for x in range(tileWidth):
                    color = tileData[tileWidth * tileHeight * i + tileWidth * y + x]
                    for bitIndex, bitsOffs in enumerate(tileFormat.bitsOffs):
                        bitsOffs = bitsOffs[y * tileWidth + x]
                        bytePos = int(tileFormat.stride * i + bitsOffs) >> 3
                        bitPos = int(tileFormat.stride * i + bitsOffs) & 7
                        rawData[bytePos] |= ((color >> bitIndex) & 1) << bitPos
        rawData.tofile(f)


#
# Run as stand-alone program
#
if __name__ == "__main__":
    app = QApplication(sys.argv)
    dialog = ImportRawTilesDialog()
    sys.exit(app.exec_())
