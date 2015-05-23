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

void FBus::getSoftwareVersion() {
  
}

void FBus::getHardwardVersion() {

}

void FBus::getHWSWFrame() {
  prepareThing();
  delay(100);
  byte hwsw[] = { 0x1E, 0x00, 0x0C, 0xD1, 0x00, 0x07, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x60, 0x00, 0x72, 0xD5 }; // get HW and SW info
  byte returnMessage[500];
  _serialPort->write(hwsw,sizeof(hwsw));
  delay(50);
  for (int i = 0; _serialPort->available() > 0; i++) {
      byte incomingByte = _serialPort->read();
      returnMessage[i] = incomingByte;
  }
  Serial.println();
  Serial.print("Software: ");
  for (int j = 10; j < 18; j++) {
    Serial.write(returnMessage[j]);
  }
  Serial.println();
  Serial.print("Date: ");
  for (int j = 23; j < 31; j++) {
    Serial.write(returnMessage[j]);
  }
  Serial.println();
  Serial.print("Hardware: ");
  for (int j = 32; j < 37; j++) {
    Serial.write(returnMessage[j]);
  }
  Serial.println();
  Serial.print("Copyright: ");
  for (int j = 38; j < 48; j++) {
    Serial.write(returnMessage[j]);
  }
}













