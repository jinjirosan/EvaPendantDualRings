/*
Project Pendant for Eva                                                                           

.-------.     .-''-.  ,---.   .--. ______        ____    ,---.   .--.,---------.  
\  _(`)_ \  .'_ _   \ |    \  |  ||    _ `''.  .'  __ `. |    \  |  |\          \ 
| (_ o._)| / ( ` )   '|  ,  \ |  || _ | ) _  \/   '  \  \|  ,  \ |  | `--.  ,---' 
|  (_,_) /. (_ o _)  ||  |\_ \|  ||( ''_'  ) ||___|  /  ||  |\_ \|  |    |   \    
|   '-.-' |  (_,_)___||  _( )_\  || . (_) `. |   _.-`   ||  _( )_\  |    :_ _:    
|   |     '  \   .---.| (_ o _)  ||(_    ._) '.'   _    || (_ o _)  |    (_I_)    
|   |      \  `-'    /|  (_,_)\  ||  (_.\.' / |  _( )_  ||  (_,_)\  |   (_(=)_)   
/   )       \       / |  |    |  ||       .'  \ (_ o _) /|  |    |  |    (_I_)    
`---'        `'-..-'  '--'    '--''-----'`     '.(_,_).' '--'    '--'    '---'    
                                                                                
hardware platform  : Gemma M0
                   : 12-led NeoPixel RGBW ring - WS2812B LED's (ring 2)
                   : 16-led NeoPixel RGBW ring - WS2812B LED's (ring 1)

Power              : 3.7v 150mAh LiPo
codebase           : C

(2024) JinjiroSan
gemma_dual_neopixel_rings.ino : v1.1 - Eva's Pendant - refactor c0.0.3

*/

#include <Adafruit_NeoPixel.h>
#include <Adafruit_FreeTouch.h>

#define NEOPIXEL_PIN 1    // Neopixel ring pin
#define NUM_LEDS    28    // Total number of pixels (both rings)
#define CAPTOUCH_PIN 0    // Capacitive touch pin

Adafruit_NeoPixel strip(NUM_LEDS, NEOPIXEL_PIN, NEO_GRBW + NEO_KHZ800);
Adafruit_FreeTouch qt_1 = Adafruit_FreeTouch(CAPTOUCH_PIN, OVERSAMPLE_4, RESISTOR_50K, FREQ_MODE_NONE);

int brightness = 12;            // Brightness of the LEDs (0-255)
int touchThreshold = 400;       // Threshold for touch sensitivity
unsigned long blinkInterval = 500; // Interval for blinking effect only (in milliseconds)
unsigned long rotationInterval = 400; // Speed of normal color rotation for smoother effect
unsigned long rainbowRotationSpeed = 20; // Speed of rainbow rotation when touched
uint32_t blinkColor = strip.Color(255, 255, 255); // Color for blinking, set to warm white so the W of RGBW is used
uint32_t warmWhiteIntensity = 100; // Set this to the desired brightness for the warm white

// State variables
bool touchedState = false;     // false: Normal state, true: Touched state
unsigned long lastChangeTime = 0;
unsigned long lastRotationTime = 0; // Tracks the last rotation time for smooth animation
unsigned long lastRainbowRotationTime = 0; // Tracks the last rainbow rotation time

// Color variables for each ring so we can set them separately
uint32_t ring1Colors[3]; // 16-led
uint32_t ring2Colors[3]; // 12-led

void setup() {
  strip.begin();
  strip.setBrightness(brightness);
  strip.show(); // Initialize all pixels to 'off'

  if (!qt_1.begin())  
    Serial.println("Failed to begin touch on pin D0");

  // Initialize color variables for both rings using RGB values
  ring1Colors[0] = strip.Color(255, 0, 0); // 16-led : Red
  ring1Colors[1] = strip.Color(0, 255, 0); // 16-led : Green
  ring1Colors[2] = strip.Color(0, 0, 255); // 16-led : Blue
  
  ring2Colors[0] = strip.Color(255, 255, 0); // 12-led : Yellow
  ring2Colors[1] = strip.Color(0, 255, 255); // 12-led : Cyan
  ring2Colors[2] = strip.Color(255, 0, 255); // 12-led : Magenta
}

void loop() {
  unsigned long currentMillis = millis();

  // Touch detection with debouncing of 1 second
  if (qt_1.measure() > touchThreshold && (currentMillis - lastChangeTime > 1000)) {
    touchedState = !touchedState; // Toggle state
    lastChangeTime = currentMillis;
  }

  // Update animations based on the state
  if (touchedState) {
    if (currentMillis - lastRainbowRotationTime > rainbowRotationSpeed) {
      rainbowWheel();
      lastRainbowRotationTime = currentMillis;
    }
    blinkWhiteRing(currentMillis); // Updated to allow blinking alongside the rainbow
  } else {
    if (currentMillis - lastRotationTime > rotationInterval) {
      colorWheelRotation();
      lastRotationTime = currentMillis;
    }
  }
  strip.show(); // Update LEDs
}

void rainbowWheel() {
  static int rainbowIndex = 0;
  rainbowIndex++; // Increment the starting point for the rainbow to rotate
  for(int i = 0; i < 16; i++) { // Apply rainbow to the first 16 LEDs (16-pixel ring)
    strip.setPixelColor(i, Wheel((i + rainbowIndex) & 255));
  }
  // Note: strip.show() is called in the loop to synchronize updates
}

void blinkWhiteRing(unsigned long currentMillis) {
    static bool isWhite = false;
    static unsigned long lastBlinkTime = 0;
    if (currentMillis - lastBlinkTime > blinkInterval) {
        isWhite = !isWhite; // Toggle the color state
        lastBlinkTime = currentMillis;
    }

    for(int i = 16; i < NUM_LEDS; i++) { // Apply warm white color to the next 12 LEDs (12-pixel ring)
        // Use the warmWhiteIntensity variable for the white component
        strip.setPixelColor(i, isWhite ? strip.Color(0, 0, 0, warmWhiteIntensity) : 0);
    }
    // strip.show() is called in the loop to synchronize the updates
}

void colorWheelRotation() {
  static int colorIndex = 0;
  colorIndex++; // Increment to rotate the color wheel
  for(int i = 0; i < 16; i++) { // Apply 16-led ring colors
    strip.setPixelColor(i, ring1Colors[(i + colorIndex) % 3]);
  }
  
  for(int i = 16; i < NUM_LEDS; i++) { // Apply 12-led ring colors
    strip.setPixelColor(i, ring2Colors[(i + colorIndex) % 3]);
  }
  // Note: strip.show() is called in the loop after this function to apply the updates
}

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, WheelPos * 3, 0);
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, 255 - WheelPos * 3, WheelPos * 3);
  } else {
    WheelPos -= 170;
    return strip.Color(WheelPos * 3, 0, 255 - WheelPos * 3);
  }
}
