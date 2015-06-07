//  fbus.cpp - Library for talking to an F-Bus device.
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
//TODO: Plug into cranor's tinyTerminal

// Enable/disable serial debug output
#define DEBUG  1  // 0 disable, 1 enable
#if DEBUG
    #define DEBUG_PRINT(x)    Serial.print (x)
    #define DEBUG_PRINT_HEX(x)    Serial.print (x, HEX)
    #define DEBUG_PRINTLN(x)  Serial.println (x)
    #define DEBUG_WRITE(x)  Serial.write (x)
#else
    #define DEBUG_PRINT(x)
    #define DEBUG_PRINT_HEX(x)
    #define DEBUG_PRINTLN(x)
    #define DEBUG_WRITE(x)
#endif

// FrameID values
#define CABLE 0x1E
#define IRDA 0x1C
// DestDEV, SrcDEV values
#define PHONE 0x00
#define HOST 0x0C
// MsgType
#define REQ_HWSW 0x1D  // Request hardward and software information
// FrameLength
#define FRAME_LENGTH_MAX 0xFF  // TODO: What is the maximum frame length?
#define FRAME_LENGTH_MIN 0x00  // TODO: What is the minimum frame length?
#define FRAME_LENGTH_MSB 0x00





// Include any necessary files
#include "Arduino.h"
#include "FBus.h"

FBus::FBus(Stream *serialPort) {
// Create the FBus object. Within the library we will reference the phone
// by talking to the serial port to which it is connected. To write to the
// serial port we use the command "_serialPort->write();" and to read we use 
// the command "_serialPort->read()".
//
// TODO: none
    _serialPort = serialPort;
}

FBus::FBus(Stream *serialPort, int SMSCenter) {
// Create the FBus object and define a known SMS Center number.
//
// TODO
// - Implement the SMS Center features. We have nothing now.
//
    _serialPort = serialPort;
    _SMSCenter = SMSCenter;
}

void FBus::initializeBus() {
// Prepare the phone to receive F-Bus messages by sending a series of 128
// ASCII "U" characters. An ASCII "U" is 0x55 in hexidecimal. In binary this
// is 01010101. Basically, we send a long stream of zeros and ones.
//
// TODO: none
    DEBUG_PRINT("Init: ");
    for (int i = 0; i < 128; i++) {
      _serialPort->write(0x55);
      DEBUG_WRITE(0x55);
    }
    DEBUG_PRINTLN();
    
    serialFlush();  // Clear out the serial input buffer
    _serialPort->flush();  // Wait until serial output buffer is empty
}

void FBus::serialFlush() {
// Clear out the serial input buffer
//
// TODO:
// - Return when flush is complete. Function calls should wait while
//   serialFlush()  operates.
  while (Serial.available() > 0) {
    _serialPort->read();
  }
}

void FBus::sendPacket(byte MsgType) {
// Send a packet
//
// TODO
// - Generate SeqNo dynamically
// - Lookup command blocks based on MsgType
//
    byte header[] = { 0x1E, (byte)0x00, 0x0C, MsgType, (byte)0x00, 0x07 };
    byte body[] = { 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x60, 0x00 };
    byte oddCheckSum = 0x00;
    byte evenCheckSum = 0x00;
    byte packet[ sizeof(header) + sizeof(body) + (sizeof(body) & 0x01) + 2 ];
    for ( int i = 0; i < sizeof(header); i++) {
        packet[i] = header[i];
    }
    for ( int i = 0; i < sizeof(body); i++) {
        packet[i + sizeof(header)] = body[i];
    }
    for ( int i = 0; i < sizeof(packet); i++) {
        packet[i + sizeof(header) + sizeof(body)] = 0x00;
    }
    for ( int i = 0; i < sizeof(packet) - 2; i += 2) {
        oddCheckSum ^= packet[i];
        evenCheckSum ^= packet[i + 1];
    }
    if (packet[5] & 0x01) {
        packet[sizeof(packet) - 2 ] = oddCheckSum;
        packet[sizeof(packet) - 1 ] = evenCheckSum;
    }else {
        packet[sizeof(packet) - 3 ] = oddCheckSum;
        packet[sizeof(packet) - 2 ] = evenCheckSum;
      
    }
    _serialPort->write( packet, sizeof(packet) );
    DEBUG_PRINT("msg >>> ");
    for(int i = 0; i < sizeof(packet); i++) {
        DEBUG_PRINT_HEX(packet[i]);
        DEBUG_PRINT(" ");
    }
    DEBUG_PRINTLN();
  
    _serialPort->flush();
}

void FBus::sendAck(byte MsgType, byte SeqNo ) {
// Acknowledge a packet received from the phone by sending
// an acknowledgement packet to the phone. We only need to know
// the message type and sequence numbers of the received packet.
//
// TODO: none
    byte oddCheckSum = 0x00, evenCheckSum = 0x00;
    byte ack[] = { 0x1E,  // FrameID: Cable (0x1E)
                   0x00,  // DestDEV: Phone (0x00)
                   0x0C,  // SrcDEV: PC (0x0C)
                   0x7F,  // MsgType: Acknowledgement (0x7F)
                   0x00,  // FrameLengthMSB, should always be 0x00
                   0x02,  // FrameLengthLSB, should always be 0x00 for an ack packet
                   MsgType,  // The message type we are acknowledging
                   SeqNo & 0x07,  // SegNo: (<SeqNo from message> & 0x07). Sequence ranges from 0 through 7.
                   oddCheckSum,  //  oddCheckSum is calculated later
                   evenCheckSum   //  evenCheckSum is calculated later
                 };
    for ( int i = 0; i < sizeof(ack) - 2; i += 2) {
        oddCheckSum ^= ack[i];
        evenCheckSum ^= ack[i + 1];
    }
    ack[8] = oddCheckSum;
    ack[9] = evenCheckSum;
    _serialPort->write(ack, sizeof(ack));
    DEBUG_PRINT("ack >>> ");
    #if DEBUG
    for(int i = 0; i < sizeof(ack); i++) {
        DEBUG_PRINT_HEX(ack[i]);
        DEBUG_PRINT(" ");
    }
    #endif
    DEBUG_PRINTLN();
}

void FBus::getPacket () {
// Get the next incoming packet.
//
// TODO
// - Search for header match. Don't assume the next batch of data is a packet.
// - Loop until a packet is found and read completely or timeout occurs
// - Maybe there is a serial interrupt we can hook into
// - Deal with packets larger than the serial buffer plus header
// - BUG: We can only process packets less than or equal to  70 bytes (header + serial input buffer size).
//
    int oddCheckSum = 0, evenCheckSum = 0;
    // We should read the serial buffer until bytes 0-2 match
    byte header[6] = {};
    while ( _serialPort->available() < 6) {}
    header[0] = _serialPort->read();  // FrameID: Cable
    header[1] = _serialPort->read();  // DestDEV: PC
    header[2] = _serialPort->read();  // SrcDEV: Phone
    header[3] = _serialPort->read();  // MsgType
    header[4] = _serialPort->read();  // FrameLength MSB
    header[5] = _serialPort->read();  // FrameLength LSB
    int packetLength = sizeof(header) + header[5] + (header[5] & 0x01) + 2;
    byte packet[ packetLength ];  // Create array to hold entire packet
    for (int i = 0; i < sizeof(header); i++) {
        packet[i] = header[i];
    }
    while ( _serialPort->available() < sizeof(packet) - sizeof(header) )  {
    }
    for (byte i = sizeof(header); i < sizeof(packet); i++) {
        packet[i] = _serialPort->read();
    }
    for ( int i = 0; i <= packetLength - 4; i += 2) {
        oddCheckSum ^= packet[i];
        evenCheckSum ^= packet[i + 1];
    }
    byte MsgType = packet[3];
    byte SegNo = packet[ packetLength - 3 - (header[5] & 0x01) ];
    
    if ( packet[3] != 0x7F) {
        DEBUG_PRINT("<<< msg ");
        #if DEBUG
        for(int i = 0; i < sizeof(packet); i++) {
            DEBUG_PRINT_HEX(packet[i]);
            DEBUG_PRINT(" ");
        }
        #endif
        DEBUG_PRINTLN();
        delay(1);
        sendAck(packet[3], packet[ sizeof(packet) - 3 ] );
    }else {
        DEBUG_PRINT("<<< ack ");
        #if DEBUG
        for(int i = 0; i < sizeof(packet); i++) {
            DEBUG_PRINT_HEX(packet[i]);
            DEBUG_PRINT(" ");
        }
        #endif
        DEBUG_PRINTLN();
    }
}

void FBus::sendSMS(byte MsgType) {
// Send an arbitrary SMS message.
//
// TODO
// - Write this funciton. This is fully hardcoded.
//
    char HWSW_block[] = { 0x00, 0x01, 0x00, 0x03, 0x00 };
    _serialPort->write(0x1E);  // FrameID: Cable
    _serialPort->write((byte)0x00);  // DestDEV: Phone
    _serialPort->write(0x0C);  // SrcDEV: PC
    _serialPort->write(MsgType);  // MsgType, depends on phone model
    _serialPort->write((byte)0x00);  // FrameLengthMSB, should always be 0x00
    _serialPort->write(0x07);  // FrameLengthLSB, depends on message
    _serialPort->write( HWSW_block, sizeof(HWSW_block) );  // getBlock(MsgType), depends on MsgType and phone model
    _serialPort->write(0x01);  // FramesToGo, how many packets are left in this message
    _serialPort->write(0x60);  // SeqNo = (previous SeqNo + 1) ^ 0x07
    _serialPort->write((byte)0x00);  // padAndChecksum
    _serialPort->write(0x72);  // padAndChecksum
    _serialPort->write(0xD5);  // padAndChecksum
  
    _serialPort->flush();
}

//
//  From here down we're working on the bite-wise processing
//

void FBus::serialInterrupt() {
// Execute this function on serial input interrupt
    while ( _serialPort->available() ) {
        //processIncomingByte();
    }
}

typedef struct { 
    byte FrameID;
    byte DestDEV;
    byte SrcDEV;
    byte MsgType;
    byte FrameLengthMSB;
    byte FrameLengthLSB;
    byte* block; // Points to the command block array
    byte FramesToGo; // Calculated number of remaining frames
    byte SeqNo; // Calculated as previous SeqNo++
//    PaddingByte?, // Include this byte if FrameLength is odd
    byte oddChecksum;
    byte evenChecksum;
} packet;

byte outgoingFieldIndex = 0;
byte outgoingBlockIndex = 0;
byte outgoingByte;
packet outgoingPacket;

byte incomingFieldIndex = 0;
byte incomingBlockIndex = 0;
byte incomingByte;
packet incomingPacket;

void FBus::processIncomingBytes() {
// Process each incomingByte in serial input buffer. As we cycle throught the first
// three bytes we make sure each ones matches the expected values of a packet
// addressed to us. We trash bytes until we see the first three bytes of a packet
// header, then we assume the rest is a packet.
//
// packet { FrameID, DestDEV, SrcDEV, MsgType, FrameLengthMSB, FrameLengthLSB, {block}, FramesToGo,
//      SeqNo, PaddingByte?, oddCheckSum, evenCheckSum }
//
// TODO
// - Add a watchdog timer to this. Reset fieldIndex to zero if no change after a while
// - Put this in the Arduino library
// - Make a struct that uses this
// - Trigger using a serial input interrupt. See http://stackoverflow.com/questions/10201590/arduino-serial-interrupts
//
    switch (incomingFieldIndex) { // oddCheckSum ^= packet
        case 0:
            incomingPacket.FrameID = _serialPort->read();
            if ( incomingPacket.FrameID != CABLE) break;
            incomingPacket.oddChecksum = FRAME_LENGTH_MSB;
            incomingPacket.oddChecksum ^= incomingPacket.FrameID;
            incomingFieldIndex++;
            break;
        case 1:
          incomingPacket.DestDEV = _serialPort->read();
          if ( incomingPacket.DestDEV != HOST) {
              incomingFieldIndex = 0;
              break;
          }
          incomingPacket.evenChecksum = 0x00;
          incomingPacket.evenChecksum ^= incomingPacket.DestDEV;
          incomingFieldIndex++;
          break;
        case 2:
          incomingPacket.SrcDEV = _serialPort->read();
          if ( incomingPacket.SrcDEV != PHONE) {
              incomingFieldIndex = 0;
              break;
          }
          incomingPacket.oddChecksum ^= incomingPacket.SrcDEV;
          incomingFieldIndex++;
          break;
        case 3:
          incomingPacket.MsgType = _serialPort->read();
          incomingPacket.evenChecksum ^= incomingPacket.MsgType;
          incomingFieldIndex++;
          break;
        case 4:
          incomingPacket.FrameLengthMSB = _serialPort->read();
          if (incomingPacket.FrameLengthMSB != FRAME_LENGTH_MSB) {
                // Throw an error
                incomingFieldIndex = 0;  // Trash packet and start over
                break;
            }
          incomingPacket.oddChecksum ^= incomingPacket.FrameLengthMSB;
          incomingFieldIndex++;
          break;
        case 5:
            incomingPacket.FrameLengthLSB = _serialPort->read();
            if (incomingPacket.FrameLengthLSB > FRAME_LENGTH_MAX ) {
                // Throw an error
                incomingFieldIndex = 0;  // Trash packet and start over
                incomingBlockIndex = 0;
                break;
            } else if (incomingPacket.FrameLengthLSB < FRAME_LENGTH_MIN ) {
                // Throw an error
                incomingFieldIndex = 0;  // Trash packet and start over
                incomingBlockIndex = 0;
                break;
            }
            incomingPacket.evenChecksum ^= incomingPacket.FrameLengthLSB;
            incomingFieldIndex++;
            break;
        case 6:
            // Process the block
            incomingFieldIndex++;
            break;
        case 7:
            incomingPacket.FramesToGo = _serialPort->read();
            // Checksum odd/even is conditional based on incomingPacket.FrameLengthLSB
            // Run checksum maybe using (incomingPacket.FrameLengthLSB & 0x01)
            incomingFieldIndex++;
            break;
        case 8:
            incomingPacket.SeqNo = _serialPort->read();  // Last byte in FrameLength
            // Checksum odd/even is conditional based on incomingPacket.FrameLengthLSB
            // Run checksum maybe using (incomingPacket.FrameLengthLSB & 0x01)
            incomingFieldIndex++;
            break;
        case 9:
        // From here to the end we are working with the checksums. We do an XOR of the
        // checksums the phone gives us against the checksums we calculated. If they are
        // the same, the results will be 0x00.
            if ( incomingPacket.FrameLengthLSB & 0x01 ) {
                _serialPort->read();  // Trash padding byte
            } else {
                incomingPacket.oddChecksum ^= _serialPort->read();
            }
            incomingFieldIndex++;
            break;
//        case 10:
            if ( incomingPacket.FrameLengthLSB & 0x01 ) {
                incomingPacket.oddChecksum ^= _serialPort->read();
            } else {
                incomingPacket.evenChecksum ^= _serialPort->read();
            }
            incomingFieldIndex++;
            break;
        case 11:
            if ( incomingPacket.FrameLengthLSB & 0x01 ) {
                incomingPacket.evenChecksum ^= _serialPort->read();
            }
            if ( incomingPacket.oddChecksum || incomingPacket.evenChecksum ) {
            // Throw an error if the delivered and calculated checksums are not the same
                // Bad packet!
                // Throw checksum error
                incomingFieldIndex = 0;  // Trash packet and start over
                incomingBlockIndex = 0;
                break;
            }
            // Packet complete.
            // Do something.
            break;
        default: 
            break;
          // We should never get here. Throw error
  }
}

