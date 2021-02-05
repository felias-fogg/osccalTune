// -*- c++ -*-
// Sketch for tuning the OSCCAL value
// 1) Burn the fuses for EEPROM preservation and the one for CLOCK-output over a port 
// 2) Connect the CLOCK port to a frequency meter
// 3) Connect MISO and (ICSP pin 1) and SCK (ICSP pin 3) with buttons switching to GND
// 4) Connect MOSI (ICSP pin  4, opposite to SCK) to RX on a FTDI adapter, 2400 baud
// 5) Press one of the buttons to change the OSCCAL value and observe frequency counter,
//    the value changes every 0.3 seconds when the button is continously pressed
// 6) When satisfied, wait 30 seconds. The OSCCAL value will then be
//    stored in EEPROM (if STORE_TO_EEPROM is defined), either at the beginning or at the end,
//    depending on whether the compile time switch STORE_AT_END is defined.
// 7) In case, you want to 'erase' the calibration value in EEPROM, just press both buttons and
//    then press reset or invoke a power-on reset. The value will then be over written with 0xFF,
//    provided the compile time constant STORE_TO_EEPROM is defined.
//
// MCU         | CLOCK pin | as Arduino pin
// ATtinyX4    | PB2       | 6
// ATtinyX41   | PB2       | 2 or 8
// ATtinyX5    | PB4       | 4
// ATtinyX61   | PB5       | 4
// ATtinyX7    | PB5       | 13
// ATtiny1634  | PC2       | 11
// ATmegaX8    | PB0       | 8

// Version 1.0.0 (20.1.2021)
//  - first running version
// Version 1.0.1 (3.2.2021)
//  - store at end of EEPROM
//  - baudrate is 9600

#define VERSION "1.0.1"
#define STORE_TO_EEPROM // store final value to EEPROM
#define STORE_AT_END   //  store it at end of EEPROM
#define STORE_OFFSET 2 // reserved for OSCCAL (the other two bytes are reserved for the INTREF value)
#define BAUDRATE 9600
#define WAITTIMEMS (30UL*1000UL) // wait before storing to EEPROM
#define REPEATTIMEMS 300

#include <EEPROM.h>
#include <TXOnlySerial.h>

#ifdef STORE_AT_END
#define EE_ADDR (E2END-STORE_OFFSET)
#else
#define EE_ADDR STORE_OFFSET
#endif

#ifndef OSCCAL // necessary for ATtiny1634
#define OSCCAL OSCCAL0
#endif

TXOnlySerial mySerial(MISO); // note: this is the MOSI-ISP pin!
unsigned long lastpress = millis();
byte lastosccal = 0xFF;

void setup(void)
{
#ifdef STORE_TO_EEPROM
  byte osccal = EEPROM.read(EE_ADDR);
#else
  byte osccal = 0xFF;
#endif
  mySerial.begin(BAUDRATE);
  mySerial.println(F("\r\n\nosccalTune V" VERSION "\n"));
  mySerial.print(F("Factory OSCCAL value: 0x"));
  mySerial.println(OSCCAL, HEX);
  pinMode(MOSI, INPUT_PULLUP); // note: this is the MISO-ISP pin!
  pinMode(SCK, INPUT_PULLUP);
#ifdef STORE_TO_EEPROM
  if (digitalRead(MOSI) == LOW && digitalRead(SCK) == LOW) {
    mySerial.println(F("Erasing stored OSCCAL value in EEPROM"));
    EEPROM.write(EE_ADDR,0xFF);
    while (1);
  }
#endif
  if (osccal == 0xFF) osccal = OSCCAL;
  mySerial.print(F("Initial OSCCAL value: 0x"));
  mySerial.println(osccal,HEX);
  OSCCAL = osccal;
}

void loop(void)
{
  unsigned long start = millis();
  bool pressed = false;
#ifdef STORE_TO_EEPROM
  if (millis() - lastpress > WAITTIMEMS && OSCCAL != EEPROM.read(EE_ADDR)) {
      mySerial.print(F("Saving current value 0x"));
      mySerial.print(OSCCAL, HEX);
      mySerial.print(F(" to EEPROM at 0x"));
      mySerial.println(EE_ADDR, HEX);
      EEPROM.write(EE_ADDR, OSCCAL);
  }
#endif
  if (digitalRead(MOSI) == LOW) { // note: this is the MISO-ISP pin
    OSCCAL++;
    lastpress = millis();
    pressed = true;
  } else if (digitalRead(SCK) == LOW) {
    OSCCAL--;
    lastpress = millis();
    pressed = true;
  }
  if (OSCCAL != lastosccal) {
    mySerial.print(F("OSCVAL: 0x"));
    mySerial.println(OSCCAL, HEX);
  }
  lastosccal = OSCCAL;
  while (millis() - start < REPEATTIMEMS && pressed ) delay(1);
}
