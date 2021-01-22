# Tuning OSCCAL

When you run an AVR MCU without a crystal or resonator, you have to use its own internal oscillator. This is factory calibrated and is promised to be within 10% accuracy. It turns out that the accuracy is usually close to 2-3%. For a given voltage and temperature, you can even do better than that and calibrate to 1% (for a given voltage and temperature).

With this sketch, you can tune the OSCCAL value that controls the frequency of the internal oscillator. You need to burn the fuse that connects the MCU clock to one of the ports (remember to change this after you have run the sketch!) and connect that port to a frequency meter. Often this is provided in multi meters or in signal generators. Perhaps, you can use a DSO or logic analyzer. Without any measurement device, you have to use alternative ways to calibrate the internal oscillator, though, e.g. as described [here](http://ernstc.dk/arduino/tinytuner.html).

Read the comments in the top of the sketch, which describe what to do. If you do not change the sketch, the best OSCCAL value will be stored in EEPROM 2 bytes before the end of EEPROM. But where the value is stored depends all on compile time constants.

The main point is  that you can use the value stored in EEPROM later in your particular application sketch. One important prerequisite for this is, however, that you burn the EEPROM preservation fuse! Otherwise the values stored into EEPROM will always be deleted when a new sketch is uploaded to flash memory. Instead of storing the value in EEPROM, you can, of course, write the value down and insert it as a constant in your application sketch.
 

