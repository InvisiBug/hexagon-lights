#include "Meteor.h"

#include "Arduino.h"
#include "Streaming.h"
using namespace std;

Meteor::Meteor(int totalLEDs, CRGB *currentLED, int interval) {
  this->totalLEDs = totalLEDs;
  this->currentLED = currentLED;
  this->interval = interval;

  meteorRandomDecay = 0;
  meteorTrailDecay = 0;
  SpeedDelay = 0;
  meteorSize = 0;

  currentLocation = totalLEDs - 1;

  chooseNewColour();
}

//////////////////////////////////////////////////////////////////////////////
//
//  ######
//  #     # #    # #    #
//  #     # #    # ##   #
//  ######  #    # # #  #
//  #   #   #    # #  # #
//  #    #  #    # #   ##
//  #     #  ####  #    #
//
//////////////////////////////////////////////////////////////////////////////
void Meteor::run() {
  run(interval);
}

void Meteor::run(int wait) {
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

  chooseNewColour();
}

void Meteor::chooseNewColour()  // need to include the rndom seed here
{
  randomSeed(analogRead(0));

  uint32_t newColour = colours[random(0, 24)];  // Pick a random colour from the table

  red = (newColour & 0xff0000) >> 16;
  green = (newColour & 0x00ff00) >> 8;
  blue = (newColour & 0x0000ff);
}

void Meteor::fadeToBlack(int ledNo, byte fadeValue) {
  currentLED[ledNo].fadeToBlackBy(fadeValue);
}
