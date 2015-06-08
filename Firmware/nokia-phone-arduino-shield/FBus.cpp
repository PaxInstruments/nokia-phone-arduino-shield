// fbus.cpp - Library for talking to an F-Bus device.
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

// Enable/disable serial debug output
#define DEBUG  1  // 0 disable, 1 enable
#if DEBUG
    #define DEBUG_PRINT(x)  Serial.print (x)
    #define DEBUG_PRINT_HEX(x)  Serial.print (x, HEX)
    #define DEBUG_PRINTLN_HEX(x)  Serial.println (x, HEX)
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
#define ACK_MSG 0x7F
// FrameLength
#define FRAME_LENGTH_MAX 0xFF  // TODO: What is the maximum frame length?
#define FRAME_LENGTH_MIN 0x00  // TODO: What is the minimum frame length?
#define FRAME_LENGTH_MSB 0x00

// Include any necessary files
#include "Arduino.h"
#include "FBus.h"

FBus::FBus(Stream *serialPort) {  // Create the FBus object
    _serialPort = serialPort;
}

FBus::FBus(Stream *serialPort, int SMSCenter) {  // Create FBus object with SMSCenter number
// TODO
// - Implement the SMS Center features. We have nothing now.
//
    _serialPort = serialPort;
    _SMSCenter = SMSCenter;
}

void FBus::initializeBus() {  // Prepare the phone to receive F-Bus messages
    serialFlush();
    for (int i = 0; i < 128; i++) {
      _serialPort->write(0x55);
    }
    _serialPort->flush();
}

void FBus::serialFlush() {  // Clear out the serial input buffer.
  while (Serial.available() > 0) {
    _serialPort->read();
  }
}

void FBus::serialInterrupt() {
 //   packet incomingPacket;
// Execute this function on serial input interrupt
}

int FBus::getIncomingPacket() {
 //   packet incomingPacket;
    for ( byte i = 0xA0; _serialPort->available(); i++) {
        _serialPort->write(incomingPacket.fieldIndex);
        processIncomingByte(&incomingPacket);
 //       _serialPort->write(incomingPacket.oddChecksum);
 //       _serialPort->write(incomingPacket.evenChecksum);
        _serialPort->write(0x88);
    }
    _serialPort->write(0x99);
    printPacket(&incomingPacket);
    return 0;
}

void FBus::printPacket(packet *_packet) {
    //_serialPort->write(_packet->fieldIndex);
    //_serialPort->write(_packet->blockIndex);
    _serialPort->write(_packet->FrameID);
    _serialPort->write(_packet->DestDEV);
    _serialPort->write(_packet->SrcDEV);
    _serialPort->write(_packet->MsgType);
    _serialPort->write(_packet->FrameLengthMSB);
    _serialPort->write(_packet->FrameLengthLSB);
    _serialPort->write(_packet->oddChecksum);
    _serialPort->write(_packet->evenChecksum);
}

void FBus::packetReset(packet *_packet) {
    _packet->fieldIndex = 0x00;
    _packet->blockIndex = 0x00;
    _packet->packetReady = 0;
    _packet->FrameID = 0x00;
    _packet->DestDEV = 0x00;
    _packet->SrcDEV = 0x00;
    _packet->MsgType = 0x00;
    _packet->FrameLengthMSB = 0x00;
    _packet->FrameLengthLSB = 0x00;
    _packet->ChkSum1 = 0x00;
    _packet->ChkSum2 = 0x00;
    _packet->oddChecksum = 0x00;
    _packet->evenChecksum = 0x00;
}

int FBus::packetOkay(packet *_packet, byte ChkSum1, byte ChkSum2) {
    if ( _packet->oddChecksum || _packet->evenChecksum ) {
        // TODO: Throw a bad checksum error
        _serialPort->write(0x96);
        packetReset(_packet);
    } else if ( _packet->MsgType == ACK_MSG ) {
        // TODO: Remove sent message from the stack
        _serialPort->write(0x69);
    } else {
        //sendAck( _packet->MsgType, _packet->SeqNo );
    }
}

void FBus::processIncomingByte(packet *_packet) {  // Add an incoming byte to the incomingPacket
// packet { FrameID, DestDEV, SrcDEV, MsgType, FrameLengthMSB, FrameLengthLSB, {block}, FramesToGo,
//      SeqNo, PaddingByte?, oddCheckSum, evenCheckSum }
//
// TODO
// - Add a watchdog timer to this. Reset fieldIndex to zero if no change after a while
// - Put this in the Arduino library
// - Make a struct that uses this
// - Trigger using a serial input interrupt. See http://stackoverflow.com/questions/10201590/arduino-serial-interrupts
//

    switch (_packet->fieldIndex) { // oddCheckSum ^= packet
        case 0x00:  // FrameID
            _packet->FrameID = _serialPort->read();
            if ( _packet->FrameID == CABLE) {
                _packet->oddChecksum ^= _packet->FrameID;
                _packet->fieldIndex++;
            } else {
                packetReset(_packet);
            }
            break;
        case 0x01:  // DestDEV
            _packet->DestDEV = _serialPort->read();
            if ( _packet->DestDEV == HOST) {
                _packet->evenChecksum ^= _packet->DestDEV;
                _packet->fieldIndex++;
            } else {
                packetReset(_packet);
            }
            break;
        case 0x02:  // SrcDEV
            _packet->SrcDEV = _serialPort->read();
            if ( _packet->SrcDEV == PHONE) {
                _packet->oddChecksum ^= _packet->SrcDEV;
                _packet->fieldIndex++;
            } else {
                packetReset(_packet);
            }
            break;
        case 0x03:  // MsgType
            // TODO: Verify MsgType is one we know
            _packet->MsgType = _serialPort->read();
            if ( 1 ) {
                _packet->evenChecksum ^= _packet->MsgType;
                _packet->fieldIndex++;
            } else {
                // TODO: Throw unknown packet error
                packetReset(_packet);
            }
            break;
        case 0x04:  // FrameLengthMSB
            _packet->FrameLengthMSB = _serialPort->read();
            if ( _packet->FrameLengthMSB == 0x00 ) {
                _packet->oddChecksum ^= _packet->FrameLengthMSB;
                _packet->fieldIndex++;
            } else {
                // TODO: throw FrameLengthMSB our of range error
                packetReset(_packet);
            }
            break;
        case 0x05:  // FrameLengthLSB
            _packet->FrameLengthLSB = _serialPort->read();
            if ( FRAME_LENGTH_MIN < _packet->FrameLengthLSB < FRAME_LENGTH_MAX ) {
                _packet->block[_packet->FrameLengthLSB] = {};  // Declare the block size
                _packet->evenChecksum ^= _packet->FrameLengthLSB;
                _packet->fieldIndex++;
            } else {
                // TODO: throw FrameLengthLSB our of range error
                packetReset(_packet);
            }
            break;
        case 0x06:  // {block}
            // Process the block
            _packet->block[_packet->blockIndex] = _serialPort->read();
            if ( _packet->blockIndex & 0x01 ) {
                _packet->evenChecksum ^= _packet->block[_packet->blockIndex];
            } else {
                _packet->oddChecksum ^= _packet->block[_packet->blockIndex];
            }
            _packet->blockIndex++;
            if (_packet->blockIndex >= _packet->FrameLengthLSB - 2 ) {
                _packet->fieldIndex++;
                if ( _packet->MsgType == ACK_MSG ) {
                    _packet->fieldIndex +=2;
                }
            }
            break;
        case 0x07:  // FramesToGo
            _packet->FramesToGo = _serialPort->read();
            if ( _packet->FrameLengthLSB & 0x01 ) {
                _packet->evenChecksum ^= _packet->block[_packet->blockIndex];
            } else {
                _packet->oddChecksum ^= _packet->block[_packet->blockIndex];
            }
            _packet->fieldIndex++;
            break;
        case 0x08:  // SeqNo. Last byte in FrameLength
            _packet->SeqNo = _serialPort->read();
            if ( _packet->FrameLengthLSB & 0x01 ) {
                _packet->oddChecksum ^= _packet->block[_packet->blockIndex];
            } else {
                _packet->evenChecksum ^= _packet->block[_packet->blockIndex];
            }
            _packet->fieldIndex++;
            break;
        case 0x09:  // Pad and checksum
            _serialPort->write(_packet->oddChecksum);
            _serialPort->write(_packet->evenChecksum);
            if ( _packet->FrameLengthLSB & 0x01 ) {
                _serialPort->read();  // Trash padding byte
            } else {
                _packet->oddChecksum ^= _serialPort->read();
            }
            _serialPort->write(_packet->oddChecksum);
            _serialPort->write(_packet->evenChecksum);
            _packet->fieldIndex++;
            break;
        case 0x0A:
            _serialPort->write(_packet->oddChecksum);
            _serialPort->write(_packet->evenChecksum);
            if ( _packet->FrameLengthLSB & 0x01 ) {
                _packet->oddChecksum ^= _serialPort->read();
            } else {
                _packet->evenChecksum ^= _serialPort->read();
                // This is the end of packet!!!
                packetReady(_packet, );
                break;
            }
            _serialPort->write(_packet->oddChecksum);
            _serialPort->write(_packet->evenChecksum);
            _packet->fieldIndex++;
            break;
        case 0x0B:
            _serialPort->write(_packet->oddChecksum);
            _serialPort->write(_packet->evenChecksum);
            if ( _packet->FrameLengthLSB & 0x01 ) {
                _packet->evenChecksum ^= _serialPort->read();
            }
            _serialPort->write(_packet->oddChecksum);
            _serialPort->write(_packet->evenChecksum);
            if ( _packet->oddChecksum || _packet->evenChecksum ) {
            // TODO: Throw a bad checksum error
                packetReset(_packet);
                break;
            } else if ( _packet->MsgType == ACK_MSG ) {
                // TODO: Remove  sent message from the stack
            } else {
               //sendAck( _packet->MsgType, _packet->SeqNo );
            }
            // Packet complete.
            // Do something.
            break;
        default: 
            break;
          // We should never get here. Throw error
  }
}

//////////////////////////////////////////////////////////////////////////////////////
//                                                                                  //
// Below here is the old functions that are not based on the packet structure. They //
// can stay here during development of this library, but should be removed once the //
// packet structure stuff is complete.                                              //
//                                                                                  //
//////////////////////////////////////////////////////////////////////////////////////

void FBus::getPacket () {  // Get the next incoming packet.
// NOTES
// - This funtion is being replaced by processIncomingByte(), which is based on 
//   the packet structure.
//
// TODO
// - Search for header match. Don't assume the next batch of data is a packet.
// - Loop until a packet is found and read completely or timeout occurs
// - Maybe there is a serial interrupt we can hook into
// - Deal with packets larger than the serial buffer plus header
// - BUG: We can only process packets less than or equal to 70 bytes (header + serial
//   input buffer size).
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

void FBus::sendPacket(byte MsgType) {// Send a packet.
// TODO
// - Generate SeqNo dynamically
// - Lookup command blocks based on MsgType
// - Rewrite this using the packet structure
// - Make function wait for acknowledgement is received or message is sent three times.
//   Throw error if ack is not received.
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

void FBus::sendAck(byte MsgType, byte SeqNo ) {  // Acknowledge packet
// TODO:
// - Rewrite this using the packet structure
//
    byte oddCheckSum = 0x00, evenCheckSum = 0x00;
    byte ack[] = { 0x1E,  // FrameID: Cable (0x1E)
                   0x00,  // DestDEV: Phone (0x00)
                   0x0C,  // SrcDEV: PC (0x0C)
                   0x7F,  // MsgType: Acknowledgement (0x7F)
                   0x00,  // FrameLengthMSB, should always be 0x00
                   0x02,  // FrameLengthLSB, should always be 0x00 for an ack packet
                   MsgType,  // The message type we are acknowledging
                   SeqNo & 0x07,  // SegNo: (<SeqNo from message> & 0x07). Sequence
                                  // ranges from 0 through 7.
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

void FBus::sendSMS(byte MsgType) {
// Send an arbitrary SMS message.
//
// TODO
// - Write this funciton. This is fully hardcoded. It doesn't even work.
//
    char HWSW_block[] = { 0x00, 0x01, 0x00, 0x03, 0x00 };
    _serialPort->write(0x1E);  // FrameID: Cable
    _serialPort->write((byte)0x00);  // DestDEV: Phone
    _serialPort->write(0x0C);  // SrcDEV: PC
    _serialPort->write(MsgType);  // MsgType, depends on phone model
    _serialPort->write((byte)0x00);  // FrameLengthMSB, should always be 0x00
    _serialPort->write(0x07);  // FrameLengthLSB, depends on message
    _serialPort->write( HWSW_block, sizeof(HWSW_block) );  // getBlock(MsgType), depends
                                                           // on MsgType and phone model
    _serialPort->write(0x01);  // FramesToGo, how many packets are left in this message
    _serialPort->write(0x60);  // SeqNo = (previous SeqNo + 1) ^ 0x07
    _serialPort->write((byte)0x00);  // padAndChecksum
    _serialPort->write(0x72);  // padAndChecksum
    _serialPort->write(0xD5);  // padAndChecksum
  
    _serialPort->flush();
}