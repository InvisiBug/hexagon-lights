#ifndef METEOR_H
#define METEOR_H
#include "FastLED.h"

class Meteor {
 public:
  Meteor(int totalLEDs, CRGB *currentLED, int interval);

  void run();
  void run(int wait);
  void reset();
  void changeSpeed(int interval);  // Time to change to next speed
  void setInterval(int interval);

  void fadeToBlack(int ledNo, byte fadeValue);

  int getInterval();
  void chooseNewColour();

 private:
  CRGB *currentLED;  // LED strip object

  uint32_t colours[24] = {  // https://www.w3schools.com/colors/colors_picker.asp
      0xff4000, 0xff8000, 0xffbf00, 0xffff00, 0xbfff00, 0x80ff00,
      0x40ff00, 0x00ff00, 0x00ff40, 0x00ff80, 0x00ffbf, 0x00ffff,
      0x00bfff, 0x0080ff, 0x0040ff, 0x0000ff, 0x4000ff, 0x8000ff,
      0xbf00ff, 0xff00ff, 0xff00bf, 0xff0080, 0xff0040, 0xff0000};

  int initialInterval;

  int Position;

  bool slowDown;

  int totalLEDs;
  int totalSteps;
  int currentStep;
  int currentLocation;

  int red;
  int green;
  int blue;

  int meteorRandomDecay;
  int meteorTrailDecay;
  int SpeedDelay;
  int meteorSize;

  bool clear;
  int pos = 0;

  // Timing Stuff
  long currentMillis;
  long lastMillis;
  int interval;

  long speedChangeCurrentMillis;
  long speedChangeLastMillis;

  int a = 0;
  int b = 0;
};
#endif
