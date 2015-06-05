/*
  fbus.h - Library for talking to an F-Bus device.
  Created by Charles Pax for Pax Instruments, 2015-05-23
  Please visit http://paxinstruments.com/products/
  Released into the Public Domain
*/

#ifndef FBus_h
#define FBus_h

#include "Arduino.h"

class FBus {
    public:
        FBus(Stream *serialPort);
            void initializeBus();  // Perpares phone to receice F-Bus messages
            void sendTest();  // Send a raw HWSW request.
            void sendPacket(byte MsgType);  // Send a packet with the specified messag
            void getPacket();
            void serialFlush();
            void sendAck(byte MsgType, byte SeqNo );
//        void listener();
//        void pulse();  // Used for debugging
//        void serialFlush();
//        FBus(Stream *serialPort, int SMSCenter);
//        FBus(Stream *serialPort, int SMSCenter)  // Create FBus object, set SMS Center number
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
//        int _SMSCenter;
//        char checksumOdd();
//        char checksumEven();
//        FBus::acknowledge()
};

#endif

