//#include "TM1637Display.h"
#include "SevenSegmentTM1637.h"
#include "SevenSegmentExtended.h"
#include "SevenSegmentFun.h"
#include "FastLED.h"

FASTLED_USING_NAMESPACE

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif


// Pins
//-------------------------------------
#define CLK_PIN 8      // 4-Digit Display Clock Pin
#define DIO_PIN 7      // 4-Digit Display I/O Pin
#define TRIGGER_PIN 9  // UltraSound Triggering Pin
#define ECHO_PIN 10    // UltraSound Data Pin
#define TOUCH_PIN 11   // Touch Sensor
#define LED_PIN 12      // Led Strip

// Constants
//-------------------------------------
#define SERIAL_RATE     115200

// Led Strip
#define NUM_LEDS        29
#define LED_TYPE        WS2812B
#define COLOR_ORDER     GRB

#define LED_BRIGHTNESS  64
#define FRAMES_PER_SECOND  120

//#define DEBUG true

// Variables
//---------------------------------------
int mode = 0;
int heatindex;
long duration;
uint8_t distance;
byte repeats = 2;

// Touch Sensor
boolean currentState = LOW;
boolean lastState = LOW;

const int numReadings = 5;

int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average

const char* modes[] = {"NAZI", "DIST", "RAIN", "GLIT","CONF","SINE","BPM","JGGL"};

// Setup
CRGB leds[NUM_LEDS]; // LED Strip

SevenSegmentFun display(CLK_PIN, DIO_PIN);  // 4 - Digit Display.

// Start
//---------------------------------------
void setup() {
#if defined(DEBUG)
  Serial.begin(9600);
#endif

  delay(3000); // 3 second delay for recovery

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(LED_BRIGHTNESS);

  // UltraSound
  pinMode(TRIGGER_PIN, OUTPUT); // Sets the trigPin as an Output
  pinMode(ECHO_PIN, INPUT); // Sets the echoPin as an Input

  // 4 Digit Display Setup
  display.begin();            // initializes the display
  display.setBacklight(50);  // set the brightness to 100 %
  display.print(modes[mode]);

  // Touch Sensor
  pinMode(TOUCH_PIN, INPUT);

  // Smoothing Distance
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }
}

typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm };

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

void loop() {
  // Led Strip
  FastLED.show();
  FastLED.delay(1000 / FRAMES_PER_SECOND);  // insert a delay to keep the framerate modest
  EVERY_N_MILLISECONDS( 20 ) {
    //Serial.println(gHue);
    gHue++; // slowly cycle the "base color" through the rainbow
  }
  EVERY_N_SECONDS( 10 ) {
    nextPattern();  // change patterns periodically
  }

  checkTouch();

  switch (mode) {
    case 0:
      randomLED();
      break;
    case 1:
      measureDistance();
      // 4 Digit Display Code:
      display.clear();
      display.print(average);
      if (average < 10) {
        display.scrollingText("PLEASE RESPECT MY SPACE", 1);
      }
      break;
    case 2:
      rainbow();
      break;
    case 3:
      rainbowWithGlitter();
      break;
    case 4:
      confetti();
      break;
    case 5:
      sinelon();
      break;
    case 6:
      bpm();
      break;
    case 7:
      juggle();
      break;
  }
}

void checkTouch() {
  // Read Touch Sensor and store into variable
  currentState = digitalRead(TOUCH_PIN);

  if (currentState == HIGH && lastState == LOW) {
    mode = (mode + 1) % 8;
    display.clear();
    display.print(modes[mode]);
#if defined(DEBUG)
    Serial.print("TOUCHED ");
    Serial.println(mode);
#endif
  }

  // Keep if statement from constantly triggering if continuously touching
  lastState = currentState;
}

DEFINE_GRADIENT_PALETTE( heatmap_gp ) {
  40,    255, 0,    0,      // Red
  112,   255, 255,  0,      // Yellow
  200,   0,   255,  0,      // Green
  255,   0,   0,    255
};  // Blue

CRGBPalette16 myPal = heatmap_gp;

void measureDistance() {
  // UltraSound
  // Clears the trigPin
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);

  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(ECHO_PIN, HIGH);

  // Calculating the distance
  distance = duration * 0.034 / 2;

  // Smoothing
  total = total - readings[readIndex];
  readings[readIndex] = distance;
  total = total + readings[readIndex];
  readIndex++;

  if (readIndex >= numReadings) {
    readIndex = 0;
  }

  average = total / numReadings;

#if defined(DEBUG)
  Serial.print("Distance: ");
  Serial.print(distance);
#endif

  heatindex = average;
  if (average > 255) {
    heatindex = 255;
  }

#if defined(DEBUG)
  Serial.print("  Index: ");
  Serial.println(heatindex);
#endif

  fill_solid( leds, NUM_LEDS, ColorFromPalette(myPal, heatindex));
  FastLED.show();
}

//////////////////////////
// LED Strip Functions: //
//////////////////////////
void randomLED() {
  gPatterns[gCurrentPatternNumber]();
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

void slowfade() {
  // Turn the LED on, then pause
  static uint8_t hue = 0;
  FastLED.showColor(CHSV(hue++, 255, 255));

}
void rainbow() {
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter() {
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) {
  if ( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti() {
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon() {
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS - 1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm() {
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 128;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for ( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for ( int i = 0; i < 8; i++) {
    leds[beatsin16( i + 7, 0, NUM_LEDS - 1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}
