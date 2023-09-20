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

from PySide2.QtWidgets import QApplication, QDialog, QDialogButtonBox, QSpinBox, QFormLayout

from tilificator.tile import TileSettings


class TileSettingsDialog(QDialog):
    def __init__(self, settings, parent=None):
        super(TileSettingsDialog, self).__init__(parent)
        self.setWindowTitle("Tile settings")
        # SpinBoxes for tile width/height
        self.spinBoxTileWidth = QSpinBox(self)
        self.spinBoxTileWidth.setRange(8, 64)
        self.spinBoxTileWidth.setSingleStep(1)
        self.spinBoxTileHeight = QSpinBox(self)
        self.spinBoxTileHeight.setRange(8, 64)
        self.spinBoxTileHeight.setSingleStep(1)
        # SpinBoxes for color size
        self.spinBoxColorSize = QSpinBox(self)
        self.spinBoxColorSize.setRange(2, 256)
        # Ok/Cancel buttons
        buttonBox = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        buttonBox.accepted.connect(self.accept)
        buttonBox.rejected.connect(self.reject)
        # Put in a form layout
        layout = QFormLayout()
        layout.addRow('Tile width', self.spinBoxTileWidth)
        layout.addRow('Tile height', self.spinBoxTileHeight)
        layout.addRow('Color size', self.spinBoxColorSize)
        layout.addWidget(buttonBox)
        self.setLayout(layout)
        self.setTileSettings(settings)
        self.setModal(True)
        self.show()

    def setTileSettings(self, settings):
        self.spinBoxTileWidth.setValue(settings.tileWidth)
        self.spinBoxTileHeight.setValue(settings.tileHeight)
        self.spinBoxColorSize.setValue(settings.colorSize)

    def getTileSettings(self):
        settings = TileSettings()
        settings.tileWidth = self.spinBoxTileWidth.value()
        settings.tileHeight = self.spinBoxTileHeight.value()
        settings.colorSize = self.spinBoxColorSize.value()
        return settings


#
# Run as stand-alone program
#
if __name__ == "__main__":
    app = QApplication(sys.argv)
    tileSettingsDialog = TileSettingsDialog(TileSettings())
    sys.exit(app.exec_())
