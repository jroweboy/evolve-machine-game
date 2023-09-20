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

from PySide2.QtWidgets import QApplication, QDialog, QDialogButtonBox, QDoubleSpinBox, QComboBox, QListWidget, QFormLayout, QCheckBox
from PySide2.QtCore import Qt

from tilificator.tilificator import OptimizationSettings


class OptimizationSettingsDialog(QDialog):
    def __init__(self, settings, parent=None):
        super(OptimizationSettingsDialog, self).__init__(parent)
        self.setWindowTitle('Optimization settings')
        # Create a ComboBox for selecting between different tiling methods
        self.tilingMethodCombo = QComboBox()
        self.tilingMethodCombo.addItem('Rect')
        self.tilingMethodCombo.addItem('ShiftedRows')
        self.tilingMethodCombo.addItem('ShiftedRowsNoOverlap')
        self.tilingMethodCombo.addItem('DragQueen')
        self.tilingMethodCombo.addItem('FullMatch')
        self.tilingMethodCombo.setCurrentText(settings.tilingMethod)
        # Create a list widget to handle reordering optimization goals
        self.listWidgetOptGoals = QListWidget(self)
        self.listWidgetOptGoals.setDragDropMode(QListWidget.DragDrop)
        self.listWidgetOptGoals.setDefaultDropAction(Qt.MoveAction)
        # SpinBox for "Max sprites/scanline limit" parameter
        self.spinBoxMaximumBeyondLimit = QDoubleSpinBox()
        self.spinBoxMaximumBeyondLimit.setRange(1, 64)
        self.spinBoxMaximumBeyondLimit.setSingleStep(1)
        # SpinBox for "Average sprites/scanline limit" parameter
        self.spinBoxAverageBeyondLimit = QDoubleSpinBox()
        self.spinBoxAverageBeyondLimit.setRange(1, 64)
        self.spinBoxAverageBeyondLimit.setSingleStep(1)
        # SpinBox for solid factor exponent
        self.spinBoxSolidFactorExponent = QDoubleSpinBox()
        self.spinBoxSolidFactorExponent.setRange(0.0, 2.0)
        self.spinBoxSolidFactorExponent.setSingleStep(0.1)
        # Checkbox for enabling global optimization
        self.checkBoxGlobalOptimization = QCheckBox()
        # Ok/Cancel buttons
        buttonBox = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        buttonBox.accepted.connect(self.accept)
        buttonBox.rejected.connect(self.reject)
        # Put in a form layout
        layout = QFormLayout()
        layout.addRow('Tiling method', self.tilingMethodCombo)
        layout.addRow('Priorities of optimization goals', self.listWidgetOptGoals)
        layout.addRow('Maximum sprites/scanline limit', self.spinBoxMaximumBeyondLimit)
        layout.addRow('Average sprites/scanline limit', self.spinBoxAverageBeyondLimit)
        layout.addRow('Enable global optimization', self.checkBoxGlobalOptimization)
        layout.addRow('Global optimization solidfactor exponent', self.spinBoxSolidFactorExponent)
        layout.addWidget(buttonBox)
        self.setLayout(layout)
        self.setOptimizationSettings(settings)
        self.setModal(True)
        self.show()

    def setOptimizationSettings(self, settings):
        self.tilingMethodCombo.setCurrentText(settings.tilingMethod)
        self.listWidgetOptGoals.clear()
        self.listWidgetOptGoals.addItems(settings.priorities)
        self.spinBoxMaximumBeyondLimit.setValue(settings.spritesScanlineMaxLimit)
        self.spinBoxAverageBeyondLimit.setValue(settings.spritesScanlineAvgLimit)
        self.checkBoxGlobalOptimization.setCheckState(Qt.Checked if settings.useGlobalOptimization else Qt.Unchecked)
        self.spinBoxSolidFactorExponent.setValue(settings.solidFactorExponent)

    def getOptimizationSettings(self):
        settings = OptimizationSettings()
        settings.tilingMethod = self.tilingMethodCombo.currentText()
        settings.priorities = [self.listWidgetOptGoals.item(i).text() for i in range(self.listWidgetOptGoals.count())]
        settings.spritesScanlineMaxLimit = self.spinBoxMaximumBeyondLimit.value()
        settings.spritesScanlineAvgLimit = self.spinBoxAverageBeyondLimit.value()
        settings.useGlobalOptimization = self.checkBoxGlobalOptimization.isChecked()
        settings.solidFactorExponent = self.spinBoxSolidFactorExponent.value()
        return settings


#
# Run as stand-alone program
#
if __name__ == "__main__":
    app = QApplication(sys.argv)
    optimizationSettingsDialog = OptimizationSettingsDialog(OptimizationSettings())
    sys.exit(app.exec_())
