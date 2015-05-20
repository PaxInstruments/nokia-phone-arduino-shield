
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
void setup() {
  Serial.begin(115200);
  delay(1000);
}
  
void loop() {
  DEBUG_PRINTLN("<loop()>");
  prepare();
  Serial.println("");
  delay(100);
  //printHWSW();
  testFun();
  Serial.println("");
  DEBUG_PRINTLN("</loop()>");
  delay(4000);
}

void prepare() {
  DEBUG_PRINTLN("<prepare()>");
  for (int i = 0; i < 128; i++) {
    Serial.write(0x55);
  }
  DEBUG_PRINTLN("");
  DEBUG_PRINTLN("</prepare()>");
}

void send(byte message[], int sizeArray) {
  DEBUG_PRINTLN("<send()>");
  for (int i = 0; i < sizeArray; i++) {
    Serial.write(message[i]);
  }
  DEBUG_PRINTLN("");
  DEBUG_PRINTLN("</send()>");
}

void printHWSW() {
  DEBUG_PRINTLN("<printHWSW()>");
  byte hwsw[] = { 0x1E, 0x00, 0x0C, 0xD1, 0x00, 0x07, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x60, 0x00, 0x72, 0xD5 }; // get HW and SW info
  byte returnMessage[214];
  send(hwsw,sizeof(hwsw));
  DEBUG_PRINT("<returnMessage>");
  for (int i = 0; Serial.available() > 0; i++) {
      byte incomingByte = Serial.read();
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

void testFun() {
  DEBUG_PRINTLN("<testFun()>");
  byte getVer[] = { 0x1F, 0x00, 0x04, 0x79, 0x00, 0x12, 0x02, 0x01, 0x02, 0x06, 0x00, 0x56, 0x20, 0x30, 0x36, 0x2E, 0x30, 0x30, 0x0A, 0x48, 0x46, 0x55, 0x32, 0x00 }; // get HW and SW info
  byte returnMessage[214];
  send(getVer,sizeof(getVer));
  DEBUG_PRINT("<returnMessage>");
  for (int i = 0; Serial.available() > 0; i++) {
      byte incomingByte = Serial.read();
      returnMessage[i] = incomingByte;
      #if DEBUG
        Serial.print(incomingByte, HEX);DEBUG_PRINT(" ");
      #endif
  }
  DEBUG_PRINT("</returnMessage>");
  Serial.println();
  for (int j = 0; j < sizeof(getVer); j++) {
    Serial.write(returnMessage[j]);
  }
  DEBUG_PRINTLN("");
  DEBUG_PRINTLN("</testFun()>");
}
