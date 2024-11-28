/*
A Simple midicontroller based on beaatrmatrix mk2 hardware, 
see https://github.com/yobalint/beatrmatrix-mk2-arduino-based-ableton-controller

This version sends out serial midi CC codes (and receives them). On Mac or PC run A.V Perotta's serialToMidi app to
convert serialmidi to IAC driver midi that can be handled by any DAW.

On startup the module needs to be allocated to a midi channel. The controller will flash led 1 twice on startup. 
Press the desired channel to lock to that channel (to do make the choce persistent).

The pots operate CC 120, 12, 122 [0..127]
the mode, length, launch buttons send 127, followed by 0 on release on CC 116, 117, 118  – t u v 
The 16 switches produce 127 on first press, 0 on second (toggle). A green led designates the state of the toggle.

On PC or Mac run the app available on https://github.com/avperrotta/serialToMidi
Also (on Mac) create an IAC driver instance in 'Audio/midi configuratie'. Open the 'Midi-studio' window, open the IAC driver
Create one or more ports for the interface (calling them, for instance, serialmidi) and assigning 1 or more input/ouptut ports.

Once you have the IAC driver configured, you can run the serialToMidi app. Select a serialport (on a Mac it will be called usbserial-xxx or wchusbserial-xxx) and baudrate 115200. 
Select IAC Driver serialmidi on the midid device and you will be good to go.

version 0.9
28 Nov 2024

compile for Arduino nano. use processor Atmega328p (old bootloader) 

*/


#include <Adafruit_NeoPixel.h>
#include <Bounce2.h>
#include <ardumidi.h>


Better yet. Use this

Comes as an app. Tested on MacosX Sequioa
*/


int channel = 15;

#define PIN            12  // Pin to which WS2812 LEDs are connected
#define NUM_LEDS       16  // Number of WS2812 LEDs

const int ROWS = 5;
const int COLS = 4;
char keys[ROWS][COLS] = {
  {'d', 'e', 'f', 'g'},
  {'h', 'i', 'j', 'k'},
  {'l', 'm', 'n', 'o'},
  {'p', 'q', 'r', 's'},
  {'t', 'u', 'v', 'w'}
};
byte state[ROWS][COLS] =  {
  {0,0,0,0},
  {0,0,0,0},
  {0,0,0,0},
  {0,0,0,0},
  {0,0,0,0}};

#define CONFIG 1
#define RUN 2

int rowPins[ROWS] = {2, 3, 4, 5, 10};
int colPins[COLS] = {6, 7, 8, 9};
int deviceState = CONFIG;

Bounce debouncers[ROWS][COLS];

// Define the analog input pins for the potentiometers
const int potPin1 = A6;
const int potPin2 = A5;
const int potPin3 = A7;

int prevPotValue1 = 0;
int prevPotValue2 = 0;
int prevPotValue3 = 0;

const int hysteresis = 2;  // Adjust the hysteresis value as needed

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);
int ledIndex = 0;
int color = 0;

void controlChange(unsigned char channel, unsigned char control, unsigned char value) {
  Serial.write(0xB0 | channel);
  Serial.write(control);
  Serial.write(value);
}


void setup() {
  Serial.begin(115200);
  strip.begin();
    strip.show();  // Initialize all pixels to 'off'
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)
  
  blinkPixel1();
  for (int r = 0; r < ROWS; r++) {
    for (int c = 0; c < COLS; c++) {
      pinMode(colPins[c], OUTPUT);
      digitalWrite(colPins[c], HIGH);
      pinMode(rowPins[r], INPUT_PULLUP);
      debouncers[r][c].attach(rowPins[r]);
      debouncers[r][c].interval(10);
    }
  }
  deviceState = CONFIG;
  readKeys(); //determine channel for MIDI to comunnicate 
 // channel=15;
}

void blinkPixel1(){
  strip.setPixelColor(0, 50, 100, 250);
  strip.show();
    delay(500);

  strip.setPixelColor(0, 0, 0, 0);
  strip.show();
delay(1000);
  strip.setPixelColor(0, 50, 100, 250);
  strip.show();  // Initialize all pixels to 'off'
    delay(500);

  strip.clear();
  strip.show();
}

void doPots(){
  // Read potentiometer values
  int potValue1 = map(analogRead(potPin1), 0, 1023, 127, 0);
  int potValue2 = map(analogRead(potPin2), 0, 1023, 127, 0);
  int potValue3 = map(analogRead(potPin3), 0, 1023, 127, 0);
  // Apply hysteresis to potentiometer values
  if (abs(potValue1 - prevPotValue1) > hysteresis) {
    controlChange(channel, 'x', potValue1);
    prevPotValue1 = potValue1;
    }
  if (abs(potValue2 - prevPotValue2) > hysteresis) {
    controlChange(channel, 'y', potValue2);
    prevPotValue2 = potValue2;
    }
    if (abs(potValue3 - prevPotValue3) > hysteresis) {
    controlChange(channel, 'z', potValue3);
    prevPotValue3 = potValue3;
    }
}

void readKeys(){
  for (int c = 0; c < COLS; c++) {
    digitalWrite(colPins[c], LOW);
    for (int r = 0; r < ROWS; r++) {
      debouncers[r][c].update();
      if (debouncers[r][c].fell()) {
       (deviceState == CONFIG)? doChannel(r, c): doKeys(r, c);
        delay(200);
      }
    }
    digitalWrite(colPins[c], HIGH);
  } 
}

void doChannel(int r, int c){
  channel = keys[r][c] - 99;
  Serial.print("channel ");
  Serial.println( channel );
    strip.setPixelColor(channel - 1, 100, 0, 250);
  strip.show();  // Initialize all pixels to 'off'
    delay(3000);
  strip.clear();
  strip.show();
  deviceState = RUN;
}

void doKeys(int r, int c){
  if (keys[r][c] == 't' || keys[r][c] == 'u' || keys[r][c] == 'v'){
    delay(50);
    controlChange(channel, keys[r][c], 0x00);
    return;
  }

  if (state[r][c] == 0){
    //midi_controller_change(channel, keys[r][c], 0x7f);
          controlChange(channel, keys[r][c], 0x7f);
          state[r][c]= 1;
          strip.setPixelColor(keys[r][c] - 100, Wheel(65));
        } else {
          controlChange(channel, keys[r][c], 0x00);
          state[r][c]= 0;
          strip.setPixelColor(keys[r][c] - 100, 0,0,0);
        }
        strip.show();
  return;
}
void loop() {
  doPots();
  readKeys();


  while (midi_message_available() > 0) {
		MidiMessage m = read_midi_message();
		if (m.command == MIDI_CONTROLLER_CHANGE) {
      if (m.channel == 1){
        ledIndex = m.param1;
        color = m.param2 * 2;
          strip.setPixelColor(ledIndex, Wheel(color));
          strip.show();
      }
	  }
    if (m.command == MIDI_NOTE_ON) {
    ledIndex = m.param1 - 48;
        color = m.param2 * 2;
          strip.setPixelColor(ledIndex, Wheel(color));
          strip.show();
      }
  }
}
// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow(int wait) {
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this loop:
  for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
    // strip.rainbow() can take a single argument (first pixel hue) or
    // optionally a few extras: number of rainbow repetitions (default 1),
    // saturation and value (brightness) (both 0-255, similar to the
    // ColorHSV() function, default 255), and a true/false flag for whether
    // to apply gamma correction to provide 'truer' colors (default true).
    strip.rainbow(firstPixelHue);
    // Above line is equivalent to:
    // strip.rainbow(firstPixelHue, 1, 255, 255, true);
    strip.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}

// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}