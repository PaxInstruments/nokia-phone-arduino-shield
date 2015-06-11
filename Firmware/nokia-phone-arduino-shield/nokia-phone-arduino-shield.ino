// nokia-phone-arduino-shield.ino - Example for F-Bus library.
// For  reference see https://github.com/davidcranor/MCP2035/blob/master/MCP2035.h
// Maybe work with structs
//
//  Created by Charles Pax for Pax Instruments, 2015-05-23
//  Please visit http://paxinstruments.com/products/
//  Released into the Public Domain
//  
//  This library is designed for use with the Arduino Leonardo though
//  it may work with other boards.
//
//  Call this by passing the pointer to a serial port:
//  FBus fred(&Serial1);
//
// Notes
// - Plug into cranor's tinyTerminal
// For reference on structs see https://github.com/davidcranor/MCP2035/blob/master/MCP2035.h

#include "FBus.h"

FBus myPhone(&Serial1);

void setup() {
    Serial.begin(115200);  // Start the serial link to PC
    Serial1.begin(115200);  // Start the serial link to Phone
    
    delay(2000);
}
    
void loop() {
    myPhone.initializeBus();
    delay(1);
    myPhone.sendPacket(0xD1);  // Send a raw HWSW request. Only supported type
    myPhone.getIncomingPacket();
    myPhone.getIncomingPacket();
    delay(10);
}

