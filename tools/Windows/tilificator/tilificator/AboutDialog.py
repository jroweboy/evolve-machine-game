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

import sys

from PySide2.QtGui import QDesktopServices, QPixmap
from PySide2.QtWidgets import QLabel, QPushButton, QDialog, QTextBrowser, QHBoxLayout, QVBoxLayout, QApplication
from PySide2.QtCore import Qt, QUrl, QTimer


class AboutDialog(QDialog):
    def __init__(self):
        self.logoCounter = 0
        super(AboutDialog, self).__init__()
        self.setWindowTitle('About Tilificator')
        # Logo
        self.logo = [QPixmap('tilificatorlogo0.png'),
                     QPixmap('tilificatorlogo1.png')]
        if (not self.logo[0].isNull()) and (not self.logo[1].isNull()):
            # Use flickering image logo if available
            self.logoCounter = 0
            self.logoTimer = QTimer(self)
            self.logoTimer.setInterval(20)
            self.logoTimer.timeout.connect(self.updateLogo)
            self.logoTimer.start()
            self.labelLogo = QLabel(self)
            self.labelLogo.setPixmap(self.logo[0])
        else:
            # Revert to boring text logo
            self.labelLogo = QLabel('<h1><b>Tilificator<b><h1>', self)
        # Version
        labelVersion = QLabel('Version: {}'.format(self.getVersionString()), self)
        # Web link
        self.website = 'https://sourceforge.net/projects/tilificator/'
        linkText = '<a href="{0}">{0}</a>'.format(self.website)
        labelLink = QLabel(self)
        labelLink.setTextFormat(Qt.RichText)
        labelLink.linkActivated.connect(self.onWebsiteClicked)  # labelLink.setOpenExternalLinks(True)
        labelLink.setText(linkText)
        # Copyright
        labelCopyright = QLabel('Copyright (c) 2011-2019 Michel Iwaniec', self)
        # License/Close buttons
        buttonLicense = QPushButton('License', self)
        buttonLicense.clicked.connect(self.onLicenseClicked)
        buttonClose = QPushButton('Close', self)
        buttonClose.clicked.connect(lambda: self.done(0))
        buttonLayout = QHBoxLayout()
        buttonLayout.addWidget(buttonLicense)
        buttonLayout.addStretch()
        buttonLayout.addWidget(buttonClose)
        # layout
        layout = QVBoxLayout()
        layout.addWidget(self.labelLogo)
        layout.addWidget(labelVersion)
        layout.addWidget(labelCopyright)
        layout.addWidget(labelLink)
        layout.addLayout(buttonLayout)
        self.setLayout(layout)
        self.show()

    def onWebsiteClicked(self):
        # TODO: Figure out why openUrl does not work at all
        QDesktopServices.openUrl(QUrl(self.website))

    def onLicenseClicked(self):
        licenseWindow = QDialog(self)
        textBrowser = QTextBrowser()
        textBrowser.setText(open('COPYING').read())
        layout = QVBoxLayout()
        layout.addWidget(textBrowser)
        licenseWindow.setLayout(layout)
        licenseWindow.resize(500, 600)
        licenseWindow.setWindowTitle('License')
        licenseWindow.show()

    @staticmethod
    def getVersionString():
        try:
            from versioning import VERSION_STRING
        except:
            VERSION_STRING = 'unknown'
        return VERSION_STRING

    def updateLogo(self):
        self.labelLogo.setPixmap(self.logo[self.logoCounter])
        self.logoCounter = (self.logoCounter + 1) & 1
        return True


#
# Run as stand-alone program
#
if __name__ == "__main__":
    app = QApplication(sys.argv)
    aboutDialog = AboutDialog()
    sys.exit(app.exec_())
