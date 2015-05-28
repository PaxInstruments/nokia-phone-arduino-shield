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
{ FrameID, DestDEV, SrcDEV, MsgType, 0x00, FrameLength, {block}, FramesToGo,
      SeqNo, PaddingByte?, ChkSum1, ChkSum2 }
     
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




