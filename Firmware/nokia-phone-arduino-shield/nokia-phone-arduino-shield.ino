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

#include "SoftwareSerial.h"
#include "FBus.h"

SoftwareSerial mySerial(3,2);

FBus myPhone(&Serial1);

void setup() {
    Serial.begin(115200);  // Start the serial link to PC
    Serial1.begin(115200);  // Start the serial link to Phone
    mySerial.begin(19200);
    
    delay(2000);
}
    
void loop() {
    //mySerial.print("OK");
    //Serial.println("OK");
 //   Serial.println();
    myPhone.initializeBus();
    delay(1);
 //   myPhone.printTest();
    myPhone.sendPacket(0xD1);  // Send a raw HWSW request. Only supported type
  //  delay(1);
    //myPhone.packetPrint( myPhone.getIncomingPacket() );  // Should be the acknowledgement packet
    myPhone.getIncomingPacket();
    myPhone.getIncomingPacket();
//    myPhone.printPacket(myPhone.getIncomingPacket());
//    myPhone.getIncomingPacket();  // Should be the requested HWSW information
//    myPhone.getPacket();  // Receive acknowledgement
//    myPhone.getPacket();  // Receive information
    delay(2000);
}

