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

from PySide6.QtWidgets import QApplication, QDialog, QFileDialog, QDialogButtonBox, QPushButton, QRadioButton, QSpinBox, QGroupBox, QHBoxLayout, QVBoxLayout, QFormLayout, QLabel, QLineEdit, QTextEdit
from PySide6.QtCore import Qt

from tilificator.sprite import SpriteImage

from tilificator.spritetile import SpriteTile, Tilification
from tilificator.array2d import array2d


class ExportMetaspritesAsmDialog(QDialog):
    def __init__(self, metaSpriteBeginFormatting, spriteFormatting, metaSpriteEndFormatting, spriteImages):
        super().__init__()
        self.spriteImages = spriteImages
        self.filename = None
        # Formatting strings
        self.metaSpriteBeginFormatterLineEdit = QLineEdit()
        self.spriteFormatterLineEdit = QLineEdit()
        self.metaSpriteEndFormatterLineEdit = QLineEdit()
        self.formLayout = QFormLayout()
        self.formLayout.addRow('MetaSprite begin text format', self.metaSpriteBeginFormatterLineEdit)
        self.metaSpriteBeginFormatterLineEdit.setText(metaSpriteBeginFormatting)
        self.metaSpriteBeginFormatterLineEdit.textChanged.connect(self.formattingChanged)
        self.formLayout.addRow('Sprite text format', self.spriteFormatterLineEdit)
        self.spriteFormatterLineEdit.setText(spriteFormatting)
        self.spriteFormatterLineEdit.textChanged.connect(self.formattingChanged)
        self.formLayout.addRow('MetaSprite end text format', self.metaSpriteEndFormatterLineEdit)
        self.metaSpriteEndFormatterLineEdit.setText(metaSpriteEndFormatting)
        self.metaSpriteEndFormatterLineEdit.textChanged.connect(self.formattingChanged)
        # Add spin boxes to control offset for x / y / tile index / palette
        self.metaSpriteXOffsetSpinBox = QSpinBox()
        self.metaSpriteXOffsetSpinBox.setRange(-256, 256)
        self.formLayout.addRow('X offset', self.metaSpriteXOffsetSpinBox)
        self.metaSpriteXOffsetSpinBox.valueChanged.connect(self.formattingChanged)
        self.metaSpriteYOffsetSpinBox = QSpinBox()
        self.metaSpriteYOffsetSpinBox.setRange(-256, 256)
        self.formLayout.addRow('Y offset', self.metaSpriteYOffsetSpinBox)
        self.metaSpriteYOffsetSpinBox.valueChanged.connect(self.formattingChanged)
        self.metaSpriteTileIndexOffsetSpinBox = QSpinBox()
        self.metaSpriteTileIndexOffsetSpinBox.setRange(-256, 256)
        self.formLayout.addRow('Tile index offset', self.metaSpriteTileIndexOffsetSpinBox)
        self.metaSpriteTileIndexOffsetSpinBox.valueChanged.connect(self.formattingChanged)
        self.metaSpritePaletteOffsetSpinBox = QSpinBox()
        self.metaSpritePaletteOffsetSpinBox.setRange(-256, 256)
        self.formLayout.addRow('Palette offset', self.metaSpritePaletteOffsetSpinBox)
        self.metaSpritePaletteOffsetSpinBox.valueChanged.connect(self.formattingChanged)
        # Add preview
        self.previewGroupBox = QGroupBox('Preview')
        self.previewTextEdit = QTextEdit()
        self.previewLayout = QVBoxLayout()
        self.previewLayout.addWidget(self.previewTextEdit)
        self.previewGroupBox.setLayout(self.previewLayout)
        self.vboxLayout = QVBoxLayout()
        self.vboxLayout.addLayout(self.formLayout)
        self.vboxLayout.addWidget(self.previewGroupBox)
        self.clipboardButton = QPushButton('Copy to clipboard')
        self.clipboardButton.clicked.connect(self.copyToClipboard)
        self.fileChooserButton = QPushButton('Write to file...')
        self.fileChooserButton.clicked.connect(self.writeToFile)
        self.hboxLayout = QHBoxLayout()
        self.hboxLayout.addWidget(self.clipboardButton)
        self.hboxLayout.addWidget(self.fileChooserButton)
        self.hboxLayout.addStretch()
        self.vboxLayout.addLayout(self.hboxLayout)
        self.setLayout(self.vboxLayout)
        self.resize(800, 650)
        self.formattingChanged()
        self.setModal(True)

    def formattingChanged(self):
        metaSpriteBeginFormatterString = self.metaSpriteBeginFormatterLineEdit.text().replace('\\n', '\n').replace('\\t', '\t')
        spriteFormatterString = self.spriteFormatterLineEdit.text().replace('\\n', '\n').replace('\\t', '\t')
        metaSpriteEndFormatterString = self.metaSpriteEndFormatterLineEdit.text().replace('\\n', '\n').replace('\\t', '\t')
        try:
            lines = []
            for metaSpriteIndex, si in enumerate(self.spriteImages):
                numSprites = len(si.tilification.tiles)
                lines.append(metaSpriteBeginFormatterString.format(metaSpriteIndex=metaSpriteIndex, numSprites=numSprites))
                for spriteIndex, st in enumerate(si.tilification.tiles):
                    x = st.x + self.metaSpriteXOffsetSpinBox.value()
                    y = st.y + self.metaSpriteYOffsetSpinBox.value()
                    tileIndex = st.tileID + self.metaSpriteTileIndexOffsetSpinBox.value()
                    p = (st.colorDataHi >> 2) + self.metaSpritePaletteOffsetSpinBox.value()
                    lines.append(spriteFormatterString.format(x=x,
                                                              xL=(x & 0xFF),
                                                              xH=((x >> 8) & 0xFF),
                                                              y=y,
                                                              yL=(y & 0xFF),
                                                              yH=((y >> 8) & 0xFF),
                                                              t=tileIndex,
                                                              tL=(tileIndex & 0xFF),
                                                              tH=((tileIndex >> 8) & 0xFF),
                                                              V=int(st.flipV),
                                                              H=int(st.flipH),
                                                              p=p,
                                                              pL=(p & 0xFF),
                                                              pH=((p >> 8) & 0xFF),
                                                              a=(int(st.flipV) << 7) | (int(st.flipH) << 6) | (p & 0x3),
                                                              spriteIndex=spriteIndex,
                                                              metaSpriteIndex=metaSpriteIndex,
                                                              numSprites=numSprites))
                lines.append(metaSpriteEndFormatterString.format(metaSpriteIndex=metaSpriteIndex, numSprites=numSprites))
            textOutput = ''.join(lines)
            self.previewTextEdit.setText(textOutput)
            self.previewTextEdit.setEnabled(True)
            self.fileChooserButton.setEnabled(True)
            self.clipboardButton.setEnabled(True)
        except:
            self.previewTextEdit.setEnabled(False)
            self.fileChooserButton.setEnabled(False)
            self.clipboardButton.setEnabled(False)

    def copyToClipboard(self):
        clipboard = QApplication.clipboard()
        clipboard.setText(self.previewTextEdit.toPlainText())

    def writeToFile(self):
        filename, fileType = QFileDialog.getSaveFileName(parent=self,
                                                         caption='Save images as .asm',
                                                         filter='Assembly files (*.asm)')
        if filename != '':
            with open(filename, 'wt') as f:
                textOutput = self.previewTextEdit.toPlainText()
                f.write(textOutput)
