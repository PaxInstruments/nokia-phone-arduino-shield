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

#include <AltSoftSerial.h>

AltSoftSerial altSerial;

#define DEBUG            0 // Debugging code: 0 disable, 1 enable
// Creates a funciton for printing to serial during debugging.
#if DEBUG
  #define DEBUG_PRINT(x)    Serial.print (x)
  #define DEBUG_PRINTLN(x)  Serial.println (x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

// Message arrays
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  altSerial.begin(115200);
  delay(1000);
}
  
void loop() {
  DEBUG_PRINTLN("<loop()>");
//  Serial.println("");
  printHWSW();
  Serial.println("");
  DEBUG_PRINTLN("</loop()>");
  delay(4000);
}

void prepare() {
  DEBUG_PRINTLN("<prepare()>");
  for (int i = 0; i < 128; i++) {
    altSerial.write(0x55);
  }
  DEBUG_PRINTLN("");
  DEBUG_PRINTLN("</prepare()>");
}

void printHWSW() {
  DEBUG_PRINTLN("<printHWSW()>");
  prepare();
  delay(100);
  byte hwsw[] = { 0x1E, 0x00, 0x0C, 0xD1, 0x00, 0x07, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x60, 0x00, 0x72, 0xD5 }; // get HW and SW info
  byte returnMessage[500];
  altSerial.write(hwsw,sizeof(hwsw));
  delay(50);
  DEBUG_PRINT("<returnMessage>");
  for (int i = 0; altSerial.available() > 0; i++) {
      byte incomingByte = altSerial.read();
      returnMessage[i] = incomingByte;
      #if DEBUG
        Serial.print(incomingByte, HEX);DEBUG_PRINT(" ");
      #endif
  }
  DEBUG_PRINT("</returnMessage>");
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
  DEBUG_PRINTLN("");
  DEBUG_PRINTLN("</printHWSW()>");
}

