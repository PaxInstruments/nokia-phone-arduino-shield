// Notes
// China Mobile SMS Center: +86-13800100500
//
// AltSoftSerial always uses these pins:
//
// Board          Transmit  Receive   PWM Unusable
// ----          -------  ------   ------------
// Arduino Uno        9         8         10
// Arduino Leonardo   5        13       (none)
// Arduino Mega      46        48       44, 45
// Wiring-S           5         6          4
// Sanguino          13        14         12
//
// For Uno connect Arduino pin D9 to F-Bus RX
// Connect Arduino D8 to F-Bus TX
//

//#include <AltSoftSerial.h>
#include <SoftwareSerial.h>
#include "FBus.h"

//AltSoftSerial mySerial;
SoftwareSerial mySerial(3, 2); // RX, TX

FBus myPhone(&Serial);
//FBus debug(&mySerial);

void setup() {
    
    pinMode(8, INPUT); 
    pinMode(9, INPUT);
    
    Serial.begin(115200);
    delay(200);
    
    mySerial.begin(19200);
    delay(200);
}
    
void loop() {
    mySerial.write("START");
    myPhone.initializeBus();
    delay(1);
    myPhone.serialFlush();
    myPhone.sendPacket(0xD1);  // Send a raw HWSW request. Only supported type
    myPhone.getPacket();
    myPhone.getPacket();
    delay(2000);
}

