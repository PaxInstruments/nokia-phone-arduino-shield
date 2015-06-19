/*
  fbus.h - Library for talking to an F-Bus device.
  Created by Charles Pax for Pax Instruments, 2015-05-23
  Please visit http://paxinstruments.com/products/
  Released into the Public Domain
*/

#ifndef FBus_h
#define FBus_h

#include "Arduino.h"

typedef struct { 
    byte fieldIndex = 0x00;
    byte blockIndex = 0x00;
    bool packetReady = 0;
    byte FrameID = 0x00;
    byte DestDEV = 0x00;
    byte SrcDEV = 0x00;
    byte MsgType = 0x00;
    byte FrameLengthMSB = 0x00;
    byte FrameLengthLSB = 0x00;
    byte block[64] = {};
    byte FramesToGo = 0x00; // Calculated number of remaining frames
    byte SeqNo = 0x00; // Calculated as previous SeqNo++
//    PaddingByte?, // Include this byte if FrameLength is odd
    byte oddChecksum = 0x00;
    byte evenChecksum = 0x00;
} packet;

class FBus {
    public:
        FBus(Stream *serialPort); // Create FBus object
        void initializeBus(); // Prepare phone for communication
        packet* getIncomingPacket(); // Retreive the incoming packet
        packet* requestHWSW(); // Send HWSW request packet
        void packetSend(packet *_packet); // Send a packet
        void sendAck(byte MsgType, byte SeqNo ); // Aknowledge received packet
        void getACK();
        String versionHW(); // Return hardware version
        String versionSW(); // Return software version
        String versionDate(); // Return software version
        void packBytes();
        byte reverseAndHex(int input);
        void setSMSC(int SMSC_number);

        unsigned char SendSMS(const char *Message, unsigned char *PhoneNumber);
    private:
        void processIncomingByte(packet *incomingPacket); // Byte-wise process the data stream
        Stream* _serialPort; // Serial port attached to phone
        int checksum(packet *_packet); // Verify or add a checksum
        packet incomingPacket; // Incoming packet buffer
        packet outgoingPacket; // Outgoing packet buffer
        void serialFlush(); // Empty the serial input buffer
        void packetReset(packet *_packet); // Reset al packet fields to 0x00
};

#endif

