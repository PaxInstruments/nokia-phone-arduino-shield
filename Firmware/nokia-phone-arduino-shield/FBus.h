/*
  FBus.h - Library for talking to an F-Bus device.
  Created by Charles Pax for Pax Instruments, 2015-05-23
  Please visit http://paxinstruments.com/products/
  Released into the Public Domain
*/

#ifndef __FBUS_H__
#define __FBUS_H__

#include "Arduino.h"
#include "stdint.h"

// Uncomment this to enable some debug functions inside the class
//#define FBUS_ENABLE_DEBUG

// The types are defined in
// GSM 03.40 ­ Technical realization of the Short Message Service (SMS) Point­to­Point (PP).
typedef enum{
    NUMTYPE_UNKNOWN = 0x81,
    NUMTYPE_INTERNATIONAL = 0x91,
    NUMTYPE_NATIONAL = 0xA1,
}fbus_number_type_e;

// States used in packet processing
#define PACKET_STATE_EMPTY          0       // Packet is empty
#define PACKET_STATE_NEW            1       // Packet just received, not ACKed
#define PACKET_STATE_READY          2       // Packet ready to be processed
#define PACKET_STATE_RECEIVING      3       // Packet being received, only partially done
#define PACKET_STATE_CHECKSUM_FAIL  4       // Packet failed checksum, drop!
#define PACKET_STATE_ERR            5       // Unknown error


// MsgTypes as defined in Gnokii Project
#define FBUSTYPE_REQ_HWSW   0xD1    // Request hardware and software information
#define FBUSTYPE_ACK_MSG    0x7F    // ACK type
#define FBUSTYPE_SMS        0x02    // SMS related functions


// The ordering of this struct is important, this matches
// the FBus frame starting with FrameID, this way we can
// just send the array without extra processing.
typedef struct {
    uint8_t input_state;
    uint8_t input_checksum_odd;
    uint8_t input_checksum_even;
    uint8_t packet_state;
    uint8_t rx_blockIndex;
    uint8_t FramesToGo;
    uint8_t SeqNo;
    // NOTE: Start transmit here
    uint8_t FrameID;
    uint8_t DestDEV;
    uint8_t SrcDEV;
    uint8_t MsgType;
    uint16_t FrameLength;
    uint16_t padding16;
    uint8_t data[128];
}packet_t;

class FBus {
    public:
        // Constructor for FBus class, associates the serial port
        // with the member reference
        FBus(HardwareSerial & serialPort);

        // The 'process' routine is used for polling the serial port for data
        // from the phone.  All 'NEW' packets are ACKed and then marked as 'READY'
        void process();

        // Prepare phone for communication
        void initialize();

        // This 'resets' the FBus by sending 0x55 to the phone 'count' times
        void ResetBus(uint8_t count);

        // Returns the current state of the packet,  The states are in the FBus.h header
        // file and represent the current state of the packet, if it is being received, if
        // is new and needs an ACK, or if it is ready to be processed
        uint8_t GetPacketState();

        // Marks a packet as empty
        void ClearPacket();

        // Set the SMS Center routing number and the type of the number based on
        // GSM 03.40 ­ Technical realization of the Short Message Service (SMS) Point­to­Point (PP).
        void SetSMSC(char * smsc, fbus_number_type_e type);

        // Set the phonenumber to use for the receiver of the SMS and the type for the number bsaed on
        // GSM 03.40 ­ Technical realization of the Short Message Service (SMS) Point­to­Point (PP).
        void SetPhoneNumber(char * number, fbus_number_type_e type);

        // Send HWSW request packet
        void RequestHWSW();

        // SendSMS functions
        void SendSMS(char * phonenum,char * msgcenter, char * message);
        void SendSMS(char * message);

        // Return the pointer of the RX packet for processing
        packet_t* GetRXPacketPtr();

        #ifdef FBUS_ENABLE_DEBUG
        // Prints a buffer to the PC Serial as hex
        void pbuf(uint8_t * buf,int len, bool hex);
        // Receives a single character command for testing
        void CMD(char c);
        #endif

    private:

        // Variables
        // ---------------------------------

        HardwareSerial & _serialPort;   // Serial port attached to phone
        packet_t incomingPacket;        // Incoming packet buffer
        packet_t outgoingPacket;        // Outgoing packet buffer
        fbus_number_type_e m_smsc_type; // SMSC phone number and type
        uint8_t m_smsc[10];             // Number is always 10!

        fbus_number_type_e m_pnum_type; // Phone number and type
        uint8_t m_phonenumber[10];      // Number is always 10!

        uint8_t m_out_seqnum;           // This is the next sequence number to use

        // Functions
        // ---------------------------------

        // Empty the serial input buffer
        void serialFlush();

        // Pack a given string into reversed octets Eg: "1234" -> 0x21,0x43
        uint8_t octetPack(char * instr,uint8_t * outbuf,uint8_t outbuf_size,uint8_t fill);

        // Send a packet, this does all the checksuming and padding, just load up the
        // correct info and call this.
        void packetSend(packet_t * packet_ptr);

        // 7bit packing algorithm based on
        // GSM 03.38 ­ Alphabets and language­specific information.
        uint8_t BitPack(uint8_t * buffer,uint8_t length);

        // Unpack a 7bit encoded string
        uint8_t BitUnpack(uint8_t * buffer,uint8_t length);

        // Clear all data in a packet
        void packetReset(packet_t *packet_ptr);

        // Byte-wise process the data stream
        void processIncomingByte(uint8_t inbyte,packet_t * pktptr);

        // Send an ACK packet for a given MsgType and SeqNo
        void sendAck(byte MsgType, byte SeqNo ); // Acknowledge received packet

        // Old functions
        String versionHW(); // Return hardware version
        String versionSW(); // Return software version
        String versionDate(); // Return software version
};

#endif

//eof
