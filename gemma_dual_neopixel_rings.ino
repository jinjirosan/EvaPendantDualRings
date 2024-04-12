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
                                         bling-bling voor m'n guppie                               

hardware platform  : Gemma M0 (ATSAMD21x18)
                   : 12-led NeoPixel RGBW ring - WS2812B LED's (ring 2)
                   : 16-led NeoPixel RGBW ring - WS2812B LED's (ring 1)

Power              : 3.7v 150mAh LiPo
codebase           : C

(2024) JinjiroSan
gemma_dual_neopixel_rings.ino : v1.3 - refactor c0.0.1

*/

#include <Adafruit_NeoPixel.h>
#include <Adafruit_FreeTouch.h>

#define NEOPIXEL_PIN 1 // Neopixel data connection on Gemma
#define NUM_LEDS 28 // total number of LEDs
#define CAPTOUCH_PIN 0 // Capacitive wire connection on Gemma
#define START_LED_RING1 0 // first LED the total ring strip
#define END_LED_RING1 15 // final LED on 16-led ring
#define START_LED_RING2 16 // first LED on 12-led ring
#define END_LED_RING2 27 // final LED on 12-led ring and led strip
#define NUM_LEDS_RING1 16  // Total LEDs in the first ring
#define NUM_LEDS_RING2 12  // Total LEDs in the second ring

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, NEOPIXEL_PIN, NEO_GRBW + NEO_KHZ800);
Adafruit_FreeTouch qt_1 = Adafruit_FreeTouch(CAPTOUCH_PIN, OVERSAMPLE_4, RESISTOR_50K, FREQ_MODE_NONE);

// Control settings
int brightness = 12; // Brightness global upper limit of all LEDs (0-255)
int touchThreshold = 400; // Threshold for touch sensitivity
unsigned long blinkInterval = 500; // Interval for blinking effect only (in milliseconds)
unsigned long rotationInterval = 300; // Speed of normal color rotation for colorwheelrotation animation 1 (lower is faster)
unsigned long rainbowUpdateInterval = 5; // Interval for rainbow rotation updates
uint32_t warmWhiteIntensity = 100; // Separate brightness for warm white blinking animation
int brightness12LED = 200; // Separate brightness for the 12-LED ring's spinning trail animation
int brightness16LED = 100; // Separate brightness for the 16-LED ring's spinning trail animation

// Color variables for each ring
uint32_t ring1Colors[3]; // 16-led
uint32_t ring2Colors[3]; // 12-led

// State variables
int animationState = 0; // Tracks the current animation state
unsigned long lastTouchTime = 0; // Tracks the last touch time for debouncing

void setup() {
  strip.begin();
  strip.setBrightness(brightness);
  strip.show(); // Initialize all pixels to 'off'

  if (!qt_1.begin())  
    Serial.println("Failed to begin touch on pin D0");

  // Initialize color variables for both rings using RGB values
  ring1Colors[0] = strip.Color(255, 0, 0); // Red
  ring1Colors[1] = strip.Color(0, 255, 0); // Green
  ring1Colors[2] = strip.Color(0, 0, 255); // Blue
  
  ring2Colors[0] = strip.Color(255, 255, 0); // Yellow
  ring2Colors[1] = strip.Color(0, 255, 255); // Cyan
  ring2Colors[2] = strip.Color(255, 0, 255); // Magenta
}

void loop() {
  unsigned long currentMillis = millis();

  // Touch detection with debouncing
  if (qt_1.measure() > touchThreshold && (currentMillis - lastTouchTime > 1000)) {
    animationState = (animationState + 1) % 3; // Cycle through animations
    lastTouchTime = currentMillis;
  }

  // Execute the current animation
  switch (animationState) {
    case 0:
      colorWheelRotation(currentMillis);
      break;
    case 1:
      rainbowWheel(currentMillis);
      blinkWhiteRing(currentMillis);
      break;
    case 2:
      spinningTrailAnimation(currentMillis);
      spinningTrailAnimation16LED(currentMillis);
      break;
  }
  strip.show();
}

void colorWheelRotation(unsigned long currentMillis) {
    static unsigned long lastUpdate = 0;
    if (currentMillis - lastUpdate > rotationInterval) {
        lastUpdate = currentMillis;
        static int colorIndex1 = 0; // Index for ring 1 color rotation
        static int colorIndex2 = 0; // Index for ring 2 color rotation

        for (int i = START_LED_RING1; i <= END_LED_RING1; i++) {
            int colorChoice = (i - START_LED_RING1 + colorIndex1) % 3;
            strip.setPixelColor(i, ring1Colors[colorChoice]);
        }

        for (int i = START_LED_RING2; i <= END_LED_RING2; i++) {
            int colorChoice = (i - START_LED_RING2 + colorIndex2) % 3;
            strip.setPixelColor(i, ring2Colors[colorChoice]);
        }

        colorIndex1 = (colorIndex1 + 1) % 3;
        colorIndex2 = (colorIndex2 + 1) % 3;
    }
}

void rainbowWheel(unsigned long currentMillis) {
    static unsigned long lastUpdate = 0;
    static int hueShift = 0; // Controls the starting hue of the first LED in the strip

    if (currentMillis - lastUpdate > rainbowUpdateInterval) {
        lastUpdate = currentMillis;
        int speedFactor = 1; 
        hueShift += speedFactor;

        for(int i = 0; i < strip.numPixels(); i++) {
            int hue = (hueShift + i * (256 / strip.numPixels())) % 256;
            strip.setPixelColor(i, Wheel(hue));
        }
    }
}

void blinkWhiteRing(unsigned long currentMillis) {
    static bool isBlinkOn = false;
    static unsigned long lastBlinkTime = 0;
    if (currentMillis - lastBlinkTime > blinkInterval) {
        isBlinkOn = !isBlinkOn;
        lastBlinkTime = currentMillis;
    }

    for(int i = START_LED_RING2; i <= END_LED_RING2; i++) {
        strip.setPixelColor(i, isBlinkOn ? strip.Color(0, 0, 0, warmWhiteIntensity) : strip.Color(0, 0, 0, 0));
    }
}

void spinningTrailAnimation(unsigned long currentMillis) {
    static unsigned long lastUpdate = 0;
    static int leadLED = END_LED_RING2;

    if (currentMillis - lastUpdate > 100) {
        lastUpdate = currentMillis;

        for (int i = START_LED_RING2; i <= END_LED_RING2; i++) {
            strip.setPixelColor(i, strip.Color(0, 0, 0, 0));
        }

        strip.setPixelColor(leadLED, strip.Color(0, 0, 0, brightness12LED));

        for (int trail = 1; trail <= 4; trail++) {
            int trailLED = leadLED - trail;
            if (trailLED < START_LED_RING2) trailLED += NUM_LEDS_RING2;
            int brightness = max(brightness12LED - (trail * (brightness12LED / 5)), 0);
            strip.setPixelColor(trailLED, strip.Color(0, 0, 0, brightness));
        }

        leadLED--;
        if (leadLED < START_LED_RING2) leadLED = END_LED_RING2;
    }
}

void spinningTrailAnimation16LED(unsigned long currentMillis) {
    static unsigned long lastUpdate = 0;
    static int leadLED = START_LED_RING1; // Start from the beginning of the 16-LED segment for clockwise motion

    unsigned long spinInterval = 100; // Adjust as needed for desired spin speed

    if (currentMillis - lastUpdate > spinInterval) {
        lastUpdate = currentMillis;

        // Clear the 16-LED ring part of the strip to reset the trail
        for (int i = START_LED_RING1; i <= END_LED_RING1; i++) {
            strip.setPixelColor(i, strip.Color(0, 0, 0, 0)); // Turn off LEDs
        }

        // Set the leading white LED
        strip.setPixelColor(leadLED, strip.Color(0, 0, 0, brightness16LED)); // Full brightness for the leader

        // Create the trailing LEDs with decaying brightness
        for (int trail = 1; trail <= 4; trail++) {
            int trailLED = leadLED - trail;
            if (trailLED < START_LED_RING1) trailLED += NUM_LEDS_RING1; // Wrap around the start of the ring if necessary
            int brightness = max(brightness16LED - (trail * (brightness16LED / 5)), 0);
            strip.setPixelColor(trailLED, strip.Color(0, 0, 0, brightness));
        }

        // Move to the next LED for the next update, wrapping around the ring
        leadLED = (leadLED - 1 + NUM_LEDS_RING1) % NUM_LEDS_RING1;
        if (leadLED < START_LED_RING1) leadLED = END_LED_RING1; // Ensure the leader stays within the 16-pixel ring
    }
}

uint32_t Wheel(byte WheelPos) {
    WheelPos = 255 - WheelPos;
    if(WheelPos < 85) {
        return strip.Color(255 - WheelPos * 3, WheelPos * 3, 0);
    } else if (WheelPos < 170) {
        WheelPos -= 85;
        return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    } else {
        WheelPos -= 170;
        return strip.Color(WheelPos * 3, 0, 255 - WheelPos * 3);
    }
}
