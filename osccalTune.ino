// -*- c++ -*-
// Sketch for tuning the OSCCAL value
// 1) Burn the fuses for EEPROM preservation and the one for CLOCK-output over a port 
// 2) Connect the CLOCK port to a frequency meter
// 3) Connect MISO and (ICSP pin 1) and SCK (ICSP pin 3) with buttons switching to GND
// 4) Connect MOSI (ICSP pin  4, opposite to SCK) to RX on a FTDI adapter, 2400 baud
// 5) Press one of the buttons to change the OSCCAL value and observe a frequency counter,
//    the value changes every 0.3 seconds when the button is continously pressed
// 6) When satisfied, wait 30 seconds. The OSCCAL value will then be
//    stored in EEPROM (if STORE_TO_EEPROM is defined), either at the beginning or at the end,
//    depending on whether the compile time switch STORE_AT_END is defined.
// 7) In case, you want to 'erase' the calibration value in EEPROM, just press both buttons and
//    then press reset or invoke a power-on reset. The value will then be over written with 0xFF,
//    provided the compile time constant STORE_TO_EEPROM is defined.
// If you have chip with a serial connection, you can define the compile time constant USE_SERIAL
// and then communicate over the the serial monitor (using the menu).
//
// CLOCK pin:
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
// Version 1.1.0 (16.3.2021)
//  - support usual Serial (on ATmegas etc.) when USE_SERIAL is defined
// Version 2.0.0 (26.06.2023)
//  - changed the location where the calibration value is stored to E2END
//  - defined the ICSP pins so that there is no difference between ATtinys and ATmegas

#define VERSION "2.0.0"
//#define USE_SERIAL
#define STORE_TO_EEPROM // store final value to EEPROM
#define BAUDRATE 9600
#define WAITTIMEMS (30UL*1000UL) // wait before storing to EEPROM
#define REPEATTIMEMS 300
#define LINEMAX 32

// The "usual" pin assignment
#if !defined(SPIE)
#define TXPIN MISO // which is the MOSI-ICSP pin!
#define UPPIN MOSI // which is the MISO-ICSP pin!
#define DOWNPIN SCK
#else
#define TXPIN MOSI 
#define UPPIN MISO 
#define DOWNPIN SCK
#endif

#include <EEPROM.h>
#include <TXOnlySerial.h>

#define EE_ADDR (E2END)

#ifndef OSCCAL // necessary for ATtiny1634
#define OSCCAL OSCCAL0
#endif

#ifdef USE_SERIAL
#define mySerial Serial
#else
TXOnlySerial mySerial(TXPIN); // note: this is the MOSI-ISP pin!
#endif
unsigned long lastpress = millis();
byte lastosccal = 0xFF;

void setup(void)
{
#ifdef STORE_TO_EEPROM
  byte osccal = EEPROM.read(EE_ADDR);
  byte valid =  EEPROM.read(EE_ADDR-1);
#else
  byte osccal = 0xFF;
  byte valid = 0xFF;
#endif
  mySerial.begin(BAUDRATE);
  mySerial.println(F("\r\n\nosccalTune V" VERSION "\n"));
  mySerial.print(F("Factory OSCCAL value: 0x"));
  mySerial.println(OSCCAL, HEX);
  pinMode(UPPIN, INPUT_PULLUP); // note: this is the MISO-ISP pin!
  pinMode(DOWNPIN, INPUT_PULLUP);
#ifdef STORE_TO_EEPROM
  if (digitalRead(UPPIN) == LOW && digitalRead(DOWNPIN) == LOW) {
    mySerial.println(F("Erasing stored OSCCAL value in EEPROM"));
    EEPROM.write(EE_ADDR,0xFF);
    EEPROM.write(EE_ADDR-1,0xFF);
    while (1);
  }
#endif
  if (valid != 0) osccal = OSCCAL;
  mySerial.print(F("Initial OSCCAL value: 0x"));
  mySerial.println(osccal,HEX);
  OSCCAL = osccal;
}

void loop(void)
{
#ifdef USE_SERIAL
  char line[LINEMAX+1];
  char *succ;
  Serial.print(F("\nUse one of the following commands: \n\r\
 S      - show current OSCCAL\n\r\
 E      - save current OSCCAL value to EEPROM \n\r\
 I      - increase OSCCAL value by one \n\r\
 I<num> - increase OSCCAL by <num>\n\r\
 D      - decrease OSCCAL by one\n\r\
 D<num> - decrease OSCCAL by <num>\n\r\
Command: "));
  readline(line);
  Serial.println();
  switch (toupper(line[0])) {
  case 'S': Serial.print(F("OSCCAL=0x"));
    Serial.println(OSCCAL,HEX); break;
  case 'E': Serial.print(F("Saving current OSCCAL value to EEPROM at address 0x"));
    Serial.println(EE_ADDR, HEX);
    EEPROM.put(EE_ADDR,OSCCAL);
    EEPROM.put(EE_ADDR-1,0);
    break;
  case 'I':
    if (line[1] == '\0') OSCCAL++;
    else OSCCAL += strtol(&line[1], &succ, 10);
    Serial.print(F("OSCCAL=0x"));
    Serial.println(OSCCAL,HEX); break;	 
  case 'D': 
    if (line[1] == '\0') OSCCAL--;
    else  OSCCAL -= strtol(&line[1], &succ, 10);
    Serial.print(F("OSCCAL=0x"));
    Serial.println(OSCCAL,HEX); break;
  default:
    Serial.print(F("Unknown command: "));
    Serial.println(line);
  }
#else
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
  if (digitalRead(UPPIN) == LOW) { // note: this is the MISO-ISP pin
    OSCCAL++;
    lastpress = millis();
    pressed = true;
  } else if (digitalRead(DOWNPIN) == LOW) {
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
#endif
}

#ifdef USE_SERIAL

void readline(char buf[])
{
  byte ix = 0;
  char c = '\0';

  while (true) {
    if (Serial.available()) {
      c = Serial.read();
      if (c == '\n' || c == '\r') {
	buf[ix] = '\0';
	return;
      } else {
	buf[ix++] = c;
	Serial.print(c);
	if (ix >= LINEMAX) ix--;
      }
    }
  }
}
#endif
