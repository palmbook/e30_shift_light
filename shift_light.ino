#include <Wire.h>
#include <FastLED.h>
#define NUM_LEDS 24 // LED strip/No of LEDs
#define DATA_PIN 6 // Signal pin for neopixel
#define BRIGHTNESS 48 // Brightness 0-255

#define MAInterval 5 // Moving Average Interval

#define MAX_RPM 8000 // Max RPM for your tachometer
#define MIN_RPM 500 // Min RPM, should be slightly lower than idle RPM
#define SHIFT_RPM 6500 // RPM at which the last light should be lit up
#define MULT 30 // Multiplier to convert tach frequency to RPM

const int pulsePin = A0; // Tach signal pin

unsigned long pulseHigh; // Integer variable to capture High time of the incoming pulse
unsigned long pulseLow; // Integer variable to capture Low time of the incoming pulse
unsigned long pulseTotal; // Float variable to capture Total time of the incoming pulse
unsigned int frequency = 0; // Calculated Frequency

unsigned int FreqReadings[MAInterval]; // Array to keep the last n frequency readings for moving average
unsigned int pointer = 0; // Pointer to point where next reading must be inserted

CRGB leds[NUM_LEDS];

void setup() {

  // Initialize array
  for (int i = 0; i < MAInterval; i++) {
    FreqReadings[i] = 0;
  }

  pinMode(pulsePin, INPUT); // Set up input pin

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS); 
  FastLED.setBrightness(BRIGHTNESS );

  Serial.begin(250000);
}

void display(int rpm){
  float lit_fraction = ((float)rpm - MIN_RPM) / SHIFT_RPM;

  for (int i = 0; i < NUM_LEDS / 2; i++) {
    float led_pos = (float)i / (NUM_LEDS / 2);
    if (led_pos < lit_fraction) {
      if (led_pos <= 0.5) {
        leds[i] = CRGB::Green;
        leds[NUM_LEDS - i - 1] = CRGB::Green;
      } else if (led_pos < 1 - (2.00 / (NUM_LEDS / 2))) {
        leds[i] = CRGB::DarkOrange;
        leds[NUM_LEDS - i - 1] = CRGB::DarkOrange;
      } else {
        leds[i] = CRGB::Red;
        leds[NUM_LEDS - i - 1] = CRGB::Red;
      }
    } else {
      leds[i] = CRGB::Black;
      leds[NUM_LEDS - i - 1] = CRGB::Black;
    }
  }

  FastLED.show();
}

void blink() {
  for (int i = 0; i < NUM_LEDS / 2; i++) {
    float led_pos = (float)i / (NUM_LEDS / 2);
    
    if (led_pos <= 0.5) {
      leds[i] = CRGB::Green;
      leds[NUM_LEDS - i - 1] = CRGB::Green;
    } else if (led_pos < 1 - (2.00 / (NUM_LEDS / 2))) {
      leds[i] = CRGB::DarkOrange;
      leds[NUM_LEDS - i - 1] = CRGB::DarkOrange;
    } else {
      leds[i] = CRGB::Red;
      leds[NUM_LEDS - i - 1] = CRGB::Red;
    }
  }
  FastLED.show();

  delay(100);

  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();

  delay(100);
}

void loop() {
  pulseHigh = pulseIn(pulsePin, HIGH); // time from Low to High
  pulseLow = pulseIn(pulsePin, LOW); // time from High to Low

  pulseTotal = pulseHigh + pulseLow; // Time period of the pulse in microseconds

  // Check if pulseTotal is 0
  // Protect against division by zero
  if (pulseTotal > 0) {
    frequency = 1000000 / pulseTotal; // Frequency in Hertz (Hz)
  } else {
    frequency = 0; 
  }

  FreqReadings[pointer] = frequency; // Insert the latest frequency
  pointer = (pointer + 1) % MAInterval; // Increment pointer and loop around if it is the end of array

  // Tally up the array and divide to get moving average
  unsigned long total = 0;
  for (int i = 0; i < MAInterval; i++) {
    total = total + FreqReadings[i];
  }
  unsigned long AvgFreq = total / MAInterval;
  int rpm = max(min((AvgFreq * MULT), MAX_RPM), MIN_RPM);

  if (rpm >= SHIFT_RPM) {
    blink();
  } else {
    display(rpm);
  }

  Serial.println(rpm);
}
