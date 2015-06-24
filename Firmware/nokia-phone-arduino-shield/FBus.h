/*
  fbus.h - Library for talking to an F-Bus device.
  Created by Charles Pax for Pax Instruments, 2015-05-23
  Please visit http://paxinstruments.com/products/
  Released into the Public Domain
*/

#ifndef FBus_h
#define FBus_h

#include "Arduino.h"
#include "stdint.h"

#define FBUS_VERSION_1      1
#define FBUS_VERSION_2      2

typedef enum{
    SMSC_TYPE_UNKNOWN = 0x81,
    SMSC_TYPE_INTERNATIONAL = 0x91,
    SMSC_TYPE_NATIONAL = 0xA1,
}fbus_smsc_e;

// The ordering of this struct is important, this matches
// the FBus frame starting with FrameID, this way we can
// just send the array without extra processing.
typedef struct {
    uint8_t FramesToGo;
    uint8_t SeqNo;
    // Start transmit here
    uint8_t FrameID;
    uint8_t DestDEV;
    uint8_t SrcDEV;
    uint8_t MsgType;
    uint16_t FrameLength;
    uint16_t padding16;
    uint8_t data[128];
}frame_header_t;

typedef struct { 
    uint8_t fieldIndex = 0x00;
    uint8_t blockIndex = 0x00;
    uint8_t packetReady = 0;
    uint8_t FrameID = 0x00;
    uint8_t DestDEV = 0x00;
    uint8_t SrcDEV = 0x00;
    uint8_t MsgType = 0x00;
    uint8_t FrameLengthMSB = 0x00;
    uint8_t FrameLengthLSB = 0x00;
    uint8_t block[64] = {};
    uint8_t FramesToGo = 0x00; // Calculated number of remaining frames
    uint8_t SeqNo = 0x00; // Calculated as previous SeqNo++
//    PaddingByte?, // Include this byte if FrameLength is odd
    uint8_t oddChecksum = 0x00;
    uint8_t evenChecksum = 0x00;
} packet_t;

class FBus {
    public:
        FBus(HardwareSerial & serialPort);
        void process();
        // Prepare phone for communication
        void initialize();
        void SetSMSC(char * smsc, fbus_smsc_e type);
        void SetPhoneNumber(char * number, fbus_smsc_e type);
        // Send HWSW request packet
        packet_t* requestHWSW();
        void SendSMS(char * phonenum,char * msgcenter, char * message);
        void SendSMS(char * message);

        void sendAck(byte MsgType, byte SeqNo ); // Aknowledge received packet
        void getACK();
        String versionHW(); // Return hardware version
        String versionSW(); // Return software version
        String versionDate(); // Return software version
        void packBytes();
        byte reverseAndHex(int input);
        void setSMSC(int SMSC_number);
        void pbuf(uint8_t * buf,int len, bool hex);

        unsigned char SendSMS(const char *Message, unsigned char *PhoneNumber);
    private:

        //uint8_t m_block[128] __attribute__ ((aligned (__BIGGEST_ALIGNMENT__)));
        //frame_header_t * m_frame_ptr;

        HardwareSerial & _serialPort; // Serial port attached to phone
        packet_t incomingPacket; // Incoming packet buffer
        frame_header_t outgoingPacket; // Outgoing packet buffer

        fbus_smsc_e m_smsc_type;
        uint8_t m_smsc[10]; // always 10!

        fbus_smsc_e m_phonenumber_type;
        uint8_t m_phonenumber[10]; // always 10!

        void serialFlush(); // Empty the serial input buffer
        uint8_t octetPack(char * instr,uint8_t * outbuf,uint8_t outbuf_size,uint8_t fill);
        void packetSend(frame_header_t * packet_ptr);
        uint8_t BitPack(uint8_t * buffer,uint8_t length);
        uint8_t BitUnpack(uint8_t * buffer,uint8_t length);
        void packetReset(frame_header_t *packet_ptr);
        void processIncomingByte(uint8_t inbyte);
        packet_t* getIncomingPacket(); // Retreive the incoming packet
};

#endif

