# Pax Instruments
# Nokia Phone Arduion Shield

## Overview
This project is the Pax Instruments Nokia Phone Arduino Shield

## Usage

### Hardware and software information

### Send a text message
Sending a text message requires three pieces of information: a string of text, the recipiant phone number, and the SMS center number, which will relay the text string to the recipiant phone number.

`setSMSC( <SMSC number> )` Set the default SMSC number to use where `<SMSC number>` is an interget value.

`setPhoneNumber( <phone number> )` Set the default recipiant phone number where `phone_number` is an interget value.

`send( <text message> )` Send a text message where `<text message>` is a string value.

A non-default SMSC number and phone number can be used for each text message.

`send( <text message>, <phone number> )` Send a text message to a specified phone number.

`send( <text message>, <phone number>, <SMSCnumber> )` Send a text message to a specified phone number via a specifiec SMSC number.



