/*
  fbus.cpp - Library for talking to an F-Bus device.
  Created by Charles Pax for Pax Instruments, 2015-05-23
  Please visit http://paxinstruments.com/products/
  Released into the Public Domain

  Call this by passing the pointer to a serial port:
  FBus fred(&altSerial);
*/

#include "Arduino.h"
#include "FBus.h"

FBus::FBus(Stream *serialPort) {
    _serialPort = serialPort;
}

FBus::FBus(Stream *serialPort, int SMSCenter) {
    _serialPort = serialPort;
    _SMSCenter = SMSCenter;
}

void FBus::initializeBus() {// Perpares phone to receice F-Bus messages
    for (int i = 0; i < 128; i++) {
        _serialPort->write(0x55);
    }
}

String FBus::softwareVersion() {
    initializeBus();
    delay(100);
    byte hwsw[] = { 0x1E, 0x00, 0x0C, 0xD1, 0x00, 0x07, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x60, 0x00, 0x72, 0xD5 }; // get HW and SW info
    _serialPort->write(hwsw,sizeof(hwsw));
    char incomingMessage[200];
    delay(50);
    for (int i = 0; _serialPort->available() > 0; i++) {
        char incomingByte = _serialPort->read();
        if (incomingByte == 'V') {
            i = 0;
        }
        incomingMessage[i] = incomingByte;
    }
    char softwareVersion[5];
    for (int j = 0; j < sizeof(softwareVersion); j++) {
        softwareVersion[j] = incomingMessage[j+2];
    }
    return softwareVersion;
}

String FBus::hardwareVersion() {
    initializeBus();
    delay(100);
    byte hwsw[] = { 0x1E, 0x00, 0x0C, 0xD1, 0x00, 0x07, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x60, 0x00, 0x72, 0xD5 }; // get HW and SW info
    _serialPort->write(hwsw,sizeof(hwsw));
    char incomingMessage[200];
    delay(50);
    for (int i = 0; _serialPort->available() > 0; i++) {
        char incomingByte = _serialPort->read();
        if (incomingByte == 'V') {
            i = 0;
        }
        incomingMessage[i] = incomingByte;
    }
    char hardwareVersion[5];
    for (int j = 0; j < sizeof(hardwareVersion); j++) {
        hardwareVersion[j] = incomingMessage[j+22];
    }
    return hardwareVersion;
}

String FBus::dateCode() {
    initializeBus();
    delay(100);
    byte hwsw[] = { 0x1E, 0x00, 0x0C, 0xD1, 0x00, 0x07, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x60, 0x00, 0x72, 0xD5 }; // get HW and SW info
    _serialPort->write(hwsw,sizeof(hwsw));
    char incomingMessage[200];
    delay(50);
    for (int i = 0; _serialPort->available() > 0; i++) {
        char incomingByte = _serialPort->read();
        if (incomingByte == 'V') {
            i = 0;
        }
        incomingMessage[i] = incomingByte;
    }
    char dateCode[8];
    for (int j = 0; j < sizeof(dateCode); j++) {
        dateCode[j] = incomingMessage[j+16];
    }
    return dateCode;
}

char FBus::checksumOdd() {
  // Go through the message array and outout an XOR of the odd numbered bytes.
  // The first byte in the array is odd. This shoudl work both sending and receiving.
}

char FBus::checksumEven() {
  // Go through the message array and outout an XOR of the even numbered bytes.
  // The first byte in the array is odd. This shoudl work both sending and receiving.
  
}


