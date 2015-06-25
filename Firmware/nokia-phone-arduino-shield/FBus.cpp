// FBus.cpp - Library for talking to an F-Bus device.
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

// Include any necessary files
#include "Arduino.h"
#include "HardwareSerial.h"
#include "FBus.h"
#include "string.h"

#define UINT16_SWAP(V)      do{(V) = ((V)>>8)|((V)<<8);}while(0)

// FrameID values
#define FBUS_VIA_CABLE      0x1E
#define FBUS_VIA_IRDA       0x1C

// DestDEV, SrcDEV values
#define FBUS_DEV_PHONE      0x00
#define FBUS_DEV_HOST       0x0C


// Constructor for FBus class, associates the serial port
// with the member reference
FBus::FBus(HardwareSerial & serialPort)
: _serialPort(serialPort)
{
    // Setup things
    return;
}

// The 'process' routine is used for polling the serial port for data
// from the phone.  All 'NEW' packets are ACKed and then marked as 'READY'
void FBus::process()
{
    char c;
    uint8_t count=0;
    while(_serialPort.available())
    {
        c=_serialPort.read();
        if(c==-1) break;
        count++;
        this->processIncomingByte(c,&incomingPacket);
        if(incomingPacket.packet_state == PACKET_STATE_NEW)
        {
            // We have a new packet here!
            Serial.println("New");
            // Send the ACK
            sendAck(incomingPacket.MsgType, incomingPacket.SeqNo);

            // We could have taken a while to send the packet, so we could be
            // receiving a new packet.  Make sure the packet is not in receiving
            // and then mark it new
            if(incomingPacket.packet_state != PACKET_STATE_RECEIVING)
                incomingPacket.packet_state = PACKET_STATE_READY;
        }
    }

    return;
}

// Prepare phone for communication
void FBus::initialize()
{
    // Clear the RX
    serialFlush();

    m_smsc_type = NUMTYPE_UNKNOWN;
    memset(m_smsc,0,sizeof(m_smsc));

    m_pnum_type = NUMTYPE_UNKNOWN;
    memset(m_phonenumber,0,sizeof(m_phonenumber));

    m_out_seqnum = 0;

    ResetBus(128);

    incomingPacket.packet_state = PACKET_STATE_EMPTY;

    // Phone should be in FBus mode now

    return;
}

// This 'resets' the FBus by sending 0x55 to the phone 'count' times
void FBus::ResetBus(uint8_t count)
{
    // Send 0x55 to initialize the phone
    // http://www.codeproject.com/Articles/13452/A-Simple-Guide-To-Mobile-Phone-File-Transferring#Nokia_FBUS_File_Transferring
    for (int i = 0; i < count; i++) {
        _serialPort.write(0x55);
    }
    _serialPort.flush();
    return;
}

// Returns the current state of the packet,  The states are in the FBus.h header
// file and represent the current state of the packet, if it is being received, if
// is new and needs an ACK, or if it is ready to be processed
uint8_t FBus::GetPacketState()
{
    return incomingPacket.packet_state;
}

// Marks a packet as empty
void FBus::ClearPacket()
{
    if(incomingPacket.packet_state != PACKET_STATE_RECEIVING)
        incomingPacket.packet_state = PACKET_STATE_EMPTY;
    return;
}

// Set the SMS Center routing number and the type based on
// GSM 03.40 ­ Technical realization of the Short Message Service (SMS) Point­to­Point (PP).
void FBus::SetSMSC(char * smsc, fbus_number_type_e type)
{
    uint8_t c;
    //pbuf((uint8_t*)smsc,strlen(smsc),true);
    // TODO: fill might be 0, check
    c = octetPack(smsc,m_smsc,sizeof(m_smsc),0x00);
    //pbuf((uint8_t*)m_smsc,sizeof(m_smsc),true);
    m_smsc_type = type;
    return;
}

// Set the phonenumber to use for the recepiant of the SMS
void FBus::SetPhoneNumber(char * number, fbus_number_type_e type)
{
    uint8_t c;
    //pbuf((uint8_t*)number,strlen(number),true);
    c = octetPack(number,m_phonenumber,sizeof(m_phonenumber),0x00);
    //pbuf((uint8_t*)m_phonenumber,sizeof(m_phonenumber),true);
    m_pnum_type = type;
    return;
}

// Send HWSW request packet
void FBus::RequestHWSW()
{
    // Send HWSW request packet
    //packetReset( &outgoingPacket );
    // Request HWSW information packet
    outgoingPacket.FrameID = FBUS_VIA_CABLE;
    outgoingPacket.DestDEV = FBUS_DEV_PHONE;
    outgoingPacket.SrcDEV = FBUS_DEV_HOST;
    outgoingPacket.MsgType = FBUSTYPE_REQ_HWSW;
    uint8_t block[] = { 0x00, 0x03, 0x00 };
    outgoingPacket.FrameLength = sizeof(block);
    for (int i=0; i<sizeof(block); i++) {
        outgoingPacket.data[i] = block[i];
    }
    outgoingPacket.FramesToGo = 0x01; // Calculated number of remaining frames

    packetSend(&outgoingPacket);

    return;
}

// SendSMS functions
void FBus::SendSMS(char * phonenum,char * msgcenter, char * message)
{
    // Set the SMSC and Phone number, then send the msg
    SetSMSC(msgcenter, NUMTYPE_UNKNOWN);
    SetPhoneNumber(phonenum, NUMTYPE_UNKNOWN);
    SendSMS(message);
    return;
}
void FBus::SendSMS(char * message)
{
    int index=0;
    int x,c,len;

    // Based on Embedtronics testing on a Nokia 3310
    // http://web.archive.org/web/20120712020156/http://www.embedtronics.com/nokia/fbus.html

    outgoingPacket.FrameID = FBUS_VIA_CABLE;
    outgoingPacket.DestDEV = FBUS_DEV_PHONE;
    outgoingPacket.SrcDEV = FBUS_DEV_HOST;
    outgoingPacket.MsgType = FBUSTYPE_SMS;
    // Add in the boilerplate for this pkt
    uint8_t block[] = { 0x00, 0x01, 0x02, 0x00 };
    for (index=0; index<sizeof(block); index++) {
        outgoingPacket.data[index] = block[index];
    }
    outgoingPacket.FrameLength = sizeof(block);

    // Add in the smsc number length (len+type = 7) and SMSC type
    outgoingPacket.data[index++] = 0x07;
    outgoingPacket.data[index++] = (uint8_t)m_smsc_type;
    outgoingPacket.FrameLength+=2;

    // Add in the SMSC number
    for(x=0;x<sizeof(m_smsc);x++)
    {
        outgoingPacket.data[index++]=m_smsc[x];
    }
    outgoingPacket.FrameLength+=sizeof(m_smsc);

    // Add in magic number for outbound SMS type
    // The message is SMS Submit, Reject Duplicates, and Validity Indicator present.
    outgoingPacket.data[index++]=0x15;
    outgoingPacket.FrameLength++;

    // Skip 3 numbers
    for(x=0;x<3;x++)
        outgoingPacket.data[index++]=0;
    outgoingPacket.FrameLength+=3;

    // Add in unpacked message size
    len = strlen(message);
    outgoingPacket.data[index++]=len;
    outgoingPacket.FrameLength++;

    // Add dest number length
    outgoingPacket.data[index++]=sizeof(m_phonenumber);
    outgoingPacket.FrameLength++;

    // Add number type
    outgoingPacket.data[index++] = m_pnum_type;
    outgoingPacket.FrameLength+=2;

    // Add in 10 bytes for phone number
    for(x=0;x<sizeof(m_phonenumber);x++)
    {
        // Blank for now, TODO: fix this
        outgoingPacket.data[index++]=m_phonenumber[x];
    }
    outgoingPacket.FrameLength+=sizeof(m_phonenumber);

    // Validity period, magic number 0xA7
    outgoingPacket.data[index++]=0xA7;
    outgoingPacket.FrameLength++;

    // Timestamp? 6 chars, all 0
    for(x=0;x<6;x++)
    {
        // Blank for now, TODO: fix this
        outgoingPacket.data[index++]=0;
    }
    outgoingPacket.FrameLength+=6;

    // TODO: The SMS message!!!
    c = BitPack((uint8_t*)message,len);
    for(x=0;x<c;x++)
    {
        // Blank for now, TODO: fix this
        outgoingPacket.data[index++]=message[x];
    }
    outgoingPacket.FrameLength+=c;

    outgoingPacket.FrameLength--;

    // TODO: This byte is not in the message, it is the FramesToGo
    // TODO: Always 0? this is byte 93 in the example, the doc says always 0 but the
    // bytes show 0x01
    //outgoingPacket.data[index++]=0x01;
    //outgoingPacket.FrameLength++;

    outgoingPacket.FramesToGo = 0x01; // Calculated number of remaining frames
    //outgoingPacket.SeqNo = m_out_seqnum;

    //pbuf(outgoingPacket.data,outgoingPacket.FrameLength,true);

    packetSend(&outgoingPacket);

    return;
}

// Return the pointer of the RX packet for processing
packet_t* FBus::GetRXPacketPtr()
{
    return &incomingPacket;
}


#ifdef FBUS_ENABLE_DEBUG
// Prints a buffer to the PC Serial as hex
void FBus::pbuf(uint8_t * buf,int len, bool hex)
{
    int x,id;
    char out[400];
    id=0;
    for(x=0;x<len;x++) id+=sprintf(&(out[id]),"x%02X,",buf[x]);
    //printf("\n");
    Serial.println(out);
    return;
}
// Receives a single character command for testing
void FBus::CMD(char c)
{
    switch(c){
    case 'D':
       Serial.println("Dump");
       break;
    case 'A':
        sendAck(0x12, 0x34);
        break;
    case 'P':
        pbuf((uint8_t*)&incomingPacket,incomingPacket.FrameLength+offsetof(packet_t,data),true);
        break;
    default:
       break;
    }
    return;
}
#endif

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

// Pack a given string into reversed octets Eg: "1234" -> 0x21,0x43
uint8_t FBus::octetPack(char * instr,uint8_t * outbuf,uint8_t outbuf_size,uint8_t fill)
{
    char c;
    uint8_t x,i;
    memset(outbuf,0,outbuf_size);
    x=0;
    i=0;
    while(*instr!=0)
    {
        c=*instr;
        if(c>='0' && c<='9')
        {
            //printf("x%02X,",c);
            outbuf[i]=(outbuf[i]&0xF0)|(c-'0');
            //outbuf[i]|=(c-'0');
            if((x&1)==0){
                outbuf[i]<<=4;
                outbuf[i]|=(0x0F&fill);
            }else{
                i++;
            }
            x++;
        }
        instr++;
    }
    //printf("digits: %d\n",x);
    for(i=0;i<(x/2);i++) outbuf[i]=((outbuf[i]&0xF)<<4)|(outbuf[i]>>4);
    return (x/2);
}

// Send a packet, this does all the checksuming and padding, just load up the
// correct info and call this.
// DONT ADD ANY EXTRA PADDING TO MSGS, ALL DONE HERE
void FBus::packetSend(packet_t * packet_ptr)
{
    int count;
    uint8_t * data_ptr;
    uint8_t checksum_odd=0,checksum_even=0;
    data_ptr = (uint8_t*)&(packet_ptr->FrameID);

    if(packet_ptr->MsgType == FBUSTYPE_ACK_MSG)
    {
        // Send the header
        count = (offsetof(packet_t,padding16)-offsetof(packet_t,FrameID));
        UINT16_SWAP(packet_ptr->FrameLength);
        for(int x=0;x<count;x++)
        {
            _serialPort.write(data_ptr[x]);
            if(x&1)
                checksum_odd ^= data_ptr[x];
            else
                checksum_even ^= data_ptr[x];
        }
        _serialPort.write(packet_ptr->data[0]);
        checksum_even ^= packet_ptr->data[0];
        _serialPort.write(packet_ptr->data[1]);
        checksum_odd ^= packet_ptr->data[1];
    }else{
        packet_ptr->SeqNo = m_out_seqnum++;
        count = packet_ptr->FrameLength;
        count += (offsetof(packet_t,data)-offsetof(packet_t,FrameID));
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
    }
    // Now send the checksums
    _serialPort.write(checksum_even);
    _serialPort.write(checksum_odd);

    return;
}

// 7bit packing algorithm based on
// GSM 03.38 ­ Alphabets and language­specific information.
//
// Improved bit packing algorithm based on Embedtronics page.  This
// overwrites the same buffer so is more efficient
// max length is 255
// http://web.archive.org/web/20120712020156/http://www.embedtronics.com/nokia/fbus.html
uint8_t FBus::BitPack(uint8_t * buffer,uint8_t length)
{
    uint8_t holder;
    uint8_t bucket;
    uint8_t x=0,shifted=0,i;

    for(i = 0; i < length; i++ )
    {
        holder = buffer[i] & 0x7f;
        holder >>= shifted;
        bucket = buffer[i+1] & 0x7f;
        bucket <<= (7-shifted);
        shifted += 1;
        holder = holder | bucket;
        if (shifted >= 7) {
            shifted = 0;
            i++;
        }
        buffer[x] = holder;
        x++;
    }

    return x;
}

// TODO
// Unpack a 7bit encoded string
// We will need (length/7) extra bytes for the message
//uint8_t decode[128];
uint8_t FBus::BitUnpack(uint8_t * buffer,uint8_t length)
{
    // TODO

    return 0;
}

// Clear all data in a packet
void FBus::packetReset(packet_t *packet_ptr)
{
    memset((uint8_t*)packet_ptr,0,sizeof(packet_t));
    return;
}

// Byte-wise process the data stream
void FBus::processIncomingByte(uint8_t inbyte, packet_t * pktptr)
{
    // TODO
    // - Add a watchdog timer to this. Reset fieldIndex to zero if no change after a while
    // - Put this in the Arduino library
    // - Make a struct that uses this
    // - Trigger using a serial input interrupt. See http://stackoverflow.com/questions/10201590/arduino-serial-interrupts

    switch (pktptr->input_state)
    {
        case 0x00:  // FrameID
            if ( inbyte == FBUS_VIA_CABLE) {
                // This is the start of a new packet
                pktptr->FrameID = inbyte;
                pktptr->rx_blockIndex = 0;
                pktptr->packet_state = PACKET_STATE_RECEIVING;
                pktptr->input_checksum_odd = 0;
                pktptr->input_checksum_even = 0;
                pktptr->input_checksum_odd^=inbyte;
                pktptr->input_state++;
            } else {
                pktptr->input_state=0;
            }
            break;
        case 0x01:  // DestDEV
            if ( inbyte == FBUS_DEV_HOST) {
                pktptr->DestDEV = inbyte;
                pktptr->input_state++;
                pktptr->input_checksum_even^=inbyte;
            } else {
                pktptr->input_state=0;
            }
            break;
        case 0x02:  // SrcDEV
            if ( inbyte == FBUS_DEV_PHONE) {
                pktptr->SrcDEV = inbyte;
                pktptr->input_state++;
                pktptr->input_checksum_odd^=inbyte;
            } else {
                pktptr->input_state=0;
            }
            break;
        case 0x03:  // MsgType
            pktptr->MsgType = inbyte;
            pktptr->input_checksum_even^=inbyte;
            pktptr->input_state++;
            break;
        case 0x04:  // FrameLengthMSB
            pktptr->FrameLength = inbyte;
            pktptr->FrameLength<<=8;
            pktptr->input_checksum_odd^=inbyte;
            pktptr->input_state++;
            break;
        case 0x05:  // FrameLengthLSB
            pktptr->FrameLength += inbyte;
            // If we don't have data, skip to FramesToGo
            pktptr->input_checksum_even^=inbyte;
            if(pktptr->FrameLength<=2)
            {
                pktptr->rx_blockIndex=1;
                pktptr->input_state+=2;
            }else{
                pktptr->input_state++;
            }
            break;
        case 0x06:  // {block}
            pktptr->data[pktptr->rx_blockIndex] = inbyte;

            if(pktptr->rx_blockIndex&1)
            {
                pktptr->input_checksum_even^=inbyte;
            }else{
                pktptr->input_checksum_odd^=inbyte;
            }

            pktptr->rx_blockIndex++;
            if(pktptr->rx_blockIndex >= pktptr->FrameLength - 2 )
            {
                pktptr->rx_blockIndex--;
                pktptr->input_state++;
            }
            break;
        case 0x07:  // FramesToGo
            pktptr->FramesToGo = inbyte;
            if(pktptr->rx_blockIndex&1)
            {
                pktptr->input_checksum_odd^=inbyte;
            }else{
                pktptr->input_checksum_even^=inbyte;
            }
            pktptr->input_state++;
            break;
        case 0x08:
            pktptr->SeqNo = inbyte;
            if((pktptr->rx_blockIndex+1)&1)
            {
                pktptr->input_checksum_odd^=inbyte;
            }else{
                pktptr->input_checksum_even^=inbyte;
            }
            pktptr->input_state++;
            break;
        case 0x09:  // 0x00 or oddChecksum
            if(pktptr->input_checksum_odd!= inbyte)
                pktptr->packet_state = PACKET_STATE_CHECKSUM_FAIL;
            pktptr->input_state++;
            break;
        case 0x0A:// Packet complete
            if(pktptr->input_checksum_even!= inbyte)
                pktptr->packet_state = PACKET_STATE_CHECKSUM_FAIL;
            // Take 2 off the length because the FramesToGo and SeqNum are removed
            pktptr->FrameLength -= 2;

            if(pktptr->packet_state != PACKET_STATE_CHECKSUM_FAIL)
            {
                pktptr->packet_state = PACKET_STATE_NEW;
            }

            // Reset the packet rx state info
            pktptr->input_state=0;
            break;
        default: 
            break;
            // We should never get here. Throw error?
    }

    return;
}

// Send an ACK packet for a given MsgType and SeqNo
void FBus::sendAck(byte MsgType, byte SeqNo )
{
    // Acknowledge packet
    outgoingPacket.FrameID = FBUS_VIA_CABLE;
    outgoingPacket.DestDEV = FBUS_DEV_PHONE;
    outgoingPacket.SrcDEV = FBUS_DEV_HOST;
    outgoingPacket.MsgType = FBUSTYPE_ACK_MSG;
    outgoingPacket.FrameLength = 0x0002;
    outgoingPacket.data[0] = MsgType;
    outgoingPacket.data[1] = SeqNo & 0x07;

    packetSend( &outgoingPacket );

    return;
}


// Old functions
// TODO: Move them into a higher level phone class

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

// eof
