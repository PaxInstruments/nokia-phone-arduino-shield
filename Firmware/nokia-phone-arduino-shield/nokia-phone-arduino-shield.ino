// nokia-phone-arduino-shield.ino - Example for F-Bus library.
// For  reference see https://github.com/davidcranor/MCP2035/blob/master/MCP2035.h
// Maybe work with structs
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
// Notes
// - Plug into cranor's tinyTerminal
// For reference on structs see https://github.com/davidcranor/MCP2035/blob/master/MCP2035.h

#include "FBus.h"

FBus myPhone(Serial1);

void setup()
{
    // Start PC serial link
    Serial.begin(115200);

    // Start Phone serial link
    Serial1.begin(115200);

    // wait for serial port to be opened by the PC. This
    // will block until we connect a serial terminal
    //while(!Serial);

    // Setup the phone interface
    myPhone.initialize();

    // Set our SMSC and the target phone number
    myPhone.SetSMSC("8613010888500",NUMTYPE_NATIONAL);
    myPhone.SetPhoneNumber("15622834051",NUMTYPE_UNKNOWN);

}
    
void loop()
{
    // The phone needs to poll for RX butes from the serial port
    // in the 'process' routine.
    myPhone.process();

    // After the process routine we could have a packet marked as
    // 'READY', if we do then we should process it.
    if(myPhone.GetPacketState() == PACKET_STATE_READY)
    {
        // If we have a packet that's ready, do something
        Serial.println("Got new packet!");

        packet_t* pkt = myPhone.GetRXPacketPtr();

        // Example:
        if(pkt->MsgType == FBUSTYPE_REQ_HWSW)
        {
            Serial.println("We got hardware info!");
            //Serial.print("HW: ");
            //Serial.print("SW: ");
        }

        // Clear the packet so we don't process it again
        myPhone.ClearPacket();
    }

    // Simple single character terminal
    if(Serial.available())
    {
        char c;
        c = Serial.read();
        switch(c){
        case 'C':
            myPhone.ResetBus(128);
            break;
        case 'I':
            Serial.println("Req info");
            myPhone.RequestHWSW();
            break;
        default:
            #ifdef FBUS_ENABLE_DEBUG
            myPhone.CMD(c);
            #endif
            break;
        }
    }
}

#if 0

// Setup timer 3 but don't start it
void timer3_setup(float _pulseTime)
{
    cli();

    // Clear timer 3
    TCCR3A = 0;
    TCCR3B = 0;

    // Designed for YUN
    // set Compare Match value:
    // ATmega32U crystal is 16MHz
    // timer resolution = 1/( 16E6 /64) = 4E-6 seconds for 64 prescaling
    // target time = timer resolution * (# timer counts + 1)
    // so timer counts = (target time)/(timer resolution) -1
    // For 1 ms interrupt, timer counts = 1E-3/4E-6 - 1 = 249
    OCR3A = (int)(_pulseTime * 249);

    //Turn on CTC mode:
    TCCR3B |= (1 << WGM32);

    // enable timer compare interrupt:
    TIMSK3 |= (1 << OCIE3A);

    sei();

    return;
}

void timer3_start()
{
    cli();

    //Start the timer counting, with prescaler 64
    TCCR3B |= (1 << CS31) | (1 << CS30);

    sei();
}

void timer3_stop()
{
    cli();
    //Stop the timer counting
    TCCR3B &= 0B11111000;
    sei();
}

void timer3_reset()
{
    cli();
    TCCR3A = 0;  // set all bits of Timer/Counter 3 Control Register to 0
    TCCR3B = 0;
    TCNT3 = 0;
    OCR3A = 0;
    sei();
}

// Timer 3 ISR
ISR(TIMER3_COMPA_vect)
{
    digitalWrite(pin_Pulse, HIGH);

    timer3_stop(); // Stop pTimer
}


#endif

//eof
