#include <SoftwareSerial.h>
#include "FBus.h"

FBus myPhone(&Serial1);

void setup() {
    Serial.begin(115200);  // Start the serial link to PC
    Serial1.begin(115200);  // Start the serial link to Phone
}
    
void loop() {
    myPhone.initializeBus();
    delay(1);
    myPhone.serialFlush();
    myPhone.sendPacket(0xD1);  // Send a raw HWSW request. Only supported type
    myPhone.getPacket();  // Receive acknowledgement
    myPhone.getPacket();  // Receive information
    delay(1000);
    Serial.println();
}

