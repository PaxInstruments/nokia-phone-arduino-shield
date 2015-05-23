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
    	void getSoftwareVersion();
    	void getHardwardVersion();
    	void getCopyright();
        void prepareThing();
        void getHWSWFrame();
    private:
        Stream* _serialPort;
};

#endif
