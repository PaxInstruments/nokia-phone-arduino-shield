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
    byte* block; // Points to the command block array
    byte FramesToGo = 0x00; // Calculated number of remaining frames
    byte SeqNo = 0x00; // Calculated as previous SeqNo++
//    PaddingByte?, // Include this byte if FrameLength is odd
    byte ChkSum1 = 0x00;
    byte ChkSum2 = 0x00;
    byte oddChecksum = 0x00;
    byte evenChecksum = 0x00;
} packet;

class FBus {
    public:
        FBus(Stream *serialPort);  // Create FBus object
        FBus(Stream *serialPort, int SMSCenter);  // Create FBus object, set SMS Center number
        void initializeBus();  // Perpares phone to receice F-Bus messages
        void sendPacket(byte MsgType);  // Send a packet with the specified messag
        void getPacket();
        void sendSMS(byte MsgType);
        void serialInterrupt();
        void processIncomingByte(packet *incomingPacket);
        int getIncomingPacket();
        void printPacket(packet *_packet);
        void sendAck(byte MsgType, byte SeqNo );
        int packetOkay(packet *_packet, byte oddChecksum, byte evenChecksum);
        void checksum(packet *_packet, byte inputByte);
//        FBus::setSMSC(int SMSCenterNumber)  // Set SMS Center number
//        FBus::messageSend(int recipientNumber, String "someMessage)  // Send message to a number
//        FBus::sendFrame(char* arbitraryMessage)  // Send an arbitrary frame to phone
//        FBus::sendFrame(char* arbitraryMessage)  // Send an arbitrary frame to phone
//    	String softwareVersion();
//    	String hardwareVersion();
//    	String dateCode();
//        char frameID
//        char destination
//        char source
    private:
        Stream* _serialPort;
        int _SMSCenter;
        packet incomingPacket;
        void serialFlush();  // Empty the serial input buffer
        void packetReset(packet *_packet);
//        char checksumOdd();
//        char checksumEven();
//        FBus::acknowledge()
};

#endif

