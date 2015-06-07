# Pax Instruments
# F-Bus Library Notes

## Overview
This are notes compiled while researching the Nokia F-Bus protocol.

## Technical description
Fbus (for "Fast Bus") is an ANSI/IEEE data bus oriented towards backplanes and cell phones. The standard specifies a way for various pieces of electronic hardware to communicate, typically with one piece acting as master (sending a request), and another acting as a slave (returning an answer). The F-Bus is a bi-directional full-duplex serial type bus running at 115,200 bit/s, 8 data bits, no parity, one stop bit (8N1). Much like a standard RS-232 serial port, F-Bus connections use one pin for data transmit, one pin for data receive and one pin for ground.

The Fastbus standard specifies completely the size, power requirements, signalling levels, and communications protocols for boards that live in a Fastbus crate, which is also a part of the specification. -[Wikipedia](http://en.wikipedia.org/wiki/FBus)

- bi-directional
- full-duplex
- 115,200 bits/s
- 8 data bits
- No parity
- One stop bit (8N1)

## Applications

There are F-Bus adapters that plug into the battery area of a phone. This has pogo pins that interface with the F-Bus contacts within the battery compartment. There are probably contacts that connect to the battery terminals to provide power.

- Set of adapters that are just PCBs with contacts and mayer level shifters that connect to a standard FTDI cable.
- Put the whole Arduino in the battery compartment. The Arduino communicates with the phone over F-Bus and also to overther devices on the F-Bus.

## Communication steps
There are three steps.

1. Source (computer/microcontroller) sends command to the phone.
2. Phone sends feedback back to source and performs command.
3. Source sends acknowledgement of feedback to phone.

If source does not acknowledge phone, phone sends feedback again, repeat, repeat, terminate communication.

- UART syncronization. Send 128 0x55 ("U") bytes to the phone. A 'U' in ASCII is 01010101. This sends 01010101010101010101...

Message breakdown:

```
// Procesing incoming bytes

void serialInterrupt() {
// Execute this function on serial input interrupt
    while ( _serialPort->available ) {
        processIncomingByte();
    }
}

byte outgoingFieldIndex = 0;
byte outgoingBlockIndex = 0;
byte outgoingByte;
packet outgoingPacket;

byte incomingFieldIndex = 0;
byte incomingBlockIndex = 0;
byte incomingByte;
packet incomingPacket;
  
void processIncomingBytes() {
// Process each incomingByte in serial input buffer. As we cycle throught the first
// three bytes we make sure each ones matches the expected values of a packet
// addressed to us. We trash bytes until we see the first three bytes of a packet
// header, then we assume the rest is a packet.
//
// TODO
// - Add a watchdog timer to this. Reset fieldIndex to zero if no change after a while
// - Put this in the Arduino library
// - Make a struct that uses this
// - Trigger using a serial input interrupt. See http://stackoverflow.com/questions/10201590/arduino-serial-interrupts
//
    switch (incomingFieldIndex) { // oddCheckSum ^= packet
        case 0:
          incomingPacket.FrameID = _serial->read();
          if ( incomingPacket.FrameID != F_BUS_CABLE) break;
          incomingPacket.oddChecksum = 0x00;
          incomingPacket.oddChecksum ^= incomingPacket.FrameID;
          incomingFieldIndex++;
          break;
        case 1:
          incomingPacket.DestDEV = _serial->read();
          if ( incomingPacket.DestDEV != HOST_DEVICE) {
              fieldIndex = 0;
              break;
          }
          incomingPacket.evenChecksum = 0x00;
          incomingPacket.evenChecksum ^= incomingPacket.DestDEV;
          incomingFieldIndex++;
          break;
        case 2:
          incomingPacket.SrcDEV = _serial->read();
          if ( incomingPacket.SrcDEV != PHONE) {
              incomingFieldIndex = 0;
              break;
          }
          incomingPacket.oddChecksum ^= incomingPacket.SrcDEV;
          incomingFieldIndex++;
          break;
        case 3:
          incomingPacket.MsgType = _serial->read();
          incomingPacket.evenChecksum ^= incomingPacket.MsgType;
          incomingFieldIndex++;
          break;
        case 4:
          incomingPacket.FrameLengthMSB = _serial->read();
          if (incomingPacket.FrameLengthMSB != 0x00 ) {
                // Throw an error
                incomingFieldIndex = 0;  // Trash packet and start over
                break;
            }
          incomingPacket.oddChecksum ^= incomingPacket.FrameLengthMSB;
          incomingFieldIndex++;
          break;
        case 5:
            incomingPacket.FrameLengthLSB = _serial->read();
            if (incomingPacket.FrameLengthLSB > PACKET_SIZE_MAX ) {
                // Throw an error
                incomingFieldIndex = 0;  // Trash packet and start over
                incomingBlockIndex = 0;
                break;
            } elif (incomingPacket.FrameLengthLSB < PACKET_SIZE_MAX ) {
                // Throw an error
                incomingFieldIndex = 0;  // Trash packet and start over
                incomingBlockIndex = 0;
                break;
            }
            incomingPacket.evenChecksum ^= incomingPacket.FrameLengthLSB;
            incomingFieldIndex++;
            break;
        case 6:
            // Process the block
            incomingFieldIndex++;
            break;
        case 7:
            incomingPacket.FramesToGo = _serial->read();
            // Checksum odd/even is conditional based on incomingPacket.FrameLengthLSB
            // Run checksum maybe using (incomingPacket.FrameLengthLSB & 0x01)
            incomingFieldIndex++;
            break;
        case 8:
            incomingPacket.SeqNo = _serial->read();  // Last byte in FrameLength
            // Checksum odd/even is conditional based on incomingPacket.FrameLengthLSB
            // Run checksum maybe using (incomingPacket.FrameLengthLSB & 0x01)
            incomingFieldIndex++;
            break;
        case 9:
        // From here to the end we are working with the checksums. We do an XOR of the
        // checksums the phone gives us against the checksums we calculated. If they are
        // the same, the results will be 0x00.
            if ( incomingPacket.FrameLengthLSB is odd ) {
                _serial->read();  // Trash padding byte
            } else
                incomingPacket.oddChecksum ^= _serial->read();
            }
            incomingFieldIndex++;
            break;
        case 10:
            if ( incomingPacket.FrameLengthLSB is odd ) {
                incomingPacket.oddChecksum ^= _serial->read();
            } else
                incomingPacket.evenChecksum ^= _serial->read();
            }
            incomingFieldIndex++;
            break;
        case 11:
            if ( incomingPacket.FrameLengthLSB is odd ) {
                incomingPacket.evenChecksum ^= _serial->read();
            }
            if ( oddChecksum || evenChecksum ) {
            // Throw an error if the delivered and calculated checksums are not the same
                // Bad packet!
                // Throw checksum error
                incomingFieldIndex = 0;  // Trash packet and start over
                incomingBlockIndex = 0;
                break;
            }
            // Packet complete.
            // Do something.
            break;
        default: 
          // We should never get here. Throw error
  }
}
```


```
// Packet:

{ FrameID, DestDEV, SrcDEV, MsgType, FrameLengthMSB, FrameLengthLSB, {block}, FramesToGo,
      SeqNo, PaddingByte?, oddCheckSum, evenCheckSum }
typedef struct { 
    byte FrameID, 
    byte DestDEV, 
    byte SrcDEV, 
    byte MsgType, 
    byte FrameLengthMSB, 
    byte FrameLengthLSB,
    byte* block, // Points to the command block array
    byte FramesToGo, // Calculated number of remaining frames
    byte SeqNo, // Calculated as previous SeqNo++
//    PaddingByte?, // Include this byte if FrameLength is odd
    byte oddCheckSum
    byte evenCheckSum
} packet;

Byte, Name
00 FrameID
  - 0x1E: Any frame sent via cable
  - 0x1C: Any frame sent via IRDA port
01 DestDEV
  - 0x00: Mobile phone
  - 0x0C: PC/uC
02 SrcDEV
  - 0x00: Mobile phone
  - 0x0C: PC/uC
03 MsgType
  - 0x1B: Get HW & SW version (on Nokia 3100)
  - Depends on the phone used
  - Depends on the command
  - See Gnokii documentation
04 FrameLength MSB
  - 0x00: If FrameLength LSB is 0xFF or less
  - Gnokii F-Bus docs leave it as 0x00
05 FrameLength LSB
  - Number of bytes from Byte 06 to Byte N-3
06 ??
  - 0x00: For non-acknowledgement packets
  - First byte in **FrameLength**
07 ??
  - 0x01: For non-acknowledgement packets
08 ??
  - 0x00: For non-acknowledgement packets
09 to N-5 {block}
  - This is the comand block
  - Depends on the phone used
  - Depends on the command
  - See Gnokii documentation
N-4 FramesToGo
  - 0x01: If this message is the last frame
  - The number of frames remaining the message including this one
N-3 SeqNo
  - 0x40: First frame
  - 0xXY format
  - X is usually 4
  - Y is 0-7
  - For each new message we write we incriment the SeqNo
  - The phone returns component Y (e.g. 0x0Y, which is 0xXY & 0x07) in the acknowledgement packet
  - Last byte in **FrameLength**
N-2 PaddingByte?
  - 0x00: If byte N-3 is odd
  - If byte N-2 is even, do not add padding byte
N-1 ChkSum1
  - XOR all odd bytes (1, 3, 5, etc.)
  - Note: ChkSum1 and ChkSum2 can be calculated together as a 16-bit XOR
N ChkSum2
  - XOR all even bytes (2, 4, 6, etc.)
  - Note: ChkSum1 and ChkSum2 can be calculated together as a 16-bit XOR
```

```
Example (get HW&SW):  
      ID DD SD MT FL FL |<<<<<<<frame>>>>>>|    CS CS  
Byte: 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15
Data: 1E 00 0C D1 00 07 00 01 00 03 00 01 60 00 72 D5

For Nokia 3100? (get HW&SW):  
      ID DD SD MT FL FL |<<<<<<<frame>>>>>>| PD CS CS  
Byte: 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15
Data: 1E 00 0C 1B 00 07 00 01 00 07 00 01 00 00
```

`XX` is the command  
`YY` Calculate message length  
`ZZZZZZ` Data. Pad if necessary  
`AAA` Odd XOR  
`BBB` Even XOR  

From gnokii nokia.txt:

```
Frame format for FBUS version 2/Direct IRDA:

    { FrameID, DestDEV, SrcDEV, MsgType, 0x00, FrameLength, {block}, FramesToGo,
      SeqNo, PaddingByte?, ChkSum1, ChkSum2 }

         where FrameID:         0x1c: IR / FBUS
                                0x1e: Serial / FBUS
               DestDev, SrcDev: 0x00: mobile phone
                                0x0c: TE (FBUS) [eg. PC]
               MsgType:         see List
               FrameLength:     {block} + 2 (+ 1 if PaddingByte exists)
               FramesToGo:      0x01 means the last frame
               SeqNo:           [0xXY]
                                  X: 4: first block
                                     0: continuing block
                                     Y: sequence number
               PaddingByte:     0x00 if FrameLength would be an odd number
                                anyways it doesn't exists
               ChkSum1:         XOR on frame's odd numbers
               ChkSum2?:        XOR on frame's even numbers
```


## Archetecture

Use cases:

- A microcontroller takes readings every ten minutes. Every hour the microcontroller averagest those readings and texts them to a phone number.
- Microcontroller is logging data. Peridotically it sends text messages to a server with the data.
- Microcontroller receives text message and controlls something based on those messages.
- My bank sends me an online banking access code while traveling. The device receives the texted code locally then texts the code to the phone I have while travelling. It may also send an email to me.


## Random notes

- Have a list of commands and hex values in a series of #define statements. This will replace those commands with the appropriate byte at compile time. Use #define statements or variables?
- I can pass bytes 0 thorugh N to checksumOdd(), which cats the odd checksum on the end. Then pass that whole thing through checksumEven(), which cats the checksum on the end. The odd checksum is always an odd byte, so checksumEven() never looks at it.
- From Gnokii the Nokia 3100 uses nk6510.c. Here are all the compatible phones listed with the 3100: 6510, 6310, 8310, 6310i, 6360, 6610, 6100, 5100, 3510, 3510i, 3595, 6800, 6810, 6820, 6820b, 6610i, 6230, 6650, 7210, 7250, 7250i, 7600, 6170, 6020, 6230i, 5140, 5140i, 6021, 6500, 6220, 3120b, 3100, 3120, 6015i, 6101, 6680, 6280, 3220, 6136, 6233, 6822, 6300, 6030, 3110c, series60, series40
- The Nokia 3100 uses FBUS version 2
- All the weird bullshit that was happening may have been due to me using nk6110.txt commands rather than nk 6510.txt commands. The software may accept a bunch of identify commands just so it can handshake and pass output the device's actual model number. Maybe when the phone reads a message that seems like the identify request it sends back identify responses in several formats in hopes the host will be able to read one and determine the phone's model number.
- What are bytes six, seven and maybe eight for? 
- It looks like the message I compose using the gnokii docs from git is different from what I see gnokii transmitting. I'll have to get better data when the new boards some in.
- Try to design things such that the uC does not have to store large values. Process and pack items as they come in. checksumOdd() and checksumEven() can be called as a message is being made. Then we only need to store the result in memeory.
- According to InsideGadgets, "After lots of testing, I found that you should firstly send 128 ‘U’, then send the request for the HW&SW command and then send the SMS."

```
getPacket () {  // private
    // Assumes the next batch of data is a packet
    // The serial input buffer does not need to be flushed
    // This function must loop until a packet is found and read completely or timeout occurs
    int oddCheckSum = 0, evenCheckSum = 0;
    1. Read the serial buffer until bytes 0-2 match
    2. Write bytes 0-2 to header[]
    3. Write bytes 3-5 to header[]
    4. Checksum header[]
    5. Process body: next header[5] bytes
       - Read the next byte
       - write byte to body[]
       - checksum byte
    6. if ( sizeof(body) is odd ) { write next 3 bytes to footer }
       else { write next 2 bytes to footer }
    7. Verify packet integrity
       - if ( oddChecksum == footer[ sizeof(footer) - 2 ] &&
             evenCheckSum == footer[ sizeof(footer) - 1 ] ) { 
             // Send acknowledgement
         }
    8. Send acknowledgement
    return msgType, 
}
```

Pass body[] to appropriate function as dictated by byte 03 MsgType

- The bytes in body[0] to body[2] are trashed
- The bytes body[3] to body[ sizeof(body) - 2 ]
- The bytes body[ sizeof(body) - 2 ] is FramesToGo
- The bytes body[ sizeof(body) - 1 ] is SeqNo


```
sendAck(byte* &body[]) {  // private
    // construct and send acknowledgement packet
}
    
```














