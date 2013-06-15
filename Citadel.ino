/*
    Basic Pin setup:
    ------------                                  ---u----
    ARDUINO   13|-> SCLK (pin 25)           OUT1 |1     28| OUT channel 0
              12|                           OUT2 |2     27|-> GND (VPRG)
              11|-> SIN (pin 26)            OUT3 |3     26|-> SIN (pin 11)
              10|-> BLANK (pin 23)          OUT4 |4     25|-> SCLK (pin 13)
               9|-> XLAT (pin 24)             .  |5     24|-> XLAT (pin 9)
               8|                             .  |6     23|-> BLANK (pin 10)
               7|                             .  |7     22|-> GND
               6|                             .  |8     21|-> VCC (+5V)
               5|                             .  |9     20|-> 2K Resistor -> GND
               4|                             .  |10    19|-> +5V (DCPRG)
               3|-> GSCLK (pin 18)            .  |11    18|-> GSCLK (pin 3)
               2|                             .  |12    17|-> SOUT
               1|                             .  |13    16|-> XERR
               0|                           OUT14|14    15| OUT channel 15
    ------------                                  --------

    -  Put the longer leg (anode) of the LEDs in the +5V and the shorter leg
         (cathode) in OUT(0-15).
    -  +5V from Arduino -> TLC pin 21 and 19     (VCC and DCPRG)
    -  GND from Arduino -> TLC pin 22 and 27     (GND and VPRG)
    -  digital 3        -> TLC pin 18            (GSCLK)
    -  digital 9        -> TLC pin 24            (XLAT)
    -  digital 10       -> TLC pin 23            (BLANK)
    -  digital 11       -> TLC pin 26            (SIN)
    -  digital 13       -> TLC pin 25            (SCLK)
    -  The 2K resistor between TLC pin 20 and GND will let ~20mA through each
       LED.  To be precise, it's I = 39.06 / R (in ohms).  This doesn't depend
       on the LED driving voltage.
    - (Optional): put a pull-up resistor (~10k) between +5V and BLANK so that
                  all the LEDs will turn off when the Arduino is reset.

    If you are daisy-chaining more than one TLC, connect the SOUT of the first
    TLC to the SIN of the next.  All the other pins should just be connected
    together:
        BLANK on Arduino -> BLANK of TLC1 -> BLANK of TLC2 -> ...
        XLAT on Arduino  -> XLAT of TLC1  -> XLAT of TLC2  -> ...
    The one exception is that each TLC needs it's own resistor between pin 20
    and GND.

    This library uses the PWM output ability of digital pins 3, 9, 10, and 11.
    Do not use analogWrite(...) on these pins.

    This sketch does the Knight Rider strobe across a line of LEDs.

    Alex Leone <acleone ~AT~ gmail.com>, 2009-02-03 */

#include "EEPROM.h"
#include "DMXSerial2.h"
#include "Tlc5940.h"

#define RedDefaultLevel   0x00
#define GreenDefaultLevel 0x00
#define BlueDefaultLevel  0x01

#define FAN_CHANNEL 0x0f


boolean processCommand(struct RDMDATA *rdm);

void setup()
{
  /* Call Tlc.init() to setup the tlc.
     You can optionally pass an initial PWM value (0 - 4095) for all channels.*/
	Tlc.init();
  
	// DMXSerial2.init(DMXReceiver);
  
  DMXSerial2.initRDM(RDMDeviceMode,
    NUM_TLCS * 15, // footprint
    "www.jeremyveleber.com",   // manufacturerLabel
    "Citadel RDM Device", // deviceModel
    processCommand // RDMCallbackFunction
  );

	uint16_t start = DMXSerial2.getStartAddress();
	for (int channel = 0; channel < NUM_TLCS * 15; channel+=3) {
		DMXSerial2.write(start + channel + 0, 30);
		DMXSerial2.write(start + channel + 1, 30);
		DMXSerial2.write(start + channel + 2,  0);
	}
}

/* NUM_TLCS is defined in "tlc_config.h" in the
   library folder.  After editing tlc_config.h for your setup, delete the
   Tlc5940.o file to save the changes. */
int direction = 1;
int val = 0; 
void loop()
{
  
  // Tlc.clear();
  // for (int channel = 0; channel < NUM_TLCS * 16; channel++) {

    /* Tlc.clear() sets all the grayscale values to zero, but does not send
       them to the TLCs.  To actually send the data, call Tlc.update() */
    

    /* Tlc.set(channel (0-15), value (0-4095)) sets the grayscale value for
       one channel (15 is OUT15 on the first TLC, if multiple TLCs are daisy-
       chained, then channel = 16 would be OUT0 of the second TLC, etc.).

       value goes from off (0) to always on (4095).

       Like Tlc.clear(), this function only sets up the data, Tlc.update()
       will send the data. */
//    if (channel == 0) {
//      direction = 1;
//    } else {
//      Tlc.set(channel - 1, 1000);
//    }
//    Tlc.set(channel, 4095);
//    if (channel != NUM_TLCS * 16 - 1) {
//      Tlc.set(channel + 1, 1000);
//    } else {
//      direction = -1;
//    }
    // if(direction == 1) {
      // if(val >= 4095) {
        // direction = 0;
      // } else {
        // val += 10;
      // }   
    // } else {
      // if(val <= 0) {
        // direction = 1;
      // } else {
        // val -= 10;
      // }  
    // } 
    
    // Tlc.set(channel, val);
    
    /* Tlc.update() sends the data to the TLCs.  This is when the LEDs will
       actually change. */
  // }
  
  Tlc.clear();
  // Calculate how long no data backet was received
  unsigned long lastPacket = DMXSerial2.noDataSince();
  if (DMXSerial2.isIdentifyMode()) {
    // RDM Command for Indentification was sent.
    // Blink the device.
    unsigned long now = millis();
    if (now % 1000 < 500) {
      rgbAll(200, 200, 200);
    } else {
      rgbAll(0, 0, 0);
    } // if
    
  } else if (lastPacket < 5000) {
    // read recent DMX values and set pwm levels 
	setLightingFromDmx();
  } else {
    // Show default color, when no data was received since 30 seconds or more.
	setDefaultLighting();
  } // if
  setFan(0x400);
  Tlc.update();
  DMXSerial2.tick();
}

void setDefaultLighting() {
	uint16_t start = DMXSerial2.getStartAddress();
	for (int channel = 0; channel < NUM_TLCS * 15; channel+=3) {
		Tlc.set(channel + 0, dmx2Tlc(RedDefaultLevel));
		Tlc.set(channel + 1, dmx2Tlc(GreenDefaultLevel));
		Tlc.set(channel + 2, dmx2Tlc(BlueDefaultLevel));
	}
	
	// unsigned long now = millis();
    // if (now % 1000 < 500) {
      // rgbAll(200, 200, 200);
    // } else {
      // rgbAll(0, 0, 0);
    // } // if
}

void setLightingFromDmx() {
	for (int channel = 0; channel < NUM_TLCS * 15; channel++) {
		Tlc.set(channel, dmx2Tlc(DMXSerial2.readRelative(channel)));
	}
}

void rgbAll(byte r, byte g, byte b) {
	uint16_t start = DMXSerial2.getStartAddress();
	for (int channel = 0; channel < NUM_TLCS * 15; channel+=3) {
		Tlc.set(channel + 0, dmx2Tlc(r));
		Tlc.set(channel + 1, dmx2Tlc(g));
		Tlc.set(channel + 2, dmx2Tlc(b));
	}
}

void setFan(uint16_t fanLevel) {
	Tlc.set(FAN_CHANNEL,  fanLevel);
}

uint16_t dmx2Tlc(uint8_t dmxIn) {
	/*
		Reversed because we are sinking the base of
		a transitor to turn off a channel
	*/
	return map(dmxIn, 0, 255, 4095, 0);
}

// This function was registered to the DMXSerial2 library in the initRDM call.
// Here device specific RDM Commands are implemented.
boolean processCommand(struct RDMDATA *rdm)
{
  byte CmdClass       = rdm->CmdClass;     // command class
  uint16_t Parameter  = rdm->Parameter;	   // parameter ID
  boolean handled = false;

// This is a sample of how to return some device specific data
  if ((CmdClass == E120_GET_COMMAND) && (Parameter == SWAPINT(E120_DEVICE_HOURS))) { // 0x0400
    rdm->DataLength = 4;
    rdm->Data[0] = 0;
    rdm->Data[1] = 0;
    rdm->Data[2] = 2;
    rdm->Data[3] = 0;
    handled = true;

  } else if ((CmdClass == E120_GET_COMMAND) && (Parameter == SWAPINT(E120_LAMP_HOURS))) { // 0x0400
    rdm->DataLength = 4;
    rdm->Data[0] = 0;
    rdm->Data[1] = 0;
    rdm->Data[2] = 0;
    rdm->Data[3] = 1;
    handled = true;
  } // if
  
  return(handled);
} // _processRDMMessage


