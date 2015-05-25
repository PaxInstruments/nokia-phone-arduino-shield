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
        FBus(Stream *serialPort, int SMSCenter);
//        FBus(Stream *serialPort, int SMSCenter)  // Create FBus object, set SMS Center number
//        FBus::setSMSC(int SMSCenterNumber)  // Set SMS Center number
//        FBus::messageSend(int recipientNumber, String "someMessage)  // Send message to a number
//        FBus::sendFrame(char* arbitraryMessage)  // Send an arbitrary frame to phone
//        FBus::sendFrame(char* arbitraryMessage)  // Send an arbitrary frame to phone
    	String softwareVersion();
    	String hardwareVersion();
    	String dateCode();
//        char frameID
//        char destination
//        char source
    private:
        Stream* _serialPort;
        int _SMSCenter;
        void initializeBus();
        char checksumOdd();
        char checksumEven();
//        FBus::acknowledge()
};

#endif

