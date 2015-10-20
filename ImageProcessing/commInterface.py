import serial
import time
import pandas
import datetime

crcTable = []

def calcCRC(message):
    
    poly = 0x1021
    
    for i in range(256):
        crc = 0
        c = i << 8
        for j in range(8):
            if (crc ^ c) & 0x8000:
                crc = (crc << 1) ^ poly
            else:
                crc = crc << 1
            c = c << 1
        crcTable.append(crc)

    crc = 0xFFFF

    for byte in message:
        tmp = (ord(byte) ^ (crc >> 8)) & 0xFF
        crc = ((crc << 8) ^ crcTable[tmp]) & 0xFFFF

    return crc

class HerbertSerial:
    baudrate = 115200

    def __init__(self, port):
        self.serialPortConnection          = serial.Serial()
        self.serialPortConnection.baudrate = self.baudrate
        self.serialPortConnection.port     = port
        self.serialPortConnection.timeout  = 1
        self.serialPortConnection.open()

    def sendPacket(self, packet):
        self.serialPortConnection.write(packet)
        return self.serialPortConnection.readline()

    def close(self):
        self.serialPortConnection.close()

def crcCheck(responsePacket):
    receivedCrc = responsePacket.strip('\r\n').strip(']').split(' ')[-1]
    expectedCrc = calcCRC(responsePacket[1:].strip(']').strip(receivedCrc).strip(' '))

    if str(expectedCrc) == receivedCrc:
        print "CRC is valid"
    else:
        print "CRC is invalid: Expected - " + str(expectedCrc) + " Actual - " +  receivedCrc


def sendCommand(comPort, commandPacket):
    try:
        HerbertSerialConnection = HerbertSerial(comPort)
        responsePacket = HerbertSerialConnection.sendPacket(str(commandPacket))
    finally:
        HerbertSerialConnection.close()
    return responsePacket

def generateFullCommandPacket(preliminaryPacket, sequenceNumber):

    splitPrelimPacket = str(preliminaryPacket).split(' ')
    parameters = []
    for param in splitPrelimPacket:
        parameters.append(param)

    if len(parameters) == 0:
        return

    Delimiter = " "
    packet = "MH " + format(sequenceNumber, '02') + Delimiter + parameters.pop(0) 
    for param in parameters:
        packet += Delimiter + param
    packet = packet 
    packet = "[" + packet + Delimiter + str(calcCRC(packet)) + "]" 
    return packet 
