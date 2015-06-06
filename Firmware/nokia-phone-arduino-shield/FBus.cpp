//  fbus.cpp - Library for talking to an F-Bus device.
//  Created by Charles Pax for Pax Instruments, 2015-05-23
//  Please visit http://paxinstruments.com/products/
//  Released into the Public Domain
//  
//  This library is designed for use with the Arduino Leonardo though
//  it may work with other boards.
//
//  Call this by passing the pointer to a serial port:
//  FBus fred(&Serial1);

// Enable/disable serial debug output
#define DEBUG  1  // 0 disable, 1 enable
#if DEBUG
    #define DEBUG_PRINT(x)    Serial.print (x)
    #define DEBUG_PRINT_HEX(x)    Serial.print (x, HEX)
    #define DEBUG_PRINTLN(x)  Serial.println (x)
    #define DEBUG_WRITE(x)  Serial.write (x)
#else
    #define DEBUG_PRINT(x)
    #define DEBUG_PRINT_HEX(x)
    #define DEBUG_PRINTLN(x)
    #define DEBUG_WRITE(x)
#endif

// Include any necessary files
#include "Arduino.h"
#include "FBus.h"

FBus::FBus(Stream *serialPort) {
// Create the FBus object. Within the library we will reference the phone
// by talking to the serial port to which it is connected. To write to the
// serial port we use the command "_serialPort->write();" and to read we use 
// the command "_serialPort->read()".
//
// TODO: none
    _serialPort = serialPort;
}

FBus::FBus(Stream *serialPort, int SMSCenter) {
// Create the FBus object and define a known SMS Center number.
//
// TODO
// - Implement the SMS Center features. We have nothing now.
//
    _serialPort = serialPort;
    _SMSCenter = SMSCenter;
}

void FBus::initializeBus() {
// Prepare the phone to receive F-Bus messages by sending a series of 128
// ASCII "U" characters. An ASCII "U" is 0x55 in hexidecimal. In binary this
// is 01010101. Basically, we send a long stream of zeros and ones.
//
// TODO: none
    DEBUG_PRINT("Init: ");
    for (int i = 0; i < 128; i++) {
      _serialPort->write(0x55);
      DEBUG_WRITE(0x55);
    }
    DEBUG_PRINTLN();
    
    serialFlush();  // Clear out the serial input buffer
    _serialPort->flush();  // Wait until serial output buffer is empty
}

void FBus::serialFlush() {
// Clear out the serial input buffer
//
// TODO:
// - Return when flush is complete. Function calls should wait while
//   serialFlush()  operates.
  while (Serial.available() > 0) {
    _serialPort->read();
  }
}

void FBus::sendPacket(byte MsgType) {
// Send a packet
//
// TODO
// - Generate SeqNo dynamically
// - Lookup command blocks based on MsgType
//
    byte header[] = { 0x1E, (byte)0x00, 0x0C, MsgType, (byte)0x00, 0x07 };
    byte body[] = { 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x60, 0x00 };
    byte oddCheckSum = 0x00;
    byte evenCheckSum = 0x00;
    byte packet[ sizeof(header) + sizeof(body) + (sizeof(body) & 0x01) + 2 ];
    for ( int i = 0; i < sizeof(header); i++) {
        packet[i] = header[i];
    }
    for ( int i = 0; i < sizeof(body); i++) {
        packet[i + sizeof(header)] = body[i];
    }
    for ( int i = 0; i < sizeof(packet); i++) {
        packet[i + sizeof(header) + sizeof(body)] = 0x00;
    }
    for ( int i = 0; i < sizeof(packet) - 2; i += 2) {
        oddCheckSum ^= packet[i];
        evenCheckSum ^= packet[i + 1];
    }
    if (packet[5] & 0x01) {
        packet[sizeof(packet) - 2 ] = oddCheckSum;
        packet[sizeof(packet) - 1 ] = evenCheckSum;
    }else {
        packet[sizeof(packet) - 3 ] = oddCheckSum;
        packet[sizeof(packet) - 2 ] = evenCheckSum;
      
    }
    _serialPort->write( packet, sizeof(packet) );
    DEBUG_PRINT("msg >>> ");
    for(int i = 0; i < sizeof(packet); i++) {
        DEBUG_PRINT_HEX(packet[i]);
        DEBUG_PRINT(" ");
    }
    DEBUG_PRINTLN();
  
    _serialPort->flush();
}

void FBus::sendAck(byte MsgType, byte SeqNo ) {
// Acknowledge a packet received from the phone by sending
// an acknowledgement packet to the phone. We only need to know
// the message type and sequence numbers of the received packet.
//
// TODO: none
    byte oddCheckSum = 0x00, evenCheckSum = 0x00;
    byte ack[] = { 0x1E,  // FrameID: Cable (0x1E)
                   0x00,  // DestDEV: Phone (0x00)
                   0x0C,  // SrcDEV: PC (0x0C)
                   0x7F,  // MsgType: Acknowledgement (0x7F)
                   0x00,  // FrameLengthMSB, should always be 0x00
                   0x02,  // FrameLengthLSB, should always be 0x00 for an ack packet
                   MsgType,  // The message type we are acknowledging
                   SeqNo & 0x07,  // SegNo: (<SeqNo from message> & 0x07). Sequence ranges from 0 through 7.
                   oddCheckSum,  //  oddCheckSum is calculated later
                   evenCheckSum   //  evenCheckSum is calculated later
                 };
    for ( int i = 0; i < sizeof(ack) - 2; i += 2) {
        oddCheckSum ^= ack[i];
        evenCheckSum ^= ack[i + 1];
    }
    ack[8] = oddCheckSum;
    ack[9] = evenCheckSum;
    _serialPort->write(ack, sizeof(ack));
    DEBUG_PRINT("ack >>> ");
    #if DEBUG
    for(int i = 0; i < sizeof(ack); i++) {
        DEBUG_PRINT_HEX(ack[i]);
        DEBUG_PRINT(" ");
    }
    #endif
    DEBUG_PRINTLN();
}

void FBus::getPacket () {
// Get the next incoming packet.
//
// TODO
// - Search for header match. Don't assume the next batch of data is a packet.
// - Loop until a packet is found and read completely or timeout occurs
// - Maybe there is a serial interrupt we can hook into
// - Deal with packets larger than the serial buffer plus header
// - BUG: We can only process packets less than or equal to  70 bytes (header + serial input buffer size).
//
    int oddCheckSum = 0, evenCheckSum = 0;
    // We should read the serial buffer until bytes 0-2 match
    byte header[6] = {};
    while ( _serialPort->available() < 6) {}
    header[0] = _serialPort->read();  // FrameID: Cable
    header[1] = _serialPort->read();  // DestDEV: PC
    header[2] = _serialPort->read();  // SrcDEV: Phone
    header[3] = _serialPort->read();  // MsgType
    header[4] = _serialPort->read();  // FrameLength MSB
    header[5] = _serialPort->read();  // FrameLength LSB
    int packetLength = sizeof(header) + header[5] + (header[5] & 0x01) + 2;
    byte packet[ packetLength ];  // Create array to hold entire packet
    for (int i = 0; i < sizeof(header); i++) {
        packet[i] = header[i];
    }
    while ( _serialPort->available() < sizeof(packet) - sizeof(header) )  {
    }
    for (byte i = sizeof(header); i < sizeof(packet); i++) {
        packet[i] = _serialPort->read();
    }
    for ( int i = 0; i <= packetLength - 4; i += 2) {
        oddCheckSum ^= packet[i];
        evenCheckSum ^= packet[i + 1];
    }
    byte MsgType = packet[3];
    byte SegNo = packet[ packetLength - 3 - (header[5] & 0x01) ];
    
    if ( packet[3] != 0x7F) {
        DEBUG_PRINT("<<< msg ");
        #if DEBUG
        for(int i = 0; i < sizeof(packet); i++) {
            DEBUG_PRINT_HEX(packet[i]);
            DEBUG_PRINT(" ");
        }
        #endif
        DEBUG_PRINTLN();
        delay(1);
        sendAck(packet[3], packet[ sizeof(packet) - 3 ] );
    }else {
        DEBUG_PRINT("<<< ack ");
        #if DEBUG
        for(int i = 0; i < sizeof(packet); i++) {
            DEBUG_PRINT_HEX(packet[i]);
            DEBUG_PRINT(" ");
        }
        #endif
        DEBUG_PRINTLN();
    }
}

void FBus::sendSMS(byte MsgType) {
// Send an arbitrary SMS message.
//
// TODO
// - Write this funciton. This is fully hardcoded.
//
    char HWSW_block[] = { 0x00, 0x01, 0x00, 0x03, 0x00 };
    _serialPort->write(0x1E);  // FrameID: Cable
    _serialPort->write((byte)0x00);  // DestDEV: Phone
    _serialPort->write(0x0C);  // SrcDEV: PC
    _serialPort->write(MsgType);  // MsgType, depends on phone model
    _serialPort->write((byte)0x00);  // FrameLengthMSB, should always be 0x00
    _serialPort->write(0x07);  // FrameLengthLSB, depends on message
    _serialPort->write( HWSW_block, sizeof(HWSW_block) );  // getBlock(MsgType), depends on MsgType and phone model
    _serialPort->write(0x01);  // FramesToGo, how many packets are left in this message
    _serialPort->write(0x60);  // SeqNo = (previous SeqNo + 1) ^ 0x07
    _serialPort->write((byte)0x00);  // padAndChecksum
    _serialPort->write(0x72);  // padAndChecksum
    _serialPort->write(0xD5);  // padAndChecksum
  
    _serialPort->flush();
}

//String FBus::softwareVersion() {
//    initializeBus();
//    delay(1);
//    byte hwsw[] = { 0x1E, 0x00, 0x0C, 0xD1, 0x00, 0x07, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x60, 0x00, 0x72, 0xD5 }; // get HW and SW info
//    _serialPort->write(hwsw,sizeof(hwsw));
//    _serialPort->flush();
//    char incomingMessage[200];
//    //delay(50);
//    for (int i = 0; _serialPort->available() > 0; i++) {
//        char incomingByte = _serialPort->read();
//        if (incomingByte == 'V') {
//            i = 0;
//        }
//        incomingMessage[i] = incomingByte;
//    }
//    char softwareVersion[5];
//    for (int j = 0; j < sizeof(softwareVersion); j++) {
//        softwareVersion[j] = incomingMessage[j+2];
//    }
//    return softwareVersion;
//}
//
//String FBus::hardwareVersion() {
//    initializeBus();
//    delay(1);
//    byte hwsw[] = { 0x1E, 0x00, 0x0C, 0xD1, 0x00, 0x07, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x60, 0x00, 0x72, 0xD5 }; // get HW and SW info
//    _serialPort->write(hwsw,sizeof(hwsw));
//    char incomingMessage[200];
//    //delay(50);
//    for (int i = 0; _serialPort->available() > 0; i++) {
//        char incomingByte = _serialPort->read();
//        if (incomingByte == 'V') {
//            i = 0;
//        }
//        incomingMessage[i] = incomingByte;
//    }
//    char hardwareVersion[5];
//    for (int j = 0; j < sizeof(hardwareVersion); j++) {
//        hardwareVersion[j] = incomingMessage[j+22];
//    }
//    return hardwareVersion;
//}
//
//String FBus::dateCode() {
//    initializeBus();
//    delay(1);
//    byte hwsw[] = { 0x1E, 0x00, 0x0C, 0xD1, 0x00, 0x07, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x60, 0x00, 0x72, 0xD5 }; // get HW and SW info
//    _serialPort->write(hwsw,sizeof(hwsw));
//    char incomingMessage[200];
//    //delay(50);
//    for (int i = 0; _serialPort->available() > 0; i++) {
//        char incomingByte = _serialPort->read();
//        if (incomingByte == 'V') {
//            i = 0;
//        }
//        incomingMessage[i] = incomingByte;
//    }
//    char dateCode[8];
//    for (int j = 0; j < sizeof(dateCode); j++) {
//        dateCode[j] = incomingMessage[j+16];
//    }
//    return dateCode;
//}

/*
0x02: SMS HANDLING
    s Send SMS              { 0x02, 0x00, 0x00, 0x00, 0x55, 0x55,
                              0x01 (1 big block), 0x02 (submit), length (big block),
			      type, reference, PID, DCS, 0x00, # blocks,
			      blocks... }
    r Send SMS              { 0x03, 0x00, 0x01, 0x0c, 0x08, 0x00, 0x00, 0xdb, 0x55, 0x55, 0x00 }

    s Get SMSC              { 0x14, location, 0x00 }
    r Get SMSC              { 0x15, err,  0x01, 0x0b, 0x28?, location, 0xf8?,
                              format, 0x00, validity, #blocks, blocks ...}
			    where: err - 0x00 ok, 0x02 - empty
			           name block: { 0x81, blklen, namelen (bytes), 0x00, name (unicode) }
				   number block: { 0x82, blklen, type, ?, number (bcd) }
				   where type: 0x01 - default, 0x02 - number

0x0a: NETSTATUS
    s Get RF Level          { 0x0b, 0x00, 0x02, 0x00, 0x00, 0x00 }
    r GET RF Level          { 0x0c, 0x00, 0x01, 0x04, 0x04, level, 0x5f }

0x14: FOLDER/PICTURE SMS HANDLING
    s Get SMS Status        { 0x08, 0x00, 0x01 }
    r Get SMS Status        { 0x09, 0x00, #blocks,
                              type, length, blocknumber,
                              a (2 octets), b (2 octets), c (2 octets), 0x00, 0x55 ,
                              type, length, blocknumber,
                              d (2 octets), e (2 octets), f (2 octets), 0x01, 0x55 }

                              where:
                              a - max. number of messages in phone memory
                              b - Number of used messages in phone memory. These
                                are messages manually moved from the other folders.
                                Picture messages are saved here.
                              c - Number of unread messages in phone memory. Probably
                                only smart messages.
                              d - max. number of messages on SIM
                              e - Number of used messages in SIM memory. These are
                                either received messages or saved into Outbox/Inbox.
                                Note that you *can't* save message into this memory
                                using 'Move' option. Picture messages are not here.
                              f - Number of unread messages in SIM memory

    s Get SMS from folder   { 0x02, memory, folderID, location, location, 0x01, 0x00}
                            where:
			    memory - 0x01 for SIM, 0x02 for phone (SIM only for IN/OUTBOX
                            folderID - see 0x14/0x017B
    r Get SMS from folder   { 0x03, 0x00, 0x01, memory, folderID, locationH, locationL, 0x55, 0x55, 0x55,
                              0x01 (on big block), type, length of big block,
			      [date/time1], [date/time2], # blocks,
			      type, length, data...
			      ... }

    s Delete SMS            { 0x04, memory, folderID, location, location, 0x0F, 0x55 }
    r Delete SMS            { 0x05 }

    s Get folder status     { 0x0c, memory, folderID, 0x0F, 0x55, 0x55, 0x55, 0x55}
                            where: folderID - see 0x14/0x017B
    r Get folder status     { 0x0d, 0x00, length, number of entries (2 bytes),
			    entry1number (2 bytes), entry2number(2 bytes), ..., 0x55[]}

    s Get message info      { 0x0e, memory, folderID, location, location, 0x55, 0x55 }
    r Get message info      { 0x0f, 0x00, 0x01, 0x00, 0x50, memory, type, 0x00, location, FolderID, status

                            where: type = 0x00 - MT
                                          0x01 - delivery report
                                          0x02 - MO
                                          0x80 - picture message
                            where: status=0x01 - received/read
					  0x03 - received/unread
					  0x05 - stored/sent
					  0x07 - stored/not sent

    s Get folder names      { 0x12, 0x00, 0x00}
    r Get folder names      { 0x13, 0x00, number of blocks, blocks... }
			    where block is: { 0x01, blocklen, folderID, length, 0x00, 0x00, padding... }
                               where: folderID = 0x02 - Inbox
                                                 0x03 - Outbox
                                                 0x04 - Archive
                                            0x05 - Templates
                                            0x06 - first "My folders"
                                            0x07 - second "My folders"
                                            0x08 - third -"-
                                            and so on
				      blocklen = length of the block including
				                 the 0x01 and blocklen itself
						 e.g.: 0x28 - 6510
						       0x58 - 6610

0x17: BATTERY
    s Get battery level     { 0x0a, 0x02, 0x00 }
    r Get battery level     { 0x0b, 0x01, 0x01, 0x16, level, 0x07, 0x05 }
                            where: level: 1-7 (as in phone display)

0x19: CLOCK
    s Get date              { 0x0a, 0x00, 0x00 }
    r Get date              { 0x0b, 0x00, 0x02 (blocks),
                              0x01 (type), 0x0c (length), 0x01, 0x03, year (2 octets), month, day, hour, minute, second, 0x00,
                              0x04, 0x04, 0x01, 0x00 }

0x1b: IDENTITY
    s Get IMEI              { 0x00, 0x41 }
    r Get IMEI              { 0x01, 0x00, 0x01, 0x41, 0x14, 0x00, 0x10, {IMEI(ASCII)}, 0x00 }
    s get HW&SW version     { 0x07, 0x00, 0x01 }
    r get HW&SW version     { 0x08, 0x00, 0x01, 0x58, 0x29, 0x00, 0x22, "V " "firmware\n" "firmware date\n"
                              "model\n" "(c) NMP.", 0x0a, 0x43, 0x00, 0x00, 0x00 }

*/

