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
gemma_dual_neopixel_rings.ino : v1.2 - refactor c0.0.0

*/

#include <Adafruit_NeoPixel.h>
#include <Adafruit_FreeTouch.h>

#define NEOPIXEL_PIN 1
#define NUM_LEDS 28
#define CAPTOUCH_PIN 0

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, NEOPIXEL_PIN, NEO_GRBW + NEO_KHZ800);
Adafruit_FreeTouch qt_1 = Adafruit_FreeTouch(CAPTOUCH_PIN, OVERSAMPLE_4, RESISTOR_50K, FREQ_MODE_NONE);

int brightness = 12; // Brightness of the LEDs (0-255)
int touchThreshold = 400; // Threshold for touch sensitivity
unsigned long blinkInterval = 500; // Interval for blinking effect only (in milliseconds)
unsigned long rotationInterval = 300; // Speed of normal color rotation for smoother effect. Higher for slower
unsigned long rainbowUpdateInterval = 5; // Interval for rainbow rotation updates, adjust for smoother effect (lower = smoother)
//unsigned long rainbowRotationSpeed = 500; // Speed of rainbow rotation when touched (may be repurposed or adjusted as needed)
uint32_t warmWhiteIntensity = 100; // Set this to the desired brightness for the warm white
int brightness12LED = 200; // Brightness for the 12-LED ring's spinning trail (0-255)
int brightness16LED = 100; // Brightness for the 16-LED ring's spinning trail (0-255)


// Color variables for each ring so we can set them separately
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
  ring1Colors[0] = strip.Color(255, 0, 0); // 16-led : Red
  ring1Colors[1] = strip.Color(0, 255, 0); // 16-led : Green
  ring1Colors[2] = strip.Color(0, 0, 255); // 16-led : Blue
  
  ring2Colors[0] = strip.Color(255, 255, 0); // 12-led : Yellow
  ring2Colors[1] = strip.Color(0, 255, 255); // 12-led : Cyan
  ring2Colors[2] = strip.Color(255, 0, 255); // 12-led : Magenta
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

        // Update colors for the first ring (0-15, 16 LEDs)
        for (int i = 0; i < 16; i++) {
            int colorChoice = (i + colorIndex1) % 3; // Cycle through 0, 1, 2
            strip.setPixelColor(i, ring1Colors[colorChoice]);
        }

        // Update colors for the second ring (16-27, 12 LEDs)
        for (int i = 16; i < NUM_LEDS; i++) {
            int colorChoice = (i + colorIndex2) % 3; // Cycle through 0, 1, 2
            strip.setPixelColor(i, ring2Colors[colorChoice]);
        }

        // Increment color indexes for the next iteration
        colorIndex1 = (colorIndex1 + 1) % 3;
        colorIndex2 = (colorIndex2 + 1) % 3;
    }
}


void rainbowWheel(unsigned long currentMillis) {
    static unsigned long lastUpdate = 0;
    static int hueShift = 0; // Controls the starting hue of the first LED in the strip

    if (currentMillis - lastUpdate > rainbowUpdateInterval) {
        lastUpdate = currentMillis;

        // Incrementally shift the starting hue for the strip to create the rotation effect
        // Increasing this value will slow down the speed of the rotation
        int speedFactor = 1; 
        hueShift += speedFactor;

        int totalHueSpread = 256; // Represents the total range of hues
        int hueIncrement = totalHueSpread / strip.numPixels(); // Distribute the hues evenly across the strip

        for(int i = 0; i < strip.numPixels(); i++) {
            // Calculate the hue for each LED, ensuring a smooth transition across the strip
            // The modulo operation ensures that the hue value wraps correctly
            int hue = (hueShift + i * hueIncrement) % totalHueSpread;
            strip.setPixelColor(i, Wheel(hue));
        }
    }
}

void blinkWhiteRing(unsigned long currentMillis) {
    static bool isBlinkOn = false;
    static unsigned long lastBlinkTime = 0;
    if (currentMillis - lastBlinkTime > blinkInterval) {
        isBlinkOn = !isBlinkOn; // Toggle the color state
        lastBlinkTime = currentMillis;
    }

    for(int i = 16; i < NUM_LEDS; i++) { // Apply warm white color to the next 12 LEDs (12-pixel ring)
        // Here we make sure to only use the warm white part of the RGBW LED
        strip.setPixelColor(i, isBlinkOn ? strip.Color(0, 0, 0, warmWhiteIntensity) : strip.Color(0, 0, 0, 0));
    }
    // Note: strip.show() is called in the loop() function to apply the updates
}

void spinningTrailAnimation(unsigned long currentMillis) {
    static unsigned long lastUpdate = 0;
    static int leadLED = 27;

    unsigned long spinInterval = 100;

    if (currentMillis - lastUpdate > spinInterval) {
        lastUpdate = currentMillis;

        for (int i = 16; i < NUM_LEDS; i++) {
            strip.setPixelColor(i, strip.Color(0, 0, 0, 0));
        }

        strip.setPixelColor(leadLED, strip.Color(0, 0, 0, brightness12LED));

        for (int trail = 1; trail <= 4; trail++) {
            int trailLED = leadLED + trail;
            if (trailLED >= NUM_LEDS) trailLED -= 12;
            int brightness = max(brightness12LED - (trail * (brightness12LED / 5)), 0);
            strip.setPixelColor(trailLED, strip.Color(0, 0, 0, brightness));
        }

        leadLED -= 1;
        if (leadLED < 16) leadLED = 27;
    }
}



void spinningTrailAnimation16LED(unsigned long currentMillis) {
    static unsigned long lastUpdate = 0;
    static int leadLED = 0;

    unsigned long spinInterval = 100;

    if (currentMillis - lastUpdate > spinInterval) {
        lastUpdate = currentMillis;

        for (int i = 0; i < 16; i++) {
            strip.setPixelColor(i, strip.Color(0, 0, 0, 0));
        }

        strip.setPixelColor(leadLED, strip.Color(0, 0, 0, brightness16LED));

        for (int trail = 1; trail <= 4; trail++) {
            int trailLED = leadLED - trail;
            if (trailLED < 0) trailLED += 16;
            int brightness = max(brightness16LED - (trail * (brightness16LED / 5)), 0);
            strip.setPixelColor(trailLED, strip.Color(0, 0, 0, brightness));
        }

        leadLED = (leadLED - 1 + 16) % 16;
    }
}




void specialAnimation(unsigned long currentMillis) {
  static unsigned long lastUpdate = 0;
  if (currentMillis - lastUpdate > 100) { // Adjust as necessary for your animation speed
    lastUpdate = currentMillis;
    static bool growing = true;
    static int brightnessLevel = 0;
    uint32_t color = strip.Color(brightnessLevel, brightnessLevel, brightnessLevel, 0); // Pulsing white (RGB)
    if (growing) {
      brightnessLevel += 5;
      if (brightnessLevel >= 255) growing = false;
    } else {
      brightnessLevel -= 5;
      if (brightnessLevel <= 0) growing = true;
    }
    for (int i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, color);
    }
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
