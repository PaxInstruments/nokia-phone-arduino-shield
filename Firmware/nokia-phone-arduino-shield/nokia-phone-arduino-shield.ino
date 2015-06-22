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
    
    //delay(2000);

    myPhone.initialize(115200);
}
    
void loop() {
    //myPhone.initializeBus();
    //delay(1);
    //Serial.print("HW: ");Serial.println(myPhone.versionHW());
    //Serial.print("SW: ");Serial.println(myPhone.versionSW());
    //Serial.print("Date: ");Serial.println(myPhone.versionDate());
//    const char message[] = { 'h', 'e', 'l', 'l', 'o' };
//    unsigned char phoneNumber[] = { '4', '5', '6', '7', '8', '9' };
   // myPhone.SendSMS(message, phoneNumber );
//    myPhone.packBytes();
    Serial.println("Test");
    delay(2000);
}

