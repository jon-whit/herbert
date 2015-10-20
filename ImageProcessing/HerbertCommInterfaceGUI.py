# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file '..\fruManualCommandGUI.ui'
#
# Created: Fri Sep 25 09:32:40 2015
#      by: PyQt4 UI code generator 4.10.4
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui
import commInterface
import sys
import serial
import itertools
import _winreg as winreg

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    def _fromUtf8(s):
        return s

try:
    _encoding = QtGui.QApplication.UnicodeUTF8
    def _translate(context, text, disambig):
        return QtGui.QApplication.translate(context, text, disambig, _encoding)
except AttributeError:
    def _translate(context, text, disambig):
        return QtGui.QApplication.translate(context, text, disambig)

class Ui_Dialog(object):

    sequenceNumber = 0;
    def sendButtonHandler(self):
        commandPacket = commInterface.generateFullCommandPacket(self.commandLineEdit.text(), sequenceNumber = self.sequenceNumber) 
        responsePacket = commInterface.sendCommand(comPort = str(self.comPortComboBox.currentText()), commandPacket = commandPacket)
        if(self.sequenceNumber >= 100):
            self.sequenceNumber = 0
        self.sequenceNumber += 1
        self.commandPacketLabel.setText(commandPacket)
        self.responsePacketLabel.setText(responsePacket)
        print commInterface.crcCheck(responsePacket)

    def enumerate_serial_ports(self):
        comports = []
        path = 'HARDWARE\\DEVICEMAP\\SERIALCOMM'
        try:
            key = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, path)
        except WindowsError:
            raise IterationError
            crcCheck
        for i in itertools.count():
          try:
            val = winreg.EnumValue(key, i)
            comports.append(str(val[1]))
          except EnvironmentError:
            break   
        self.comPortComboBox.addItems(comports)

    def setupUi(self, Dialog):
        Dialog.setObjectName(_fromUtf8("Dialog"))
        Dialog.resize(496, 174)
        self.buttonBox = QtGui.QDialogButtonBox(Dialog)
        self.buttonBox.setGeometry(QtCore.QRect(140, 130, 341, 32))
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Ok)
        self.buttonBox.setObjectName(_fromUtf8("buttonBox"))
        self.comPortComboBox = QtGui.QComboBox(Dialog)
        self.comPortComboBox.setGeometry(QtCore.QRect(20, 30, 69, 22))
        self.comPortComboBox.setObjectName(_fromUtf8("comPortComboBox"))
        self.comPortLabel = QtGui.QLabel(Dialog)
        self.comPortLabel.setGeometry(QtCore.QRect(20, 10, 61, 16))
        self.comPortLabel.setObjectName(_fromUtf8("comPortLabel"))
        self.commandLineEdit = QtGui.QLineEdit(Dialog)
        self.commandLineEdit.setGeometry(QtCore.QRect(120, 30, 281, 20))
        self.commandLineEdit.setObjectName(_fromUtf8("commandLineEdit"))
        self.sendCommandPushButton = QtGui.QPushButton(Dialog)
        self.sendCommandPushButton.setGeometry(QtCore.QRect(410, 30, 75, 23))
        self.sendCommandPushButton.setObjectName(_fromUtf8("sendCommandPushButton"))
        self.exampleLabel = QtGui.QLabel(Dialog)
        self.exampleLabel.setGeometry(QtCore.QRect(120, 10, 271, 16))
        self.exampleLabel.setObjectName(_fromUtf8("exampleLabel"))
        self.commandPacketTitleLabel = QtGui.QLabel(Dialog)
        self.commandPacketTitleLabel.setGeometry(QtCore.QRect(20, 80, 100, 20))
        self.commandPacketTitleLabel.setObjectName(_fromUtf8("commandPacketTitleLabel"))
        self.responsePacketTitleLabel = QtGui.QLabel(Dialog)
        self.responsePacketTitleLabel.setGeometry(QtCore.QRect(20, 120, 100, 20))
        self.responsePacketTitleLabel.setObjectName(_fromUtf8("responsePacketTitleLabel"))
        self.commandPacketLabel = QtGui.QLabel(Dialog)
        self.commandPacketLabel.setGeometry(QtCore.QRect(120, 80, 200, 30))
        self.commandPacketLabel.setText(_fromUtf8(""))
        self.commandPacketLabel.setObjectName(_fromUtf8("commandPacket"))
        self.responsePacketLabel = QtGui.QLabel(Dialog)
        self.responsePacketLabel.setGeometry(QtCore.QRect(120, 120, 200, 30))
        self.responsePacketLabel.setText(_fromUtf8(""))
        self.responsePacketLabel.setObjectName(_fromUtf8("responsePacket"))

        self.enumerate_serial_ports()

        self.retranslateUi(Dialog)
        #QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("accepted()")), Dialog.accept)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("rejected()")), Dialog.reject)
        QtCore.QObject.connect(self.sendCommandPushButton, QtCore.SIGNAL(_fromUtf8("clicked()")), self.sendButtonHandler)
        QtCore.QMetaObject.connectSlotsByName(Dialog)

    def retranslateUi(self, Dialog):
        Dialog.setWindowTitle(_translate("Dialog", "Dialog", None))
        self.comPortLabel.setText(_translate("Dialog", "COM PORT", None))
        self.sendCommandPushButton.setText(_translate("Dialog", "Send", None))
        self.commandPacketTitleLabel.setText(_translate("Dialog", "Command Packet:", None))
        self.exampleLabel.setText(_translate("Dialog", "Command Code Followed by Parameters. Example Execute Move R", None))
        self.responsePacketTitleLabel.setText(_translate("Dialog", "Response Packet:", None))

if __name__ == "__main__":
    import sys
    app = QtGui.QApplication(sys.argv)
    Dialog = QtGui.QDialog()
    ui = Ui_Dialog()
    ui.setupUi(Dialog)
    Dialog.show()
    sys.exit(app.exec_())