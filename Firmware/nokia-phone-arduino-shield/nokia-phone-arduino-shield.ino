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
SoftwareSerial mySerial(8, 9); // RX, TX

FBus myPhone(&mySerial);

#define DEBUG            0 // Debugging code: 0 disable, 1 enable
// Creates a funciton for printing to serial during debugging.
#if DEBUG
    #define DEBUG_PRINT(x)    Serial.print (x)
    #define DEBUG_PRINTLN(x)  Serial.println (x)
#else
    #define DEBUG_PRINT(x)
    #define DEBUG_PRINTLN(x)
#endif

void setup() {
    Serial.begin(115200);
    delay(200);
    
    mySerial.begin(115200);
    delay(200);
}
    
void loop() {
    Serial.print("SW version: ");Serial.println(myPhone.softwareVersion());
    Serial.print("HW version: ");Serial.println(myPhone.hardwareVersion());
    Serial.print("Date code: ");Serial.println(myPhone.dateCode());
    Serial.println();
    delay(1000);
}

