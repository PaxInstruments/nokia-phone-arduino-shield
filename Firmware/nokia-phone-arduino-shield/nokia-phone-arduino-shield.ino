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

FBus myPhone(Serial1);

void setup() {
    Serial.begin(115200);  // Start the serial link to PC
    Serial1.begin(115200);

    // wait for serial port to be opened by the PC!
    while(!Serial);

    myPhone.initialize();

    //delay(2000);

    //myPhone.SetSMSC("+16091231234", SMSC_TYPE_NATIONAL);
    //myPhone.SetPhoneNumber("+16095298807", SMSC_TYPE_NATIONAL);

    myPhone.SetSMSC("8613010888500",SMSC_TYPE_NATIONAL);
    myPhone.SetPhoneNumber("15622834051",SMSC_TYPE_UNKNOWN);
    myPhone.SendSMS("This message sent from Arduino");

    //myPhone.requestHWSW();

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

    //myPhone.requestHWSW();

    myPhone.process();


    //Serial.write("Frame: ");
    //Serial.write(sizeof(frame_header_t));
    //Serial.write(" Offset: ");
    //Serial.write(offsetof(frame_header_t,FrameID));
    //Serial.write('\n');
    Serial.println("Loop");

    delay(2000);
}

