#include <Adafruit_NeoPixel.h>
#include <Adafruit_FreeTouch.h>

#define NEOPIXEL_PIN 1    // Neopixel ring pin
#define NUM_LEDS    28    // Total number of pixels
#define CAPTOUCH_PIN 0    // Capacitive touch pin

Adafruit_NeoPixel strip(NUM_LEDS, NEOPIXEL_PIN, NEO_GRBW + NEO_KHZ800);
Adafruit_FreeTouch qt_1 = Adafruit_FreeTouch(CAPTOUCH_PIN, OVERSAMPLE_4, RESISTOR_50K, FREQ_MODE_NONE);

int brightness = 12;
int touchThreshold = 400;
unsigned long blinkInterval = 500;
unsigned long lastChangeTime = 0;
int animationState = 0;

void setup() {
  strip.begin();
  strip.setBrightness(brightness);
  strip.show(); // Initialize all pixels to 'off'
  
  if (!qt_1.begin()) {
    Serial.println("Failed to begin touch on pin D0");
  }
}

void loop() {
  unsigned long currentMillis = millis();

  if (qt_1.measure() > touchThreshold && (currentMillis - lastChangeTime > 1000)) {
    animationState = (animationState + 1) % 3;
    lastChangeTime = currentMillis;
  }

  strip.clear();

  switch (animationState) {
    case 0:
      colorWheelRotation();
      break;
    case 1:
      rainbowWheel();
      blinkWhiteRing(currentMillis);
      break;
    case 2:
      specialAnimation(currentMillis);
      break;
  }

  strip.show();
}

void colorWheelRotation() {
  static int colorIndex = 0;
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + colorIndex) & 255));
  }
  colorIndex = (colorIndex + 1) % 256; // Increment and wrap at 255
}

void rainbowWheel() {
  static int rainbowIndex = 0;
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + rainbowIndex) & 255));
  }
  rainbowIndex = (rainbowIndex + 1) % 256; // Increment and wrap at 255
}

void blinkWhiteRing(unsigned long currentMillis) {
  static bool isBlinkOn = false;
  static unsigned long lastBlinkTime = 0;
  if (currentMillis - lastBlinkTime > blinkInterval) {
    isBlinkOn = !isBlinkOn;
    lastBlinkTime = currentMillis;
  }

  for (int i = 16; i < NUM_LEDS; i++) { // Assuming the 12-pixel ring starts at index 16
    strip.setPixelColor(i, isBlinkOn ? strip.Color(0, 0, 0, 255) : 0); // Using white LED for blinking
  }
}

void specialAnimation(unsigned long currentMillis) {
  static bool growing = true;
  static int brightnessLevel = 0;
  uint32_t color = strip.Color(brightnessLevel, brightnessLevel, brightnessLevel); // Pulsing white (RGB)

  if (currentMillis - lastChangeTime > 20) { // Adjust the pulse speed
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
  if (WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, WheelPos * 3, 0);
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
    WheelPos -= 170;
    return strip.Color(WheelPos * 3, 0, 255 - WheelPos * 3);
  }
}
