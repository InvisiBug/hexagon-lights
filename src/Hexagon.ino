// Dual core ESP32 tutorial =>https://randomnerdtutorials.com/esp32-dual-core-arduino-ide/
////////////////////////////////////////////////////////////////////////
//  ###
//   #  #    #  ####  #      #    # #####  ######  ####
//   #  ##   # #    # #      #    # #    # #      #
//   #  # #  # #      #      #    # #    # #####   ####
//   #  #  # # #      #      #    # #    # #           #
//   #  #   ## #    # #      #    # #    # #      #    #
//  ### #    #  ####  ######  ####  #####  ######  ####
//
////////////////////////////////////////////////////////////////////////
// Frameworks
#include <ArduinoJson.h>  // Json Library
#include <PubSubClient.h>
#include <WiFiClient.h>

#include "FastLED.h"
#include "Streaming.h"
#include "WiFi.h"

// Effects
#include "ColourCycle.h"
#include "ColourFade.h"
#include "Crisscross.h"  // Has some odd flickering
#include "Fire.h"
// #include "Meteor.h"

#include "Rainbow.h"
#include "Test.h"
#include "Tetris.h"

////////////////////////////////////////////////////////////////////////
//
//  ######
//  #     # ###### ###### # #    # # ##### #  ####  #    #  ####
//  #     # #      #      # ##   # #   #   # #    # ##   # #
//  #     # #####  #####  # # #  # #   #   # #    # # #  #  ####
//  #     # #      #      # #  # # #   #   # #    # #  # #      #
//  #     # #      #      # #   ## #   #   # #    # #   ## #    #
//  ######  ###### #      # #    # #   #   #  ####  #    #  ####
//
////////////////////////////////////////////////////////////////////////
// #define cloudLEDs 0
#define numPanels 9
// #define stripLEDs 13 + 6 * 12
// #define totalLEDs stripLEDs + cloudLEDs
#define totalLEDs (numPanels * 12) + 1
// #define totalLEDs 60 // LEDs in the cloud

#define LED_BUILTIN 2  // ESP32, nothing required for ESP8266
#define connectionLED LED_BUILTIN

#define dataPin 4  // ESP32
// #define dataPin D6  // ESP8266

#define OFF LOW
#define ON HIGH

#define cloudStart 136
#define cloudFinish 135 + 60

////////////////////////////////////////////////////////////////////////
//
//  #     #
//  #     #   ##   #####  #####  #    #   ##   #####  ######
//  #     #  #  #  #    # #    # #    #  #  #  #    # #
//  ####### #    # #    # #    # #    # #    # #    # #####
//  #     # ###### #####  #    # # ## # ###### #####  #
//  #     # #    # #   #  #    # ##  ## #    # #   #  #
//  #     # #    # #    # #####  #    # #    # #    # ######
//
////////////////////////////////////////////////////////////////////////
WiFiClient espClient;
PubSubClient mqtt(espClient);

// LED Strip
CRGB currentLED[totalLEDs];

Fire fire(totalLEDs, currentLED);
Test test(totalLEDs, currentLED, 50);
// Meteor meteor(totalLEDs, currentLED, 50);
Crisscross crissCross(totalLEDs, currentLED, 50);
ColourCycle colourCycle(totalLEDs, currentLED, 50);
ColourFade colourFade(totalLEDs, currentLED, 50);
Rainbow rainbow(totalLEDs, currentLED, 50);
Tetris tetris(totalLEDs, currentLED, 100);

TaskHandle_t Task1;
TaskHandle_t Task2;

////////////////////////////////////////////////////////////////////////
//
//  #     #
//  #     #   ##   #####  #   ##   #####  #      ######  ####
//  #     #  #  #  #    # #  #  #  #    # #      #      #
//  #     # #    # #    # # #    # #####  #      #####   ####
//   #   #  ###### #####  # ###### #    # #      #           #
//    # #   #    # #   #  # #    # #    # #      #      #    #
//     #    #    # #    # # #    # #####  ###### ######  ####
//
////////////////////////////////////////////////////////////////////////
int LEDBrightness = 100;  // As a percentage (saved as a dynamic variable to let us change later)

const char* wifiSsid = "I Don't Mind";
const char* wifiPassword = "Have2Biscuits";

const char* nodeName = "Hexagon Lights";

const char* disconnectMsg = "Hexagon Lights Disconnected";

const char* mqttServerIP = "mqtt.kavanet.io";

// Wifi Params
bool WiFiConnected = false;
long connectionTimeout = (2 * 1000);
long lastWiFiReconnectAttempt = 0;
long lastMQTTReconnectAttempt = 0;

unsigned long previousMillis = 0;

int testRand = 25;

int red, green, blue;

int mode = 5;

////////////////////////////////////////////////////////////////////////
//
//  ######                                                #####
//  #     # #####   ####   ####  #####    ##   #    #    #     # #####   ##   #####  ##### #    # #####
//  #     # #    # #    # #    # #    #  #  #  ##  ##    #         #    #  #  #    #   #   #    # #    #
//  ######  #    # #    # #      #    # #    # # ## #     #####    #   #    # #    #   #   #    # #    #
//  #       #####  #    # #  ### #####  ###### #    #          #   #   ###### #####    #   #    # #####
//  #       #   #  #    # #    # #   #  #    # #    #    #     #   #   #    # #   #    #   #    # #
//  #       #    #  ####   ####  #    # #    # #    #     #####    #   #    # #    #   #    ####  #
//
////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);

  // System architecture
  xTaskCreatePinnedToCore(core1Loop, "Task1", 10000, NULL, 1, &Task1, 0);
  delay(500);

  xTaskCreatePinnedToCore(core2Loop, "Task2", 10000, NULL, 1, &Task2, 1);
  delay(500);

  disableCore0WDT();  // This prevents the WDT taking out an idle core
  disableCore1WDT();  // the wifi code was triggering the WDT

  // LEDs
  FastLED.addLeds<NEOPIXEL, dataPin>(currentLED, totalLEDs);

  FastLED.setBrightness(LEDBrightness * 2.55);
  FastLED.setCorrection(0xFFB0F0);
  FastLED.setDither(1);

  FastLED.clear();  // clear all pixel data
  FastLED.show();

  // Wireless comms
  startWifi();
  startMQTT();

  tetris.begin();

  // On-board status led (Used for wifi and MQTT indication)
  pinMode(connectionLED, OUTPUT);

  Serial << "\n|** " << nodeName << " **|" << endl;
  delay(100);
}

///////////////////////////////////////////////////////////////////////
//
//  #     #                    ######
//  ##   ##   ##   # #    #    #     # #####   ####   ####  #####    ##   #    #
//  # # # #  #  #  # ##   #    #     # #    # #    # #    # #    #  #  #  ##  ##
//  #  #  # #    # # # #  #    ######  #    # #    # #      #    # #    # # ## #
//  #     # ###### # #  # #    #       #####  #    # #  ### #####  ###### #    #
//  #     # #    # # #   ##    #       #   #  #    # #    # #   #  #    # #    #
//  #     # #    # # #    #    #       #    #  ####   ####  #    # #    # #    #
//
//////////////////////////////////////////////////////////////////////
void core1Loop(void* pvParameters) {
  for (;;) {
    handleMQTT();
    handleWiFi();
    // delay(500); // * Add this back if WDT issues come back

    long interval = (5 * 1000);
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;

      publishAll();
    }
  }
}

void core2Loop(void* pvParameters) {
  for (;;) {
    if (WiFi.status() == WL_CONNECTED) {
      switch (mode) {
        case 0:  //* Off
          FastLED.clear();
          FastLED.show();
          delay(5);
          break;

        case 1:  //* Fire
          fire.run(55, 120, 20, true);
          break;

        case 2:  //* Colour Fade
          colourFade.run();
          break;

        case 3:  //* Colour Cycle
          colourCycle.run();
          break;

        case 4:  //* Meteor rain
          meteorRain(0xff, 0xff, 0x00, 10, 64, true, 30);
          break;

        case 5:  //* Rainbow
          rainbow.run();
          // rainbowCycle(20);
          break;

        case 6:
          EVERY_N_MILLISECONDS(20) {
            pacifica_loop();
            FastLED.show();
            delay(5);
          }
          break;

        case 9:
          // Do nothing, used for setting the colour via json
          break;
      }
    }
  }
}

void loop() {}

CRGBPalette16 pacifica_palette_1 =
    {0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117,
     0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x14554B, 0x28AA50};
CRGBPalette16 pacifica_palette_2 =
    {0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117,
     0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x0C5F52, 0x19BE5F};
CRGBPalette16 pacifica_palette_3 =
    {0x000208, 0x00030E, 0x000514, 0x00061A, 0x000820, 0x000927, 0x000B2D, 0x000C33,
     0x000E39, 0x001040, 0x001450, 0x001860, 0x001C70, 0x002080, 0x1040BF, 0x2060FF};

void meteorRain(byte red, byte green, byte blue, byte meteorSize, byte meteorTrailDecay, boolean meteorRandomDecay, int SpeedDelay) {
  for (int i = 0; i < totalLEDs; i++) {
    currentLED[i] = 0x000000;
  }

  for (int i = 0; i < totalLEDs + totalLEDs; i++) {
    // fade brightness all LEDs one step
    for (int j = 0; j < totalLEDs; j++) {
      if ((!meteorRandomDecay) || (random(10) > 5)) {
        fadeToBlack(j, meteorTrailDecay);
      }
    }

    // draw meteor
    for (int j = 0; j < meteorSize; j++) {
      if ((i - j < totalLEDs) && (i - j >= 0)) {
        currentLED[i - j].setRGB(red, green, blue);
        Serial << i - j << endl;
      }
    }

    FastLED.show();
    delay(SpeedDelay);
  }

  Serial << "restart" << endl;
}

void fadeToBlack(int ledNo, byte fadeValue) {
  currentLED[ledNo].fadeToBlackBy(fadeValue);
}

void rainbowCycle(int DelayDuration) {
  byte* c;
  uint16_t i, j;

  for (j = 0; j < 256; j++) {
    for (i = 0; i < totalLEDs; i++) {
      c = Wheel(((i * 256 / totalLEDs) + j) & 255);
      currentLED[totalLEDs - 1 - i].setRGB(*c, *(c + 1), *(c + 2));
    }
    FastLED.show();
    delay(DelayDuration);
  }
}

byte* Wheel(byte WheelPosition) {
  static byte c[3];

  if (WheelPosition < 85) {
    c[0] = WheelPosition * 3;
    c[1] = 255 - WheelPosition * 3;
    c[2] = 0;
  } else if (WheelPosition < 170) {
    WheelPosition -= 85;
    c[0] = 255 - WheelPosition * 3;
    c[1] = 0;
    c[2] = WheelPosition * 3;
  } else {
    WheelPosition -= 170;
    c[0] = 0;
    c[1] = WheelPosition * 3;
    c[2] = 255 - WheelPosition * 3;
  }

  return c;
}
