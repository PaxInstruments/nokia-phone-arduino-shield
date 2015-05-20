
#define DEBUG            1 // Debugging code: 0 disable, 1 enable

// Creates a funciton for printing to serial during debugging.
#if DEBUG
  #define DEBUG_PRINT(x)    Serial.print (x)
  #define DEBUG_PRINTLN(x)  Serial.println (x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

// Message arrays
byte hwsw[] = { 0x1E, 0x00, 0x0C, 0xD1, 0x00, 0x07, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x60, 0x00, 0x72, 0xD5 }; // get HW and SW info

void setup() {
  Serial.begin(115200);
  delay(1000);
}


  
void loop() {
  prepare();
  Serial.println("");
  delay(100);
  printHWSW();
  Serial.println("");
  delay(4000);
}

void prepare() {
  DEBUG_PRINTLN("<PERPARE()>");
  for (int i = 0; i < 128; i++) {
    Serial.write(0x55);
  }
  DEBUG_PRINTLN("");
  DEBUG_PRINTLN("</PERPARE()>");
}

void send(byte message[], int sizeArray) {
  DEBUG_PRINTLN("<SEND()>");
  for (int i = 0; i < sizeArray; i++) {
    Serial.write(message[i]);
  }
  DEBUG_PRINTLN("");
  DEBUG_PRINTLN("</SEND()>");
}

void printHWSW() {
  DEBUG_PRINTLN("<PRINTHWSW()>");
  send(hwsw,sizeof(hwsw));
  while (Serial.available() > 0) {
      int incomingByte = Serial.read();
      Serial.print(incomingByte, HEX);
      Serial.print(" ");
  }
  DEBUG_PRINTLN("");
  DEBUG_PRINTLN("</PRINTHWSW()>");
}

