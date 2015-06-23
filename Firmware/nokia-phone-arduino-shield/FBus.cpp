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

#define UINT16_SWAP(V)      do{(V) = ((V)>>8)|((V)<<8);}while(0)

// FrameID values
#define FBUS_VIA_CABLE      0x1E
#define FBUS_VIA_IRDA       0x1C

// DestDEV, SrcDEV values
#define FBUS_DEV_PHONE      0x00
#define FBUS_DEV_HOST       0x0C

// MsgType
#define REQ_HWSW            0xD1  // Request hardward and software information
#define ACK_MSG             0x7F
#define SMS                 0x02

// FrameLength
#define FRAME_LENGTH_MAX    0xFF  // TODO: What is the maximum frame length?
#define FRAME_LENGTH_MIN    0x00  // TODO: What is the minimum frame length?
#define FRAME_LENGTH_MSB    0x00

// Include any necessary files
#include "Arduino.h"
#include "HardwareSerial.h"
#include "FBus.h"
#include "string.h"

FBus::FBus(HardwareSerial & serialPort)
: _serialPort(serialPort)
{
    memset(m_block,0,sizeof(m_block));

    m_frame_ptr = (frame_header_t*)&m_block;
}

void FBus::process()
{
    char c;
    while(_serialPort.available())
    {
        c=_serialPort.read();
        if(c==-1) break;
        this->processIncomingByte(c);
    }
    return;
}

// Prepare phone for communication
void FBus::initialize()
{
    // Clear the RX
    serialFlush();

    // Send 0x55 to initialize the phone, based on
    // captures 55 should be enough
    // http://www.codeproject.com/Articles/13452/A-Simple-Guide-To-Mobile-Phone-File-Transferring#Nokia_FBUS_File_Transferring
    for (int i = 0; i < 55; i++) {
        _serialPort.write(0x55);
    }
    _serialPort.flush();

    // Phone should be in FBus mode now

    return;
}

packet_t* FBus::requestHWSW()
{
    // Send HWSW request packet
    //packetReset( &outgoingPacket );
    // Request HWSW information packet
    outgoingPacket.FrameID = FBUS_VIA_CABLE;
    outgoingPacket.DestDEV = FBUS_DEV_PHONE;
    outgoingPacket.SrcDEV = FBUS_DEV_HOST;
    outgoingPacket.MsgType = REQ_HWSW;
    uint8_t block[] = { 0x00, 0x03, 0x00 };
    outgoingPacket.FrameLength = sizeof(block);
    //if(outgoingPacket.FrameLength&1) outgoingPacket.FrameLength++;
    for (int i=0; i<sizeof(block); i++) {
        outgoingPacket.data[i] = block[i];
    }
    outgoingPacket.FramesToGo = 0x01; // Calculated number of remaining frames
    outgoingPacket.SeqNo = 0x60; // Calculated as previous SeqNo++
    packetSend(&outgoingPacket);
    //packet_t* _packet = getIncomingPacket();
    //return _packet;
    return NULL;
}



// Private functions
// ------------------------------------------------------

// Empty the serial input buffer
void FBus::serialFlush()
{
    while(_serialPort.available() > 0) {
        _serialPort.read();
    }
    return;
}

// Send a packet
void FBus::packetSend(frame_header_t * packet_ptr)
{
    #if 0
    _serialPort->write(_packet->FrameID);
    _serialPort->write(_packet->DestDEV);
    _serialPort->write(_packet->SrcDEV);
    _serialPort->write(_packet->MsgType);
    _serialPort->write(_packet->FrameLengthMSB);
    _serialPort->write(_packet->FrameLengthLSB);
    if (_packet->MsgType == ACK_MSG )
    {
        for ( byte i = 0x00; i < 2; i++ )
        {
            _serialPort->write(_packet->block[i]);
        }
    } else {
        for ( byte i = 0x00; i < _packet->FrameLengthLSB - 2; i++ )
        {
            _serialPort->write(_packet->block[i]);
        }
        _serialPort->write(_packet->FramesToGo);
        _serialPort->write(_packet->SeqNo);
        if (_packet->FrameLengthLSB & 0x01)
        {
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
    #else
    int count;
    uint8_t * data_ptr;
    uint8_t checksum_odd=0,checksum_even=0;
    data_ptr = (uint8_t*)&(packet_ptr->FrameID);
    count = (packet_ptr->FrameLength+(sizeof(frame_header_t)-offsetof(frame_header_t,FrameID)));
    if(count&1) packet_ptr->FrameLength++;
    // Add 2 to FrameLength for the padding16 bytes
    // Add 1 to FrameLength for the FramesToGo value
    packet_ptr->FrameLength+=3;
    packet_ptr->padding16 = 1;
    UINT16_SWAP(packet_ptr->FrameLength);
    UINT16_SWAP(packet_ptr->padding16);
    for(int x=0;x<count;x++)
    {
        _serialPort.write(data_ptr[x]);
        if(x&1)
            checksum_odd ^= data_ptr[x];
        else
            checksum_even ^= data_ptr[x];
    }

    // Now send the last 2 data bytes
    _serialPort.write(packet_ptr->FramesToGo);
    _serialPort.write(packet_ptr->SeqNo);

    if(count&1)
    {
        checksum_odd ^= packet_ptr->FramesToGo;
        checksum_even ^= packet_ptr->SeqNo;
    }else{
        checksum_even ^= packet_ptr->FramesToGo;
        checksum_odd ^= packet_ptr->SeqNo;
    }

    // Make sure we send an even number of bytes
    if(count&1) _serialPort.write(0);

    // Now send the checksums
    _serialPort.write(checksum_even);
    _serialPort.write(checksum_odd);

    #endif

    return;
}

void FBus::packetReset(frame_header_t *packet_ptr)
{
    memset(packet_ptr,0,sizeof(frame_header_t));
}
#if 0
// Verify or add a checksum
int FBus::checksum(packet_t *_packet)
{
    byte oddChecksum, evenChecksum = 0x00;
    oddChecksum ^= _packet->FrameID;
    evenChecksum ^= _packet->DestDEV;
    oddChecksum ^= _packet->SrcDEV;
    evenChecksum ^= _packet->MsgType;
    oddChecksum ^= _packet->FrameLengthMSB;
    evenChecksum ^= _packet->FrameLengthLSB;
    if ( _packet->MsgType == ACK_MSG )
    {
        for ( byte i = 0x00; i < _packet->FrameLengthLSB; i += 2 )
        {
            oddChecksum ^= _packet->block[i];
        }
        for ( byte i = 0x01; i < _packet->FrameLengthLSB; i += 2 )
        {
            evenChecksum ^= _packet->block[i];
        }
    } else {
        for ( byte i = 0x00; i < _packet->FrameLengthLSB - 2; i += 2 )
        {
            oddChecksum ^= _packet->block[i];
        }
        for ( byte i = 0x01; i < _packet->FrameLengthLSB - 2; i += 2 )
        {
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
    if ( oddChecksum == _packet->oddChecksum && evenChecksum == _packet->evenChecksum )
    {
        return 1;
    } else {
        _packet->oddChecksum = oddChecksum;
        _packet->evenChecksum = evenChecksum;
        return 0;
    }
}
#endif

// Byte-wise process the data stream
void FBus::processIncomingByte(uint8_t inbyte)
{
    static uint8_t input_state = 0;
// TODO
// - Add a watchdog timer to this. Reset fieldIndex to zero if no change after a while
// - Put this in the Arduino library
// - Make a struct that uses this
// - Trigger using a serial input interrupt. See http://stackoverflow.com/questions/10201590/arduino-serial-interrupts
//
#if 0
    switch (input_state)
    {
        case 0x00:  // FrameID
            _packet->FrameID = inbyte;
            if ( _packet->FrameID == FBUS_VIA_CABLE) {
                input_state++;
            } else {
                //packetReset(_packet);
            }
            break;
        case 0x01:  // DestDEV
            _packet->DestDEV = inbyte;
            if ( _packet->DestDEV == FBUS_DEV_HOST) {
                input_state++;
            } else {
                //packetReset(_packet);
            }
            break;
        case 0x02:  // SrcDEV
            _packet->SrcDEV = inbyte;
            if ( _packet->SrcDEV == FBUS_DEV_PHONE) {
                input_state++;
            } else {
                //packetReset(_packet);
            }
            break;
        case 0x03:  // MsgType
            // TODO: Verify MsgType is one we know
            _packet->MsgType = inbyte;
            if ( 1 ) {
                input_state++;
            } else {
                // TODO: Throw unknown packet error
                //packetReset(_packet);
            }
            break;
        case 0x04:  // FrameLengthMSB
            _packet->FrameLengthMSB = inbyte;
            if ( _packet->FrameLengthMSB == 0x00 ) {
                input_state++;
            } else {
                // TODO: throw FrameLengthMSB our of range error
                //packetReset(_packet);
            }
            break;
        case 0x05:  // FrameLengthLSB
            _packet->FrameLengthLSB = inbyte;
            if ( FRAME_LENGTH_MIN < _packet->FrameLengthLSB < FRAME_LENGTH_MAX ) {
                input_state++;
            } else {
                // TODO: throw FrameLengthLSB our of range error
                //packetReset(_packet);
            }
            break;
        case 0x06:  // {block}
            _packet->block[_packet->blockIndex] = inbyte;
            _packet->blockIndex++;
            if ( _packet->MsgType == ACK_MSG ) {
                if ( _packet->blockIndex >= 2 ) {
                    input_state += 3;
                }
            } else if (_packet->blockIndex >= _packet->FrameLengthLSB - 2 ) {
                input_state++;
            }
            break;
        case 0x07:  // FramesToGo
            _packet->FramesToGo = inbyte;
            input_state++;
            break;
        case 0x08:
            _packet->SeqNo = inbyte;
            input_state++;
            break;
        case 0x09:  // 0x00 or oddChecksum
            if (_packet->FrameLengthLSB & 0x01) {
                //_serialPort->read();
            } else {
                _packet->oddChecksum = inbyte;
            }
            input_state++;
            break;
        case 0x0A:
            if (_packet->FrameLengthLSB & 0x01) {
                _packet->oddChecksum = inbyte;
                input_state++;
            } else {  // Packet complete
                _packet->evenChecksum = inbyte;
                //_packet->packetReady = checksum(_packet);
            }
            break;
        case 0x0B:  // Packet complete
            _packet->evenChecksum = inbyte;
            //_packet->packetReady = checksum(_packet);
            break;
        default: 
            break;
            // We should never get here. Throw error?
    }
#endif

    return;
}


// Retreive the incoming packet
packet_t* FBus::getIncomingPacket()
{
#if 0
    //packetReset(&incomingPacket);
    while ( !incomingPacket.packetReady )
    {  // TODO: Added a timeout function here
        // DEBUG NTOES:
        // We get to here just fine
        //_serialPort->write(0xD0);
        // Repeats 0xD0 forever
        if ( _serialPort->available() )
        {
            // DEBUG NOTES:
            // We get here just fine
            // _serialPort->write(0xD1);
            // Appears to write 0xD1 while processing bytes
            //processIncomingByte(&incomingPacket);
        }
    }
    if ( incomingPacket.MsgType != ACK_MSG )
    {
        sendAck(incomingPacket.MsgType, incomingPacket.SeqNo );
        _serialPort->flush();
    }
    return &incomingPacket;
#endif
    return NULL;
}


void FBus::getACK()
{
    // TODO
    // - Implement a stack system
    // - Remove acknowledged packets form the outgoing stack
    getIncomingPacket();
}



String FBus::versionSW()
{
#if 0
    // Return
    // TODO
    // - Seek through block to find software version string
    String sw_version = "";
    packet_t* _packet = requestHWSW();
    for (int i = 4; i < 11; i++) {
        char j = (byte)_packet->block[i];
        sw_version = String(sw_version + j);
    }

    return sw_version;
#endif
    return "0";
}

String FBus::versionDate()
{
    // Return hardware version
    // TODO
    // - Seek through block to find date string
#if 0
    String version_date = "";
    packet_t* _packet = requestHWSW();
    for (int i = 17; i < 25; i++) {
        char j = (byte)_packet->block[i];
        version_date = String(version_date + j );
    }
    
    return version_date;
#endif
    return "0";
}

String FBus::versionHW()
{
#if 0
    // Return hardware version
    // TODO
    // - Seek through block to find hardware version string
    String hw_version = "";
    packet_t* _packet = requestHWSW();
    for (int i = 26; i < 31; i++) {
        char j = (byte)_packet->block[i];
        hw_version = String(hw_version + j );
    }
    
    return hw_version;
#endif
    return "0";
}

void FBus::sendAck(byte MsgType, byte SeqNo )
{
#if 0
    // Acknowledge packet
    //packetReset( &outgoingPacket );
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
    //checksum(&outgoingPacket);
    //packetSend( &outgoingPacket );    byte oddCheckSum = 0x00, evenCheckSum = 0x00;
#endif
}

void FBus::packBytes()
{
#if 0
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
#endif
}

byte FBus::reverseAndHex(int input)
{
#if 0
    // Reverse digits of an interger and output hex
    // Example: input = 73, output = 0x37
    // Example: input = 5, output = 0x50
    int reverse = input%10 * 10 + input/10;
    byte reverseHex = 0; // Fix this
    return reverseHex;
#endif
    return 0;
}

void FBus::setSMSC(int SMSC_number)
{
#if 0
    char a[] = { 0x12, 0x34, 0x56, 0x78, 0x90};
    for ( int i; i < sizeof(a); i++ ){
        a[i] = ((a[i] & 0xF) << 4) | (a[i] >> 4);
    }
    Serial.write(a, sizeof(a));
#endif
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
