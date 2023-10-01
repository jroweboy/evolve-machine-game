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
import array

from PIL import Image

from PySide6.QtCore import Signal, Slot
from PySide6.QtWidgets import QApplication, QDialog, QDialogButtonBox, QGroupBox, QScrollArea, QHBoxLayout, QVBoxLayout
from PySide6.QtCore import Qt

from tilificator.sprite import SpriteImage

from tilificator.common import makeRisingEdge, rectangleInsideRectangle
from tilificator.rectangles_widget import RectanglesWidget
from tilificator.rectangles_widget import Rectangle


class CutSpriteSheetDialog(QDialog):
    def __init__(self, image, parent=None):
        super(CutSpriteSheetDialog, self).__init__(parent)
        self.setWindowTitle('Cut sprite sheet')
        self.bg = 0
        self.rectanglesWidget = RectanglesWidget(image=image, color=(0.8, 0.0, 0.0), selectedColor=(0.7, 0.7, 0.0))
        self.createBoundingBoxes()
        # ScrollArea
        scrollArea = QScrollArea(self)
        scrollArea.setWidget(self.rectanglesWidget)
        # Ok/Cancel buttons
        buttonBox = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        buttonBox.accepted.connect(self.accept)
        buttonBox.rejected.connect(self.reject)
        # Use VBox layout
        layout = QVBoxLayout()
        layout.addWidget(scrollArea)
        layout.addWidget(buttonBox)
        self.setLayout(layout)
        self.resize(600, 600)
        self.setModal(True)
        self.show()

    def redraw(self):
        self.rectanglesWidget.redraw()

    def blankLine(self, y):
        imageData = self.rectanglesWidget.image.getdata()
        imageWidth, imageHeight = self.rectanglesWidget.image.size
        for x in range(imageWidth):
            if imageData[imageWidth * y + x] != self.bg:
                return False
        return True

    def splitBoundingBoxesIntoRows(self, boundingBoxes):
        rowLines = []
        imageWidth, imageHeight = self.rectanglesWidget.image.size
        lines = [1 if not self.blankLine(y) else 0 for y in range(imageHeight)]
        newRow = makeRisingEdge(lines)
        rowLines = [i for i in range(imageHeight) if newRow[i]]
        rowLines.append(imageHeight)

        rows = []
        for i in range(len(rowLines) - 1):
            rows.append([bbox for bbox in boundingBoxes if rowLines[i] <= bbox.y < rowLines[i + 1]])
        sortedBoundingBoxes = []

        for row in rows:
            sortedBoundingBoxes.extend(sorted(row, key=lambda bbox: bbox.x))

        return sortedBoundingBoxes

    def createBoundingBoxes(self):
        labelData = self.labelImage(self.rectanglesWidget.image)
        numLabels = max(labelData)

        labelExists = [False] * numLabels

        imageWidth, imageHeight = self.rectanglesWidget.image.size
        labelMinX = [imageWidth - 1] * numLabels
        labelMinY = [imageHeight - 1] * numLabels
        labelMaxX = [0] * numLabels
        labelMaxY = [0] * numLabels
        for y in range(imageHeight):
            for x in range(imageWidth):
                label = labelData[y * imageWidth + x]
                if label != 0:
                    labelMinX[label - 1] = min([labelMinX[label - 1], x])
                    labelMinY[label - 1] = min([labelMinY[label - 1], y])
                    labelMaxX[label - 1] = max([labelMaxX[label - 1], x])
                    labelMaxY[label - 1] = max([labelMaxY[label - 1], y])
                    labelExists[label - 1] = True

        for i in range(numLabels):
            if labelExists[i]:
                self.rectanglesWidget.rectangles.append(Rectangle(labelMinX[i], labelMinY[i], labelMaxX[i] - labelMinX[i] + 1, labelMaxY[i] - labelMinY[i] + 1))

        # Remove bounding boxes which are fully contained within another
        finalBoxes = []
        for bbox in self.rectanglesWidget.rectangles:
            testRectangles = [r for r in self.rectanglesWidget.rectangles if r != bbox]
            if not [r for r in testRectangles if rectangleInsideRectangle(bbox, r)]:
                finalBoxes.append(bbox)

        self.rectanglesWidget.rectangles = finalBoxes

    def labelImage(self, image):
        data = image.getdata()
        imageWidth, imageHeight = image.size
        self.bg = data[0]
        labelData = array.array('L', [0] * imageWidth * imageHeight)
        linked = [set([])]
        currentLabel = 1
        # First pass (generate labels)
        for y in range(imageHeight):
            for x in range(imageWidth):
                c = data[y * imageWidth + x]
                if c != self.bg:
                    #currentLabel = len(labels)-1
                    neighbors = []
                    if x > 0 and data[y * imageWidth + x - 1] != self.bg:
                        neighbors.append(labelData[y * imageWidth + x - 1])
                    if x > 0 and y > 0 and data[(y - 1) * imageWidth + x - 1] != self.bg:
                        neighbors.append(labelData[(y - 1) * imageWidth + x - 1])
                    if y > 0 and data[(y - 1) * imageWidth + x] != self.bg:
                        neighbors.append(labelData[(y - 1) * imageWidth + x])
                    if x < imageWidth - 1 and y > 0 and data[(y - 1) * imageWidth + x + 1] != self.bg:
                        neighbors.append(labelData[(y - 1) * imageWidth + x + 1])

                    if not neighbors:
                        labelData[y * imageWidth + x] = currentLabel
                        linked.append(set([currentLabel]))
                        currentLabel += 1
                    else:
                        lowestLabel = min(neighbors)
                        labelData[y * imageWidth + x] = lowestLabel
                        for label in neighbors:
                            linked[label] = linked[label] | set(neighbors)

        # for i, s in enumerate(linked):
        #    line = " label %d = [" % i
        #    for e in s:
        #        line += str(e) + ','
        #    line += ']'
        #    if len(s) != 0:
        #        line += ', min = %d' % min(s)
        #    print line

        # Second pass (remove duplicate labels)
        for y in range(imageHeight):
            for x in range(imageWidth):
                label = labelData[y * imageWidth + x]
                if label != 0:
                    while label != min(linked[label]):
                        label = min(linked[label])
                    labelData[y * imageWidth + x] = label

        return labelData

    def getImages(self):
        boundingBoxes = self.splitBoundingBoxesIntoRows(self.rectanglesWidget.rectangles)
        images = []
        for i, r in enumerate(boundingBoxes):
            imageCopy = self.rectanglesWidget.image.copy()
            images.append(imageCopy.crop((r.x, r.y, r.x + r.w, r.y + r.h)))
        return images


#
# Run as stand-alone program
#
if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: cut_spritesheet.py <image.png>")
        sys.exit(2)

    image = Image.open(sys.argv[1])
    if image.mode != 'P':
        print("Error: Only 8-bit paletted images are supported")
        sys.exit(3)

    app = QApplication(sys.argv)
    dialog = CutSpriteSheetDialog(image)
    response = dialog.exec_()

    if response == QDialog.Accepted:
        name = sys.argv[1].split('.')[0]
        for i, image in enumerate(dialog.getImages()):
            image.save(name + str(i) + ".png")

    sys.exit(app.exec_())
