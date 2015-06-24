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

// FrameID values
#define CABLE 0x1E
#define IRDA 0x1C
// DestDEV, SrcDEV values
#define PHONE 0x00
#define HOST 0x0C
// MsgType
#define REQ_HWSW 0x1D  // Request hardward and software information
#define ACK_MSG 0x7F
#define SMS 0x02
// FrameLength
#define FRAME_LENGTH_MAX 0xFF  // TODO: What is the maximum frame length?
#define FRAME_LENGTH_MIN 0x00  // TODO: What is the minimum frame length?
#define FRAME_LENGTH_MSB 0x00

// Include any necessary files
#include "Arduino.h"
#include "FBus.h"

FBus::FBus(Stream *serialPort) { // Create FBus object
    _serialPort = serialPort;
}

void FBus::initializeBus() { // Prepare phone for communication
    serialFlush();
    for (int i = 0; i < 128; i++) {
      _serialPort->write(0x55);
    }
    _serialPort->flush();
}

void FBus::serialFlush() { // Empty the serial input buffer
  while (Serial.available() > 0) {
    _serialPort->read();
  }
}

void FBus::packetSend(packet *_packet) { // Send a packet
    _serialPort->write(_packet->FrameID);
    _serialPort->write(_packet->DestDEV);
    _serialPort->write(_packet->SrcDEV);
    _serialPort->write(_packet->MsgType);
    _serialPort->write(_packet->FrameLengthMSB);
    _serialPort->write(_packet->FrameLengthLSB);
    if (_packet->MsgType == ACK_MSG ) {
        for ( byte i = 0x00; i < 2; i++ ) {
            _serialPort->write(_packet->block[i]);
        }
    } else {
        for ( byte i = 0x00; i < _packet->FrameLengthLSB - 2; i++ ) {
            _serialPort->write(_packet->block[i]);
        }
        _serialPort->write(_packet->FramesToGo);
        _serialPort->write(_packet->SeqNo);
        if (_packet->FrameLengthLSB & 0x01) {
            _serialPort->write((byte)0x00);
        }
    }
    checksum(_packet);
    _serialPort->write(_packet->oddChecksum);
    _serialPort->write(_packet->evenChecksum);
    _serialPort->flush();
    if (_packet->MsgType != ACK_MSG ) {
        getACK();
    }
}

void FBus::packetReset(packet *_packet) { // Reset al packet fields to 0x00
    _packet->fieldIndex = 0x00;
    _packet->blockIndex = 0x00;
    _packet->packetReady = 0;
    _packet->FrameID = 0x00;
    _packet->DestDEV = 0x00;
    _packet->SrcDEV = 0x00;
    _packet->MsgType = 0x00;
    _packet->FrameLengthMSB = 0x00;
    _packet->FrameLengthLSB = 0x00;
    for ( byte i = 0x00; i < sizeof(_packet->block); i++ ) {
        _packet->block[i] = 0x00;
    }
    _packet->FramesToGo = 0x00;
    _packet->SeqNo = 0x00;
    _packet->oddChecksum = 0x00;
    _packet->evenChecksum = 0x00;
}

int FBus::checksum(packet *_packet) { // Verify or add a checksum
    byte oddChecksum, evenChecksum = 0x00;
    oddChecksum ^= _packet->FrameID;
    evenChecksum ^= _packet->DestDEV;
    oddChecksum ^= _packet->SrcDEV;
    evenChecksum ^= _packet->MsgType;
    oddChecksum ^= _packet->FrameLengthMSB;
    evenChecksum ^= _packet->FrameLengthLSB;
    if ( _packet->MsgType == ACK_MSG ) {
        for ( byte i = 0x00; i < _packet->FrameLengthLSB; i += 2 ) {
            oddChecksum ^= _packet->block[i];
        }
        for ( byte i = 0x01; i < _packet->FrameLengthLSB; i += 2 ) {
            evenChecksum ^= _packet->block[i];
        }
    } else {
        for ( byte i = 0x00; i < _packet->FrameLengthLSB - 2; i += 2 ) {
            oddChecksum ^= _packet->block[i];
        }
        for ( byte i = 0x01; i < _packet->FrameLengthLSB - 2; i += 2 ) {
            evenChecksum ^= _packet->block[i];
        }
        if ( _packet->FrameLengthLSB & 0x01 ) {
            evenChecksum ^= _packet->FramesToGo;
            oddChecksum ^= _packet->SeqNo;
        } else {
            oddChecksum ^= _packet->FramesToGo;
            evenChecksum ^= _packet->SeqNo;
        }
    }
    if ( oddChecksum == _packet->oddChecksum && evenChecksum == _packet->evenChecksum ) {
        return 1;
    } else {
        _packet->oddChecksum = oddChecksum;
        _packet->evenChecksum = evenChecksum;
        return 0;
    }
}

packet* FBus::getIncomingPacket() { // Retreive the incoming packet
    packetReset(&incomingPacket);
    while ( !incomingPacket.packetReady ) {  // TODO: Added a timeout function here
        // DEBUG NTOES: 
        // We get to here just fine
        //_serialPort->write(0xD0);
        // Repeats 0xD0 forever
        if ( _serialPort->available() ) {
            // DEBUG NOTES:
            // We get here just fine
            // _serialPort->write(0xD1);
            // Appears to write 0xD1 while processing bytes
            processIncomingByte(&incomingPacket);
        }
    }
    if ( incomingPacket.MsgType != ACK_MSG ) {
        sendAck(incomingPacket.MsgType, incomingPacket.SeqNo );
        _serialPort->flush();
    }
    return &incomingPacket;
}

void FBus::processIncomingByte(packet *_packet) { // Byte-wise process the data stream
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
                _packet->fieldIndex++;
            } else {
                packetReset(_packet);
            }
            break;
        case 0x01:  // DestDEV
            _packet->DestDEV = _serialPort->read();
            if ( _packet->DestDEV == HOST) {
                _packet->fieldIndex++;
            } else {
                packetReset(_packet);
            }
            break;
        case 0x02:  // SrcDEV
            _packet->SrcDEV = _serialPort->read();
            if ( _packet->SrcDEV == PHONE) {
                _packet->fieldIndex++;
            } else {
                packetReset(_packet);
            }
            break;
        case 0x03:  // MsgType
            // TODO: Verify MsgType is one we know
            _packet->MsgType = _serialPort->read();
            if ( 1 ) {
                _packet->fieldIndex++;
            } else {
                // TODO: Throw unknown packet error
                packetReset(_packet);
            }
            break;
        case 0x04:  // FrameLengthMSB
            _packet->FrameLengthMSB = _serialPort->read();
            if ( _packet->FrameLengthMSB == 0x00 ) {
                _packet->fieldIndex++;
            } else {
                // TODO: throw FrameLengthMSB our of range error
                packetReset(_packet);
            }
            break;
        case 0x05:  // FrameLengthLSB
            _packet->FrameLengthLSB = _serialPort->read();
            if ( FRAME_LENGTH_MIN < _packet->FrameLengthLSB < FRAME_LENGTH_MAX ) {
                _packet->fieldIndex++;
            } else {
                // TODO: throw FrameLengthLSB our of range error
                packetReset(_packet);
            }
            break;
        case 0x06:  // {block}
            _packet->block[_packet->blockIndex] = _serialPort->read();
            _packet->blockIndex++;
            if ( _packet->MsgType == ACK_MSG ) {
                if ( _packet->blockIndex >= 2 ) {
                    _packet->fieldIndex += 3;
                }
            } else if (_packet->blockIndex >= _packet->FrameLengthLSB - 2 ) {
                    _packet->fieldIndex++;
            }
            break;
        case 0x07:  // FramesToGo
            _packet->FramesToGo = _serialPort->read();
            _packet->fieldIndex++;
            break;
        case 0x08:
            _packet->SeqNo = _serialPort->read();
            _packet->fieldIndex++;
            break;
        case 0x09:  // 0x00 or oddChecksum
            if (_packet->FrameLengthLSB & 0x01) {
                _serialPort->read();
            } else {
                _packet->oddChecksum = _serialPort->read();
            }
            _packet->fieldIndex++;
            break;
        case 0x0A:
            if (_packet->FrameLengthLSB & 0x01) {
                _packet->oddChecksum = _serialPort->read();
                _packet->fieldIndex++;
            } else {  // Packet complete
                _packet->evenChecksum = _serialPort->read();
                _packet->packetReady = checksum(_packet);
            }
            break;
        case 0x0B:  // Packet complete
            _packet->evenChecksum = _serialPort->read();
            _packet->packetReady = checksum(_packet);
            break;
        default: 
            break;
            // We should never get here. Throw error
  }
}

void FBus::getACK() {
    // TODO
    // - Implement a stack system
    // - Remove acknowledged packets form the outgoing stack
    getIncomingPacket();
}

packet* FBus::requestHWSW() { // Send HWSW request packet
    packetReset( &outgoingPacket );
    // Request HWSW information packet
    outgoingPacket.fieldIndex = 0x00;
    outgoingPacket.blockIndex = 0x00;
    outgoingPacket.packetReady = 0;
    outgoingPacket.FrameID = 0x1E;
    outgoingPacket.DestDEV = 0x00;
    outgoingPacket.SrcDEV = 0x0C;
    outgoingPacket.MsgType = 0xD1;
    outgoingPacket.FrameLengthMSB = 0x00;
    outgoingPacket.FrameLengthLSB = 0x07;
    byte block[] = { 0x00, 0x01, 0x00, 0x03, 0x00 };
    for (int i=0; i<sizeof(block); i++) {
        outgoingPacket.block[i] = block[i];
    }
    outgoingPacket.FramesToGo = 0x01; // Calculated number of remaining frames
    outgoingPacket.SeqNo = 0x60; // Calculated as previous SeqNo++
//    PaddingByte?, // Include this byte if FrameLength is odd
    packetSend(&outgoingPacket);
    packet* _packet = getIncomingPacket();
    return _packet;
}

String FBus::versionSW() { // Return
    // TODO
    // - Seek through block to find software version string
    String sw_version = "";
    packet* _packet = requestHWSW();
    for (int i = 4; i < 11; i++) {
        char j = (byte)_packet->block[i];
        sw_version = String(sw_version + j);
    }
    
    return sw_version;
}

String FBus::versionDate() { // Return hardware version
    // TODO
    // - Seek through block to find date string
    String version_date = "";
    packet* _packet = requestHWSW();
    for (int i = 17; i < 25; i++) {
        char j = (byte)_packet->block[i];
        version_date = String(version_date + j );
    }
    
    return version_date;
}

String FBus::versionHW() { // Return hardware version
    // TODO
    // - Seek through block to find hardware version string
    String hw_version = "";
    packet* _packet = requestHWSW();
    for (int i = 26; i < 31; i++) {
        char j = (byte)_packet->block[i];
        hw_version = String(hw_version + j );
    }
    
    return hw_version;
}

void FBus::sendAck(byte MsgType, byte SeqNo ) {  // Acknowledge packet
    packetReset( &outgoingPacket );
    outgoingPacket.FrameID = 0x1E;
    outgoingPacket.DestDEV = 0x00;
    outgoingPacket.SrcDEV = 0x0C;
    outgoingPacket.MsgType = 0x7F;
    outgoingPacket.FrameLengthMSB = 0x00;
    outgoingPacket.FrameLengthLSB = 0x02;
    byte block[] = { MsgType, SeqNo & 0x07 };
    for (int i=0; i<sizeof(block); i++) {
        outgoingPacket.block[i] = block[i];
    }
    checksum(&outgoingPacket);
    packetSend( &outgoingPacket );    byte oddCheckSum = 0x00, evenCheckSum = 0x00;
}

void FBus::packBytes() {
    //
    //
    // TODO
    // - Input an array pointer to input[]
    // - Output an array pointer to decoded[]

    char input [] = { 0b01101000, 0b01100101, 0b01101100, 0b01101100, 0b01101111 };
    char msg[128];
    char decode[128] = {};
    int len;

    unsigned char c = 0;
    unsigned char w  = 0;
    int n = 0;
    int shift = 0;
    int x = 0;

    for ( n=0 ; n<5 ; ++n ) {
        c = input[n] & 0b01111111;
        c >>= shift;
        w = input[n+1] & 0b01111111;
        w <<= (7-shift);
        shift +=1;
        c = c | w;
        if (shift == 7) {
            shift = 0x00;
            n++;
        }
        x = strlen(decode);
        decode[x] = c;
        decode[x+1] = 0;
    }
}

byte FBus::reverseAndHex(int input) { // Reverse digits of an interger and output hex
    // Example: input = 73, output = 0x37
    // Example: input = 5, output = 0x50
    int reverse = input%10 * 10 + input/10;
    byte reverseHex = 0; // Fix this
    return reverseHex;
}

void FBus::setSMSC(int SMSC_number) {
    char a[] = { 0x12, 0x34, 0x56, 0x78, 0x90};
    for ( int i; i < sizeof(a); i++ ){
       // a[i] = (([i] & 0b1111) << 4) | (a[i] >> 4);
    }
    Serial.write(a, sizeof(a));
}

/*
    byte message[] = { 'h', 'e', 'l', 'l', 'o', '\0' };
    char messagePacked[sizeof(message)];

    unsigned char holder;
    unsigned char bucket;
    char decode[128];
    int x;
    int shifted = 0;
    for ( int i = 0; i < sizeof(message); i++ ) {
        holder = message[i] & 0x7f;
        holder >>= shifted;
        bucket = message[i+1] & 0x7f;
        bucket <<= (7-shifted);
        shifted += 1;
        holder = holder | bucket;
        if (shifted == 7) {
            shifted = 0x00;
            i++;
        }
        x = strlen(decode);
        decode[x] = holder;
        decode[x+1] = 0;
    }


    for ( int i = 0; i < strlen(decode); i++ ) {
        Serial.print((unsigned char)decode[i]);
    }
*/
