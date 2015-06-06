#include <SoftwareSerial.h>
#include "FBus.h"

//SoftwareSerial mySerial(3, 2); // RX, TX

FBus myPhone(&Serial1);

void setup() {
    
    pinMode(8, INPUT); 
    pinMode(9, INPUT);
    
    Serial1.begin(115200);
    delay(200);
    
    Serial.begin(115200);
    delay(200);
}
    
void loop() {
    //Serial.println("START");
    myPhone.initializeBus();
    delay(1);
    myPhone.serialFlush();
    myPhone.sendPacket(0xD1);  // Send a raw HWSW request. Only supported type
    myPhone.getPacket();  // Receive acknowledgement
    myPhone.getPacket();  // Receive information
    delay(1000);
    Serial.println();
}

