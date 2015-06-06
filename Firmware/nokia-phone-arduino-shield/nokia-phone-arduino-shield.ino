#include <SoftwareSerial.h>
#include "FBus.h"

SoftwareSerial mySerial(3, 2); // RX, TX

FBus myPhone(&Serial);

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
    myPhone.getPacket();  // Receive acknowledgement
    myPhone.getPacket();  // Receive information
    delay(20);
}

