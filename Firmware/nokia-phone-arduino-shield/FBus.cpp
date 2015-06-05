/*
  fbus.cpp - Library for talking to an F-Bus device.
  Created by Charles Pax for Pax Instruments, 2015-05-23
  Please visit http://paxinstruments.com/products/
  Released into the Public Domain

  Call this by passing the pointer to a serial port:
  FBus fred(&altSerial);
*/

#include "Arduino.h"
#include "FBus.h"

FBus::FBus(Stream *serialPort) {
  _serialPort = serialPort;
}

void FBus::initializeBus() {  // Perpares phone to receice F-Bus messages
  for (int i = 0; i < 128; i++) {
    _serialPort->write(0x55);
  }
  _serialPort->flush();
}

void FBus::sendPacket(byte MsgType) {
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

void FBus::sendAck(byte MsgType, byte SeqNo ) {
  int oddCheckSum = 0, evenCheckSum = 0;
  byte ack[] = { 0x1E,   // FrameID: Cable
                 0x00,   // DestDEV: Phone
                 0x0C,   // SrcDEV: PC
                 0x7F,   // MsgType: Ack, depends on phone model
                 0x00,   // FrameLengthMSB, should always be 0x00
                 0x02,   // FrameLengthLSB, should always be 0x00
//                 0xD2,   //** The message type we are acknowledging
                 MsgType,
                 SeqNo & 0x07,   //** SegNo: <SeqNo from message> & 0x07
                 0x00, 0x00  //** Checksums
               };
    for ( int i = 0; i <= sizeof(ack) - 4; i += 2) {
        oddCheckSum ^= ack[i];
        evenCheckSum ^= ack[i + 1];
    }
    ack[sizeof(ack)-2] = oddCheckSum;
    ack[sizeof(ack)-1] = evenCheckSum;
  _serialPort->write(ack, sizeof(ack));
}

void FBus::getPacket () {
    // Assumes the next batch of data is a packet
    // The serial input buffer does not need to be flushed
    // This function must loop until a packet is found and read completely or timeout occurs
    int oddCheckSum = 0, evenCheckSum = 0;
    // Read the serial buffer until bytes 0-2 match
    byte header[6] = {};
    while ( _serialPort->available() < 6) {}
    header[0] = _serialPort->read();  // FrameID: Cable
    header[1] = _serialPort->read();  // DestDEV: PC
    header[2] = _serialPort->read();  // SrcDEV: Phone
    header[3] = _serialPort->read();  // MsgType
    header[4] = _serialPort->read();  // FrameLength MSB
    header[5] = _serialPort->read();  // FrameLength LSB
    int packetLength = sizeof(header) + header[5] + (header[5] & 0x01) + 2;
    byte packet[ packetLength ];
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
      delay(1);
        sendAck(packet[3], packet[ sizeof(packet) - 3 ] );
    }
    //_serialPort->write( packet, sizeof(packet) );
}
//    2. Write bytes 0-2 to header[]
//    3. Write bytes 3-5 to header[]
//    4. Checksum header[]
//    5. Process body: next header[5] bytes
//       - Read the next byte
//       - write byte to body[]
//       - checksum byte
//    6. if ( sizeof(body) is odd ) { write next 3 bytes to footer }
//       else { write next 2 bytes to footer }
//    7. Verify packet integrity
//       - if ( oddChecksum == footer[ sizeof(footer) - 2 ] &&
//             evenCheckSum == footer[ sizeof(footer) - 1 ] ) {
//             // Send acknowledgement
//         }
//    8. Send acknowledgement
//    return msgType,

void FBus::serialFlush() {
  while (Serial.available() > 0) {
    _serialPort->read();
  }
}

void FBus::sendTest() {  // Send a raw HWSW request.
  byte hwsw[] = { 0x1E, 0x00, 0x0C,
                  0xD1, 0x00, 0x07,
                  0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x60,
                  0x00, 0x72, 0xD5
                };
  _serialPort->write( hwsw, sizeof(hwsw) );
}

//void FBus::listener() { // Watch serial stream for packet header
//    byte header[6] = {};
//    byte firstByte;
//    byte secondByte;
//    byte incomingByte;
//    if ( _serialPort->available() >= 6 && header[0] == 0x00 ) {
//        while (_serialPort->available() > 0) {
//            firstByte = secondByte;
//            secondByte = incomingByte;
//            incomingByte = _serialPort->read();
//            if ( firstByte == 0x1e && secondByte == 0x0c && incomingByte ==0x00 ) {
//                pulse();
//                header[0] = firstByte;
//                header[1] = secondByte;
//                header[2] = incomingByte;
//                header[3] = _serialPort->read();  // MsgType
//                header[4] = _serialPort->read();  // FrameLength MSB
//                header[5] = _serialPort->read();  // FrameLength LSB
//                Serial.write(header,sizeof(header));
//            }
//        }
//    }
//    if ( _serialPort->available() >= header[5] ) {
//        pulse();pulse();pulse();
//    }
//}
//
//void FBus::serialFlush(){
//  while(Serial.available() > 0) {
//    _serialPort->read();
//  }
//}
//
//void FBus::pulse() {
//    digitalWrite(2, HIGH);
//    digitalWrite(3, HIGH);
//    digitalWrite(2, LOW);
//    digitalWrite(3, LOW);
//    digitalWrite(2, HIGH);
//    digitalWrite(3, HIGH);
//    digitalWrite(2, LOW);
//    digitalWrite(3, LOW);
//    digitalWrite(2, HIGH);
//    digitalWrite(3, HIGH);
//}

//FBus::FBus(Stream *serialPort, int SMSCenter) {
//    _serialPort = serialPort;
//    _SMSCenter = SMSCenter;
//}

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
//
//char FBus::checksumOdd() {
//  // Go through the message array and outout an XOR of the odd numbered bytes.
//  // The first byte in the array is odd. This shoudl work both sending and receiving.
//}

//char FBus::checksumEven() {
//  // Go through the message array and outout an XOR of the even numbered bytes.
//  // The first byte in the array is odd. This shoudl work both sending and receiving.
//
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

