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

void FBus::prepareThing() {  // Perpares phone to receice F-Bus messages
  for (int i = 0; i < 128; i++) {
    _serialPort->write(0x55);
  }
}

String FBus::softwareVersion() {
  prepareThing();
  delay(100);
  byte hwsw[] = { 0x1E, 0x00, 0x0C, 0xD1, 0x00, 0x07, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x60, 0x00, 0x72, 0xD5 }; // get HW and SW info
  _serialPort->write(hwsw,sizeof(hwsw));
  char incomingMessage[200];
  delay(50);
  Serial.println();
  for (int i = 0; _serialPort->available() > 0; i++) {
      char incomingByte = _serialPort->read();
      if (incomingByte == 'V') {
        i = 0;
      }
      incomingMessage[i] = incomingByte;
  }
  char versionNumber[5];
  for (int j = 0; j < sizeof(versionNumber); j++) {
    versionNumber[j] = incomingMessage[j+2];
  }
  return versionNumber;
}

String FBus::hardwareVersion() {
  prepareThing();
  delay(100);
  byte hwsw[] = { 0x1E, 0x00, 0x0C, 0xD1, 0x00, 0x07, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x60, 0x00, 0x72, 0xD5 }; // get HW and SW info
  _serialPort->write(hwsw,sizeof(hwsw));
  char incomingMessage[200];
  delay(50);
  Serial.println();
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
  prepareThing();
  delay(100);
  byte hwsw[] = { 0x1E, 0x00, 0x0C, 0xD1, 0x00, 0x07, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x60, 0x00, 0x72, 0xD5 }; // get HW and SW info
  _serialPort->write(hwsw,sizeof(hwsw));
  char incomingMessage[200];
  delay(50);
  Serial.println();
  for (int i = 0; _serialPort->available() > 0; i++) {
      char incomingByte = _serialPort->read();
      if (incomingByte == 'V') {
        i = 0;
      }
      incomingMessage[i] = incomingByte;
  }
  char hardwareVersion[8];
  for (int j = 0; j < sizeof(hardwareVersion); j++) {
    hardwareVersion[j] = incomingMessage[j+16];
  }
  return hardwareVersion;
}
